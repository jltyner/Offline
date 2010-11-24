//
// This class makes available a collection of SimParticles that 
// have more than a minimum number of StrawHits with energy deposition
// in the gas above some cut.  The class can also return a
// vector of information about all of the StrawHits on each track
// in the collection.  If a StrawHit contains contributions from
// more than one SimParticle, that StrawHit will appear in the hit lists
// for all of the contributing SimParticles.
//
// This class is not designed to be peristable.
//
// $Id: SimParticlesWithHits.cc,v 1.1 2010/11/24 01:04:28 kutschke Exp $
// $Author: kutschke $
// $Date: 2010/11/24 01:04:28 $
//
// Original author Rob Kutschke.
//
// Notes:
// 1) Near the end of the c'tor we loop over a map and remove entries with
//    too few hits. To make erase work properly we must use the 
//     postincrement ++ on the iterator that is the argument to erase. See 
//     Josuttis (1999) section 6.6 p 205.

// Framework includes
#include "FWCore/Framework/interface/Event.h"

// Mu2e includes
#include "Mu2eUtilities/inc/SimParticlesWithHits.hh"
#include "Mu2eUtilities/inc/resolveDPIndices.hh"

namespace mu2e{

  SimParticlesWithHits::SimParticlesWithHits( const edm::Event& evt,
                                              string const& _g4ModuleLabel,
                                              string const& _hitMakerModuleLabel,
                                              string const& _trackerStepPoints,
                                              double minEnergyDep,
                                              size_t minHits ):
    _hitsPerTrack(),
    _hits_mcptr(0),
    _stepPointsMC(0){

    // Get information from the event.
    edm::Handle<StrawHitCollection> pdataHandle;
    evt.getByLabel(_hitMakerModuleLabel,pdataHandle);
    StrawHitCollection const& hits = *pdataHandle;

    edm::Handle<StrawHitMCTruthCollection> truthHandle;
    evt.getByLabel(_hitMakerModuleLabel,truthHandle);
    StrawHitMCTruthCollection const& hits_truth = *truthHandle;

    edm::Handle<StrawHitMCPtrCollection> mcptrHandle;
    evt.getByLabel(_hitMakerModuleLabel,mcptrHandle);
    _hits_mcptr = mcptrHandle.product();

    edm::Handle<StepPointMCCollection> mchitsHandle;
    evt.getByLabel(_g4ModuleLabel,_trackerStepPoints,mchitsHandle);
    _stepPointsMC = mchitsHandle.product();

    edm::Handle<SimParticleCollection> simsHandle;
    evt.getByLabel(_g4ModuleLabel,simsHandle);
    SimParticleCollection const& sims = *simsHandle;

    // Loop over all straw hits.
    for ( size_t ihit=0; ihit<hits.size(); ++ihit ) {

      // Data and MC truth for this hit.
      StrawHit        const&   hit(hits.at(ihit));
      StrawHitMCTruth const& truth(hits_truth.at(ihit));
      StrawHitMCPtr   const& mcptr(_hits_mcptr->at(ihit));

      // Skip hits with too little energy deposited in the straw.
      if ( hit.energyDep() < minEnergyDep ){
        continue;
      }

      // Find all simulated tracks that contribute to this hit.
      set<key_type> contributingTracks;
      for ( size_t istep = 0; istep<mcptr.size(); ++istep){
        StepPointMC const& step = *resolveDPIndex<StepPointMCCollection>( evt, mcptr.at(istep) );
        contributingTracks.insert(step.trackId());
      }

      // Add this hit to the list of hits on all simulated tracks that contribute to it.
      for ( set<key_type>::const_iterator icontrib=contributingTracks.begin();
            icontrib != contributingTracks.end(); ++icontrib){

        key_type key=*icontrib;

        // Find the SimParticleInfo in the map; if absent, create it.
        map<key_type,SimParticleInfo>::iterator ii = _hitsPerTrack.find(key);
        if ( ii == _hitsPerTrack.end() ){
          pair<map<key_type,SimParticleInfo>::iterator,bool> yy = _hitsPerTrack.insert( 
              make_pair(key,SimParticleInfo( key, sims[*icontrib], evt)) );
          ii = yy.first;
        }

        // Add information about this hit to this track.
        vector<StrawHitMCInfo>& vv = ii->second.strawHitInfos();
        vv.push_back(StrawHitMCInfo(evt,key,ihit,hit,truth,mcptr,contributingTracks.size()) );

      }
    }

    // Remove tracks with too few hits.
    // For tracks that remain, sort in order of increasing time.
    // See note 1 about using erase on a map.
    for ( map_type::iterator i=_hitsPerTrack.begin();
          i != _hitsPerTrack.end(); ){
      vector<StrawHitMCInfo>& v = i->second.strawHitInfos();
      if ( v.size() < minHits ){
        _hitsPerTrack.erase(i++);
      } else{
        sort ( v.begin(), v.end() );
        ++i;
      }
    }

    // Check that the map is self consistent.
    selfTest();

  }  // end c'tor

  SimParticlesWithHits::mapped_type const* SimParticlesWithHits::findOrNull( key_type key){
    const_iterator i = _hitsPerTrack.find(key);
    if ( i == _hitsPerTrack.end() ) return 0;
    return &i->second;
  }

  SimParticlesWithHits::mapped_type const& SimParticlesWithHits::at( key_type key){
    mapped_type const* p = findOrNull(key);
    if ( !p ){
      ostringstream out;
      out << "SimParticlesWithHits: no such key " << key;
      throw std::out_of_range( out.str() );
    }
    return *p;
  }

  // Follow pointers from SimParticles to StepPointMC and check that the StepPointMCs
  // are indeed created by this SimParticle.
  void SimParticlesWithHits::selfTest(){

    for ( map_type::const_iterator i=_hitsPerTrack.begin();
          i != _hitsPerTrack.end(); ++i ){
      key_type simId = i->first;
      vector<StrawHitMCInfo> const& v = i->second.strawHitInfos();
      for ( size_t j=0; j<v.size(); ++j ){
        StrawHitMCInfo const& info = v[j];
        StrawHitMCPtr const& mcptr(_hits_mcptr->at(info.index()));
        bool ok = false;
        for ( size_t k=0; k<mcptr.size(); ++k ){
          StepPointMC const& step = _stepPointsMC->at(mcptr.at(k).index);
          if ( step.trackId() == simId ){
            ok = true;
          }
        }
        if ( !ok ){
          throw cms::Exception("HITS")
            << "SimParticlesWithHits fails self test.  "
            << "The event is not internally consistent.\n";
        }
      }
    }

  } // end selfTest

}
