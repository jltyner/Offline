// stoppedMuMinusList() extracts a list of stopped negative muons from
// a SimParticleCollection.  The intended clients are stopped muon
// resampling generators, which operate on inputs pre-filtered in the
// previous simulation stage.  So we are NOT doing things like finding
// muon stops in the stopping target here.  Instead we simply look for
// muons that have the muMinusCaptureAtRest stopping code, and return
// them all.
//
// Similar functions for resampling mu+, pi-, etc.  can be implemented
// by calling simParticleList() with appropriate parameters.
//
// Andrei Gaponenko, 2021

#ifndef Mu2eUtilities_inc_stoppedMuonList_hh
#define Mu2eUtilities_inc_stoppedMuonList_hh

#include <vector>

#include "art/Framework/Principal/Handle.h"

#include "MCDataProducts/inc/SimParticle.hh"
#include "MCDataProducts/inc/ProcessCode.hh"
#include "DataProducts/inc/PDGCode.hh"

namespace mu2e {

  std::vector<art::Ptr<SimParticle> > simParticleList(art::ValidHandle<SimParticleCollection> simh,
                                                      PDGCode::type pdgId,
                                                      ProcessCode stoppingCode);

  inline std::vector<art::Ptr<SimParticle> > stoppedMuMinusList(art::ValidHandle<SimParticleCollection> simh) {
    // G4 sets this end code for both decay and capture cases
    return simParticleList(simh, PDGCode::mu_minus, ProcessCode::muMinusCaptureAtRest);
  }

}

#endif/*Mu2eUtilities_inc_stoppedMuonList_hh*/
