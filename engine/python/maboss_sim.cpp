#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <set>
#include "src/BooleanNetwork.h"
#include "src/MaBEstEngine.h"
#include "maboss_res.cpp"

typedef struct {
  PyObject_HEAD
  Network* network;
  RunConfig* runconfig;
  // MaBEstEngine* simulation;
} cMaBoSSSimObject;

static void cMaBoSSSim_dealloc(cMaBoSSSimObject *self)
{
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * cMaBoSSSim_new(PyTypeObject* type, PyObject *args, PyObject* kwargs) 
{
  try {
    // Loading arguments
    char * network_file = NULL;
    char * config_file = NULL;
    char * network_str = NULL;
    char * config_str = NULL;
    static const char *kwargs_list[] = {"network", "config", "network_str", "config_str", NULL};
    if (!PyArg_ParseTupleAndKeywords(
      args, kwargs, "|ssss", const_cast<char **>(kwargs_list), 
      &network_file, &config_file, &network_str, &config_str
    ))
      return NULL;
      
    Network* network = nullptr;
    RunConfig* runconfig = nullptr;
    if (network_file != NULL && config_file != NULL) {
      // Loading bnd file
      network = new Network();
      network->parse(network_file);

      // Loading cfg file
      runconfig = new RunConfig();
      IStateGroup::reset(network);
      runconfig->parse(network, config_file);

    } 
    else if (network_str != NULL && config_str != NULL) {
      // Loading bnd file
      network = new Network();
      network->parseExpression((const char *) network_str);
      
      // Loading cfg file
      runconfig = new RunConfig();
      IStateGroup::reset(network);
      runconfig->parseExpression(network, config_str);

    }
    
    if (network != nullptr && runconfig != nullptr) {
      // Error checking
      IStateGroup::checkAndComplete(network);
      
      cMaBoSSSimObject* simulation;
      simulation = (cMaBoSSSimObject *) type->tp_alloc(type, 0);
      simulation->network = network;
      simulation->runconfig = runconfig;

      return (PyObject *) simulation;
    } else return Py_None;
  }
  catch (BNException& e) {
    std::cout << "EXCEPTION" << std::endl;
    std::cout << e.getMessage() << std::endl;
    return Py_None;
  }
}

static PyObject* cMaBoSSSim_run(cMaBoSSSimObject* self, PyObject*Py_UNUSED) {
  
  MaBEstEngine* simulation = new MaBEstEngine(self->network, self->runconfig);

  simulation->run(NULL);
  
  cMaBoSSResultObject* res = (cMaBoSSResultObject*) PyObject_New(cMaBoSSResultObject, &cMaBoSSResult);
  res->network = self->network;
  res->engine = simulation;
  
  return (PyObject*) res;
}

static PyMethodDef cMaBoSSSim_methods[] = {
    {"run", (PyCFunction) cMaBoSSSim_run, METH_NOARGS, "runs the simulation"},
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