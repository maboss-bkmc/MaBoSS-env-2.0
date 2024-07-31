/*
#############################################################################
#                                                                           #
# BSD 3-Clause License (see https://opensource.org/licenses/BSD-3-Clause)   #
#                                                                           #
# Copyright (c) 2011-2020 Institut Curie, 26 rue d'Ulm, Paris, France       #
# All rights reserved.                                                      #
#                                                                           #
# Redistribution and use in source and binary forms, with or without        #
# modification, are permitted provided that the following conditions are    #
# met:                                                                      #
#                                                                           #
# 1. Redistributions of source code must retain the above copyright notice, #
# this list of conditions and the following disclaimer.                     #
#                                                                           #
# 2. Redistributions in binary form must reproduce the above copyright      #
# notice, this list of conditions and the following disclaimer in the       #
# documentation and/or other materials provided with the distribution.      #
#                                                                           #
# 3. Neither the name of the copyright holder nor the names of its          #
# contributors may be used to endorse or promote products derived from this #
# software without specific prior written permission.                       #
#                                                                           #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       #
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED #
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           #
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER #
# OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  #
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       #
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        #
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    #
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      #
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        #
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              #
#                                                                           #
#############################################################################

   Module:
     maboss_sim.cpp

   Authors:
     Vincent Noël <vincent.noel@curie.fr>
 
   Date:
     January-March 2020
*/

#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <set>
#include "src/BooleanNetwork.h"
#include "src/MaBEstEngine.h"
#include "src/FinalStateSimulationEngine.h"
#include "maboss_res.cpp"
#include "maboss_resfinal.cpp"
#include "maboss_commons.h"
#include "maboss_net.cpp"
#include "maboss_cfg.cpp"
#include <sstream>
typedef struct {
  PyObject_HEAD
  Network* network;
  RunConfig* runconfig;
} cMaBoSSSimObject;

