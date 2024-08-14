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
     PopMaBEstEngine.h

   Authors:
     Vincent Noël <vincent.noel@curie.fr>

   Date:
     March 2021
*/

#ifndef _POPMABESTENGINE_H_
#define _POPMABESTENGINE_H_

#include <string>
#include <map>
#include <vector>
#include <assert.h>

#include "MetaEngine.h"
#include "BooleanNetwork.h"
#include "Cumulator.h"
#include "RandomGenerator.h"
#include "RunConfig.h"
#include "FixedPointDisplayer.h"
#include "ProbTrajDisplayer.h"

struct ArgWrapper;

#ifdef POPNETWORKSTATE_STD_MAP
// EV 2021-11-12: use std::map instead of STATE_MAP (std::unordered_map) for PopNetworkStateMap
typedef std::map<PopNetworkState, double> PopNetworkStateMap;
#else
typedef STATE_MAP<PopNetworkState, double> PopNetworkStateMap;
#endif

class PopMaBEstEngine : public MetaEngine {

  PopNetwork* pop_network;
  
  STATE_MAP<NetworkState_Impl, unsigned int> fixpoints;
  std::vector<STATE_MAP<NetworkState_Impl, unsigned int>*> fixpoint_map_v;
  
  Cumulator<PopNetworkState>* merged_cumulator;
  std::vector<Cumulator<PopNetworkState>* > cumulator_v;

  STATE_MAP<NetworkState_Impl, unsigned int>* mergeFixpointMaps();

public:
  static const std::string VERSION;
  static int verbose;
  static void setVerbose(int level);

#ifdef MPI_COMPAT
  PopMaBEstEngine(PopNetwork* pop_network, RunConfig* runconfig, int world_size, int world_rank);
#else  
  PopMaBEstEngine(PopNetwork* pop_network, RunConfig* runconfig);
#endif
  void run(std::ostream* output_traj);

  ~PopMaBEstEngine();

  bool converges() const {return fixpoints.size() > 0;}
  const STATE_MAP<NetworkState_Impl, unsigned int>& getFixpoints() const {return fixpoints;}
  const std::map<unsigned int, std::pair<NetworkState, double> > getFixPointsDists() const;

  Cumulator<PopNetworkState>* getMergedCumulator() {
    return merged_cumulator; 
  }

  void displayFixpoints(FixedPointDisplayer* displayer) const;
  void displayPopProbTraj(ProbTrajDisplayer<PopNetworkState>* displayer) const;
  void display(ProbTrajDisplayer<PopNetworkState>* pop_probtraj_displayer, FixedPointDisplayer* fp_displayer) const;
  
  std::vector<ArgWrapper*> arg_wrapper_v;
#ifdef EV_OPTIM_2021_10
  PopNetworkState getTargetNode(RandomGenerator* random_generator, const PopNetworkStateMap& popNodeTransitionRates, double total_rate) const;
#else
  PopNetworkState getTargetNode(RandomGenerator* random_generator, const PopNetworkStateMap popNodeTransitionRates, double total_rate) const;
#endif
  double computeTH(const MAP<NodeIndex, double>& nodeTransitionRates, double total_rate) const;
  void epilogue();
  static void* threadWrapper(void *arg);
  void runThread(Cumulator<PopNetworkState>* cumulator, unsigned int start_count_thread, unsigned int sample_count_thread, RandomGeneratorFactory* randgen_factory, int seed, STATE_MAP<NetworkState_Impl, unsigned int>* fixpoint_map, std::ostream* output_traj);
  void displayRunStats(std::ostream& os, time_t start_time, time_t end_time) const;

  static void mergePairOfFixpoints(STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints_1, STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints_2);
  static void* threadMergeWrapper(void *arg);
  std::pair<Cumulator<PopNetworkState>*, STATE_MAP<NetworkState_Impl, unsigned int>*> mergeResults(std::vector<Cumulator<PopNetworkState>*> cumulator_v, std::vector<STATE_MAP<NetworkState_Impl, unsigned int>*> fixpoint_map_v);
  
#ifdef MPI_COMPAT
  static std::pair<Cumulator<PopNetworkState>*, STATE_MAP<NetworkState_Impl, unsigned int>*> mergeMPIResults(RunConfig* runconfig, Cumulator<PopNetworkState>* ret_cumul, STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints, int world_size, int world_rank, bool pack=false);  
  
  static void mergePairOfMPIFixpoints(STATE_MAP<NetworkState_Impl, unsigned int>* fixpoints, int world_rank, int dest, int origin, bool pack=false);
  static void MPI_Unpack_Fixpoints(STATE_MAP<NetworkState_Impl, unsigned int>* fp_map, char* buff, unsigned int buff_size);
  static char* MPI_Pack_Fixpoints(const STATE_MAP<NetworkState_Impl, unsigned int>* fp_map, int dest, unsigned int * buff_size);
  static void MPI_Send_Fixpoints(const STATE_MAP<NetworkState_Impl, unsigned int>* fp_map, int dest);
  static void MPI_Recv_Fixpoints(STATE_MAP<NetworkState_Impl, unsigned int>* fp_map, int origin);
  
#endif

};

#endif
