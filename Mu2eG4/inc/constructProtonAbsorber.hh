#ifndef Mu2eG4_constructProtonAbsorber_hh
#define Mu2eG4_constructProtonAbsorber_hh
//
// Free function to construct Proton Absorber
//
// $Id: constructProtonAbsorber.hh,v 1.3 2012/11/19 23:03:24 genser Exp $
// $Author: genser $
// $Date: 2012/11/19 23:03:24 $
//
// Original author KLG
//

// Mu2e includes.
#include "G4Helper/inc/VolumeInfo.hh"

namespace mu2e {

  class SimpleConfig;

  void constructProtonAbsorber(SimpleConfig const & _config
                               );

}

#endif /* Mu2eG4_constructProtonAbsorber_hh */
