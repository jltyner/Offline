// Author: S Middleton
// Date: Dec 2019
// Purpose: For Developing Straight Track Trigger

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "Offline/RecoDataProducts/inc/TrkFitFlag.hh"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Sequence.h"

#include "Offline/RecoDataProducts/inc/CosmicTrackSeed.hh"
#include "Offline/RecoDataProducts/inc/TriggerInfo.hh"

#include <string>
#include <vector>
#include <iostream>
#include <memory>

using namespace CLHEP;

namespace mu2e
{
  class CosmicSeedFilter : public art::EDFilter
  {
  public:
    struct Config{
      using Name=fhicl::Name;
      using Comment=fhicl::Comment;
      fhicl::Atom<art::InputTag>   cosmictag {Name("CosmicTrackSeedCollection"),Comment("track collection")};
      fhicl::Sequence<std::string> goodcosmic{Name("cosmicseedFitFlag"),Comment("Required flags"),std::vector<std::string>{"HelixOK","HelixConverged"}};
      fhicl::Atom<int>             minnsh    {Name("minnsh"), Comment("minimum number of straw hits ")};
      fhicl::Atom<int>             debug     {Name("debugLevel"), Comment("set to 1 for debug prints")};
    };
    typedef art::EDFilter::Table<Config> Parameters;
    explicit CosmicSeedFilter(const Parameters& conf);

    virtual bool filter(art::Event& event) override;
    virtual bool endRun( art::Run& run )   override;

  private:

    art::InputTag   _cosmicTag;
    TrkFitFlag      _goodcosmic;
    unsigned int    _minnsh;
    int             _debug;
    unsigned        _nevt, _npass;
  };

  CosmicSeedFilter::CosmicSeedFilter(const Parameters& conf) :
    art::EDFilter(conf),
    _cosmicTag (conf().cosmictag()),
    _goodcosmic (conf().goodcosmic()),
    _minnsh (conf().minnsh()),
    _debug (conf().debug()),
    _nevt(0), _npass(0)
  {
    produces<TriggerInfo>();
  }

  bool CosmicSeedFilter::filter(art::Event& evt){
    std::unique_ptr<TriggerInfo> triginfo(new TriggerInfo);
    ++_nevt;
    bool   retval(false);
    // find the collection
    auto cosH = evt.getValidHandle<CosmicTrackSeedCollection>(_cosmicTag);
    const CosmicTrackSeedCollection* coscol = cosH.product();
    for(auto icos = coscol->begin(); icos != coscol->end(); ++icos) {
      auto const& cosmic = *icos;

      if( cosmic.status().hasAllProperties(_goodcosmic) && cosmic.trkstrawhits().size() > _minnsh ){

        ++_npass;
        size_t index = std::distance(coscol->begin(),icos);
        triginfo->_cosmics.push_back(art::Ptr<CosmicTrackSeed>(cosH,index));
        if(_debug > 1){
          std::cout << moduleDescription().moduleLabel() << " passed event " << evt.id() << std::endl;
        }
        break;
      }
    }
    evt.put(std::move(triginfo));
    return retval;
  }

  bool CosmicSeedFilter::endRun( art::Run& run ) {
    if(_debug > 0 && _nevt > 0){
      std::cout << moduleDescription().moduleLabel() << " passed " <<  _npass << " events out of " << _nevt << " for a ratio of " << float(_npass)/float(_nevt) << std::endl;
    }
    return true;
  }
}
using mu2e::CosmicSeedFilter;
DEFINE_ART_MODULE(CosmicSeedFilter);
