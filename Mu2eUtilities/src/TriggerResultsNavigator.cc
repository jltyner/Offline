// #include "fhiclcpp/ParameterSet.h"
// #include "fhiclcpp/ParameterSetRegistry.h"
#include "Mu2eUtilities/inc/TriggerResultsNavigator.hh"
#include <ostream>

namespace mu2e {

  // TriggerResultsNavigator::TriggerResultsNavigator(const art::TriggerResults* trigResults): 
  //   _trigResults(trigResults){
  //   auto const  id   = trigResults->parameterSetID();
  //   auto const& pset = fhicl::ParameterSetRegistry::get(id);
  //   //set the vector<string> with the names of the tirgger_paths
  //   _trigPathsNames  = pset.get<std::vector<std::string>>("trigger_paths");
      
  //   //loop over trigResults to fill the map <string, unsigned int) 
  //   for (unsigned int i=0; i< _trigPathsNames.size(); ++i){
  //     _trigMap.insert(std::pair<std::string, unsigned int>(_trigPathsNames[i], i));
  //   }
  // }

  size_t
  TriggerResultsNavigator::findTrigPath(std::string const& name) const
  {
    return find(_trigMap, name);
  }

  size_t
  TriggerResultsNavigator::find(std::map<std::string, unsigned int> const& posmap, std::string const& name) const
  {
    auto const pos = posmap.find(name);
    if (pos == posmap.cend()) {
      return posmap.size();
    } else {
      return pos->second;
    }
  }

  // Has ith path accepted the event?
  bool
  TriggerResultsNavigator::accept(std::string const& name) const
  {
    size_t index = findTrigPath(name);
    return _trigResults->accept(index);
  }
    
  void 
  TriggerResultsNavigator::print() const {
    printf("TriggerResultsNaviogator Map\n");
    printf("//------------------------------------------//\n");
    printf("//  trig_pathName          id     accepted  //\n");
    printf("//------------------------------------------//\n");

    for  (unsigned int i=0; i< _trigPathsNames.size(); ++i){
      std::string name     = _trigPathsNames[i];
      size_t      index    = findTrigPath(name);
      bool        accepted = accept(name);
      printf("// %24s  %2li       %i    //\n", name.c_str(), index, accepted == true ? 1:0);
    }
      
  }

}
