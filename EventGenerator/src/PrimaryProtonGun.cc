//
// Generate a proton with the primary proton energy
// from a random spot within the target system at
// a random time during the accelerator cycle.
//
// $Id: PrimaryProtonGun.cc,v 1.3 2010/04/07 17:43:50 rhbob Exp $ 
// $Author: rhbob $
// $Date: 2010/04/07 17:43:50 $
//
// Original author Rob Kutschke
// 

// C++ incldues.
#include <iostream>

// Framework includes
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

// Mu2e includes
#include "EventGenerator/inc/PrimaryProtonGun.hh" 
#include "Mu2eUtilities/inc/SimpleConfig.hh"
#include "Mu2eUtilities/inc/safeSqrt.hh"
#include "GeometryService/inc/GeomHandle.hh"
#include "ConditionsService/inc/ConditionsHandle.hh"
#include "ConditionsService/inc/AcceleratorParams.hh"
#include "ConditionsService/inc/DAQParams.hh"
#include "TargetGeom/inc/Target.hh"
#include "Mu2eUtilities/inc/PDGCode.hh"

// Other external includes.
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Units/PhysicalConstants.h"
#include "CLHEP/Random/RandGaussQ.h"


using namespace std;

using CLHEP::Hep3Vector;
using CLHEP::HepLorentzVector;
using CLHEP::RandFlat;
using CLHEP::twopi;
using CLHEP::RandGaussQ;


namespace mu2e {

  // Mass of the proton.
  // Once we have the HepPDT package installed, get this number from there.
  static const double m = 938.272*MeV;
 
  
  // the kinetic energy of proton is 8 Gev; E = T + m, p = sqrt(E^2 - m^2)
    static const double pPrimaryProton = 8888.6*MeV;

  PrimaryProtonGun::PrimaryProtonGun( edm::Run& run, const SimpleConfig& config ):
    GeneratorBase(){
    
    _doPrimaryProt = config.getBool( "primaryProtonGun.do", 1);
    _p      = config.getDouble("primaryProtonGun.p", pPrimaryProton );
    
    _czmin  = config.getDouble("primaryProtonGun.czmin",  -1);
    _czmax  = config.getDouble("primaryProtonGun.czmax",  1);
    _phimin = config.getDouble("primaryProtonGun.phimin", 0. );
    _phimax = config.getDouble("primaryProtonGun.phimax", twopi );
    _tmin   = config.getDouble("primaryProtonGun.tmin", 700 );
    _tmax   = config.getDouble("primaryProtonGun.tmax", 1694 );

    _beamDisplacementOnTarget = config.getHep3Vector("beamDisplacementOnTarget");   
    _zOffset = config.getDouble("targetPS_halfLength",80.);
    _stdDev = config.getDouble("primaryProtonGun.stdDev" );
	
    _dcz  = (  _czmax -  _czmin);
    _dphi = ( _phimax - _phimin);
    _dt   = (   _tmax -   _tmin);
    
    edm::Service<edm::TFileService> tfs;
    _primaryProtonKE = tfs->make<TH1D>( "primaryProtonKE", "Primary Proton Kinetic Energy", 10, 7000.,9000.);
    _primaryProtonMomentumMeV = tfs->make<TH1D>( "primaryProtonMomentumMeV", "Primary Proton Momentum in MeV", 10, 7000.,9000.);
    _primaryProtonKEZoom = tfs->make<TH1D>( "primaryProtonEZoom", "Primary Proton Kinetic Energy (zoom)", 200, 7000.,9000.);

  }
  
  PrimaryProtonGun::~PrimaryProtonGun(){
  }
  
  void PrimaryProtonGun::generate( ToyGenParticleCollection& genParts ){
    
    if (!_doPrimaryProt) return;

    double r = fabs(CLHEP::RandGaussQ::shoot(0.0, _stdDev));
    double phi = twopi*RandFlat::shoot();
    
    //
    //last piece moves the gun to fire at the upstream end of the 
    //proton target.  When we have the proton beam exiting the solenoid
    //this will all be replaced.
    Hep3Vector pos( _beamDisplacementOnTarget.x() + r*cos(phi), 
		    _beamDisplacementOnTarget.y() + r*sin(phi), 
		    _beamDisplacementOnTarget.z() + _zOffset);
    
    // Random direction.
    const double cz   = _czmin  + _dcz*RandFlat::shoot();
    const double phi2 = _phimin + _dphi*RandFlat::shoot();
    
    // This should be an exponential decay.
    const double time = _tmin   +   _dt*RandFlat::shoot();

    // Derived quantities.
    const double sz   = safeSqrt(1.- cz*cz);
    double e = sqrt( _p*_p +m*m);
    
    double primaryProtonKineticEnergy = e - m;
    
    _primaryProtonKE->Fill(primaryProtonKineticEnergy);
    
    // direction of Primary Proton Gun. "-_p*cz" because Z axis is in opposite direction
    HepLorentzVector mom( _p*sz*cos(phi2), _p*sz*sin(phi2), -_p*cz, e); 

    // Add the proton to  the list.
    genParts.push_back( ToyGenParticle( PDGCode::p_plus, GenId::primaryProtonGun, pos, mom, time));

  }

}