static void cMaBoSSSim_dealloc(cMaBoSSSimObject *self)
{
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * cMaBoSSSim_new(PyTypeObject* type, PyObject *args, PyObject* kwargs) 
{
  try {
    // Loading arguments
    PyObject* net = NULL;
    PyObject* cfg = NULL;
    char * network_file = NULL;
    char * config_file = NULL;
    char * network_str = NULL;
    char * config_str = NULL;
    bool use_sbml_names = false;
    static const char *kwargs_list[] = {"network", "config", "network_str", "config_str", "net", "cfg", "use_sbml_names", NULL};
    if (!PyArg_ParseTupleAndKeywords(
      args, kwargs, "|ssssOOp", const_cast<char **>(kwargs_list), 
      &network_file, &config_file, &network_str, &config_str, &net, &cfg, &use_sbml_names
    ))
      return NULL;
      
    Network* network = nullptr;
    RunConfig* runconfig = nullptr;
    if (network_file != NULL) {
      // Loading bnd file
      std::string nf(network_file);
      network = new Network();
#ifdef SBML_COMPAT
      if (nf.substr(nf.find_last_of(".") + 1) == "sbml" || nf.substr(nf.find_last_of(".") + 1) == "xml" ) {
        network->parseSBML(network_file, NULL, use_sbml_names); 
      } else {
#endif
        network->parse(network_file);
#ifdef SBML_COMPAT
      }
#endif
      // Loading cfg file
      runconfig = new RunConfig();
      IStateGroup::reset(network);
      if (config_file != NULL) {
        runconfig->parse(network, config_file);
      }
    } 
    else if (network_str != NULL && config_str != NULL) {
      // Loading bnd file
      network = new Network();
      network->parseExpression((const char *) network_str);
      
      // Loading cfg file
      runconfig = new RunConfig();
      IStateGroup::reset(network);
      runconfig->parseExpression(network, config_str);

    } else if (net != NULL && cfg != NULL) {
      network = ((cMaBoSSNetworkObject*) net)->network;
      runconfig = ((cMaBoSSConfigObject*) cfg)->config;
    }
    
    if (network != nullptr && runconfig != nullptr) {

      // Error checking
      IStateGroup::checkAndComplete(network);
      network->getSymbolTable()->checkSymbols();

      cMaBoSSSimObject* simulation;
      simulation = (cMaBoSSSimObject *) type->tp_alloc(type, 0);
      simulation->network = network;
      simulation->runconfig = runconfig;

      return (PyObject *) simulation;
    } else return Py_None;
  }
  catch (BNException& e) {
    PyErr_SetString(PyBNException, e.getMessage().c_str());
    return NULL;
  }
}

static PyObject* cMaBoSSSim_run(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) {
  
  int only_last_state = 0;
  static const char *kwargs_list[] = {"only_last_state", NULL};
  if (!PyArg_ParseTupleAndKeywords(
    args, kwargs, "|i", const_cast<char **>(kwargs_list), 
    &only_last_state
  ))
    return NULL;
    
  bool b_only_last_state = PyObject_IsTrue(PyBool_FromLong(only_last_state));
  time_t start_time, end_time;

  RandomGenerator::resetGeneratedNumberCount();
  if (b_only_last_state) {
  
    FinalStateSimulationEngine* simulation = new FinalStateSimulationEngine(self->network, self->runconfig);
    time(&start_time);
    simulation->run(NULL);
    time(&end_time);
    cMaBoSSResultFinalObject* res = (cMaBoSSResultFinalObject*) PyObject_New(cMaBoSSResultFinalObject, &cMaBoSSResultFinal);
    res->network = self->network;
    res->runconfig = self->runconfig;
    res->engine = simulation;
    res->start_time = start_time;
    res->end_time = end_time;
    res->last_probtraj = Py_None;
    return (PyObject*) res;
  } else {

    MaBEstEngine* simulation = new MaBEstEngine(self->network, self->runconfig);
    time(&start_time);
    simulation->run(NULL);
    time(&end_time);
    
    cMaBoSSResultObject* res = (cMaBoSSResultObject*) PyObject_New(cMaBoSSResultObject, &cMaBoSSResult);
    res->network = self->network;
    res->runconfig = self->runconfig;
    res->engine = simulation;
    res->start_time = start_time;
    res->end_time = end_time;
    res->probtraj = Py_None;
    res->last_probtraj = Py_None;
    return (PyObject*) res;
  }
}


static PyObject* cMaBoSSSim_check(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) {
  try {
    IStateGroup::checkAndComplete(self->network);
    self->network->getSymbolTable()->checkSymbols();
  } catch (BNException& e) {
    PyErr_SetString(PyBNException, e.getMessage().c_str());
    return NULL;
  }
  return Py_None;
}

static PyObject* cMaBoSSSim_get_logical_rules(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) {
  
  std::ostringstream ss;
  self->network->generateLogicalExpressions(ss);
  return PyUnicode_FromString(ss.str().c_str());
}

static PyObject* cMaBoSSSim_bnd_str(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) {
  std::ostringstream bnd;
  self->network->display(bnd);
  return PyUnicode_FromString(bnd.str().c_str());
}


static PyObject* cMaBoSSSim_cfg_str(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) {
  std::ostringstream cfg;
  self->runconfig->dump(self->network, cfg, MaBEstEngine::VERSION);
  return PyUnicode_FromString(cfg.str().c_str());
}

static PyObject* cMaBoSSSim_update_parameters(cMaBoSSSimObject* self, PyObject *args, PyObject* kwargs) 
{
  PyObject * time_tick = NULL;
  PyObject * max_time = NULL;
  PyObject * sample_count = NULL;
  PyObject * init_pop = NULL;
  PyObject * discrete_time = NULL;
  PyObject * use_physrandgen = NULL;
  PyObject * use_glibcrandgen = NULL;
  PyObject * use_mtrandgen = NULL;
  PyObject * seed_pseudorandom = NULL;
  PyObject * display_traj = NULL;
  PyObject * statdist_traj_count = NULL;
  PyObject * statdist_cluster_threshold = NULL;
  PyObject * thread_count = NULL;
  PyObject * statdist_similarity_cache_max_size = NULL; 
 
  static const char *kwargs_list[] = {
    "time_tick", "max_time", "sample_count",
    "discrete_time", "use_physrandgen", "use_glibcrandgen",
    "use_mtrandgen", "seed_pseudorandom", "display_traj", 
    "statdist_traj_count", "statdist_cluster_threshold", 
    "thread_count", "statdist_similarity_cache_max_size",
    NULL
  };
  
  if (!PyArg_ParseTupleAndKeywords(
    args, kwargs, "|OOOOOOOOOOOOO", const_cast<char **>(kwargs_list), 
    &time_tick, &max_time, &sample_count,
    &discrete_time, &use_physrandgen, &use_glibcrandgen, 
    &use_mtrandgen, &seed_pseudorandom, &display_traj, 
    &statdist_traj_count, &statdist_cluster_threshold, 
    &thread_count, &statdist_similarity_cache_max_size
  ))
    return NULL;
    
 if (time_tick != NULL) {
    self->runconfig->setParameter("time_tick", PyFloat_AsDouble(time_tick));
  }
  if (max_time != NULL) {
    self->runconfig->setParameter("max_time", PyFloat_AsDouble(max_time));
  }
  if (sample_count != NULL) {
    self->runconfig->setParameter("sample_count", PyLong_AsLong(sample_count));
  }
  if (init_pop != NULL) {
    self->runconfig->setParameter("init_pop", PyLong_AsLong(init_pop));
  }
  if (discrete_time != NULL) {
    self->runconfig->setParameter("discrete_time", PyLong_AsLong(discrete_time));
  }
  if (use_physrandgen != NULL) {
    self->runconfig->setParameter("use_physrandgen", PyLong_AsLong(use_physrandgen));
  }
  if (use_glibcrandgen != NULL) {
    self->runconfig->setParameter("use_glibcrandgen", PyLong_AsLong(use_glibcrandgen));
  }
  if (use_mtrandgen != NULL) {
    self->runconfig->setParameter("use_mtrandgen", PyLong_AsLong(use_mtrandgen));
  }
  if (seed_pseudorandom != NULL) {
    self->runconfig->setParameter("seed_pseudorandom", PyFloat_AsDouble(seed_pseudorandom));
  }
  if (display_traj != NULL) {
    self->runconfig->setParameter("display_traj", PyLong_AsLong(display_traj));
  }
  if (statdist_traj_count != NULL) {
    self->runconfig->setParameter("statdist_traj_count", PyLong_AsLong(statdist_traj_count));
  }
  if (statdist_cluster_threshold != NULL) {
    self->runconfig->setParameter("statdist_cluster_threshold", PyFloat_AsDouble(statdist_cluster_threshold));
  }
  if (thread_count != NULL) {
    self->runconfig->setParameter("thread_count", PyLong_AsLong(thread_count));
  }
  if (statdist_similarity_cache_max_size != NULL) {
    self->runconfig->setParameter("statdist_similarity_cache_max_size", PyLong_AsLong(statdist_similarity_cache_max_size));
  }
  
  return Py_None;
}

static PyObject* cMaBoSSSim_get_nodes(cMaBoSSSimObject* self) {

  PyObject *list = PyList_New(self->network->getNodes().size());

  size_t index = 0;
  for (auto* node: self->network->getNodes()) {
    PyList_SetItem(list, index, PyUnicode_FromString(node->getLabel().c_str()));
    index++;
  }

  return list;
}

static PyMethodDef cMaBoSSSim_methods[] = {
    {"run", (PyCFunction) cMaBoSSSim_run, METH_VARARGS | METH_KEYWORDS, "runs the simulation"},
    {"check", (PyCFunction) cMaBoSSSim_check, METH_VARARGS | METH_KEYWORDS, "checks the model"},
    {"str_bnd", (PyCFunction) cMaBoSSSim_bnd_str, METH_VARARGS | METH_KEYWORDS, "returns the contents of the bnd file"},
    {"str_cfg", (PyCFunction) cMaBoSSSim_cfg_str, METH_VARARGS | METH_KEYWORDS, "checks the contents of the cfg file"},
    {"get_logical_rules", (PyCFunction) cMaBoSSSim_get_logical_rules, METH_VARARGS | METH_KEYWORDS, "returns logical formulas"},
    {"update_parameters", (PyCFunction) cMaBoSSSim_update_parameters, METH_VARARGS | METH_KEYWORDS, "changes the parameters of the simulation"},
    {"get_nodes", (PyCFunction) cMaBoSSSim_get_nodes, METH_NOARGS, "returns the list of nodes"},
    {NULL}  /* Sentinel */
};

static PyTypeObject cMaBoSSSim = []{
    PyTypeObject sim{PyVarObject_HEAD_INIT(NULL, 0)};

    sim.tp_name = "cmaboss.cMaBoSSSimObject";
    sim.tp_basicsize = sizeof(cMaBoSSSimObject);
    sim.tp_itemsize = 0;
    sim.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    sim.tp_doc = "cMaBoSS Simulation object";
    sim.tp_new = cMaBoSSSim_new;
    sim.tp_dealloc = (destructor) cMaBoSSSim_dealloc;
    sim.tp_methods = cMaBoSSSim_methods;
    return sim;
}();
