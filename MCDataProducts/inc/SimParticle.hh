#ifndef MCDataProducts_SimParticle_hh
#define MCDataProducts_SimParticle_hh

//
// Information about particles created by Geant4.
//
// Original author Rob Kutschke
//
// Notes:
// 1) Internally G4 numbers tracks 1...N.  An earlier version of TrackingAction
//    renumbered them 0...(N-1); this was an artifact of the SimParticleCollection
//    class being a std::vector, which starts at 0. But now SimParticleCollection
//    is a cet::map_vector, so it is no longer necessary to do the renumbering.
//    Therefore the correct test to see if a particle has a parent is
//
// 2) The trackId, parentIds and daughterIds are all of the correct type to be
//    used to find the corresponding information in SimParticleCollection
//
// 3) The trackId, parentIds and daughterIds are all redundant now that the Ptr
//    versions are available.  We will get rid of them as soon as we check
//    backwards compatibility.

#include "Offline/DataProducts/inc/GenVector.hh"
#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "Offline/MCDataProducts/inc/GenParticle.hh"
#include "Offline/MCDataProducts/inc/ProcessCode.hh"
#include "Offline/DataProducts/inc/PDGCode.hh"

#include "canvas/Persistency/Common/Ptr.h"

#include "cetlib/map_vector.h"

#include <iostream>
#include <vector>

namespace mu2e {

  struct SimParticle {

    typedef cet::map_vector_key key_type;

    // Parameters used by GEANT4 to describe excited ions
    struct IonDetail {
      float excitationEnergy;
      short int floatLevelBaseIndex;
      IonDetail(float e, short int i) : excitationEnergy{e}, floatLevelBaseIndex{i} {}
      IonDetail(): excitationEnergy{0.}, floatLevelBaseIndex{0} {}
    };


    // A default c'tor is required for ROOT.
    SimParticle():
      _id(),
      _simStage(-1u),
      _parentSim(),
      _pdgId(),
      _genParticle(),
      _startPosition(),
      _startMomentum(),
      _startGlobalTime(0.),
      _startProperTime(0.),
      _startVolumeIndex(0),
      _startG4Status(),
      _creationCode(),
      _ion(),
      _endPosition(),
      _endMomentum(),
      _endGlobalTime(0.),
      _endProperTime(0.),
      _endVolumeIndex(0),
      _endG4Status(),
      _stoppingCode(),
      _preLastStepKE(-1.),
      _endKE(-1.),
      _nSteps(0),
      _trackLength(-1.),
      _daughterSims(){
    }

    SimParticle( key_type                       aid,
                 unsigned                       simStage,
                 art::Ptr<SimParticle> const&   aparentSim,
                 PDGCode::type                  apdgId,
                 art::Ptr<GenParticle> const&   agenParticle,
                 const CLHEP::Hep3Vector&       aposition,
                 const CLHEP::HepLorentzVector& amomentum,
                 float                         astartGlobalTime,
                 float                         astartProperTime,
                 unsigned                       astartVolumeIndex,
                 unsigned                       astartG4Status,
                 ProcessCode                    acreationCode,
                 IonDetail             const&   ion = IonDetail()
                 ):
      _id(aid),
      _simStage(simStage),
      _parentSim(aparentSim),
      _pdgId(apdgId),
      _genParticle(agenParticle),
      _startPosition(aposition),
      _startMomentum(amomentum),
      _startGlobalTime(astartGlobalTime),
      _startProperTime(astartProperTime),
      _startVolumeIndex(astartVolumeIndex),
      _startG4Status(astartG4Status),
      _creationCode(acreationCode),
      _ion(ion),
      _endPosition(),
      _endMomentum(),
      _endGlobalTime(0.),
      _endProperTime(0.),
      _endVolumeIndex(0),
      _endG4Status(),
      _stoppingCode(),
      _preLastStepKE(-1),
      _endKE(-1),
      _nSteps(0),
      _trackLength(-1.),
      _daughterSims()
    {}

    // Accept compiler generated d'tor, copy c'tor and assignment operator.

    void addEndInfo( CLHEP::Hep3Vector       aendPosition,
                     CLHEP::HepLorentzVector aendMomentum,
                     float                  aendGlobalTime,
                     float                  aendProperTime,
                     unsigned                aendVolumeIndex,
                     unsigned                aendG4Status,
                     ProcessCode             astoppingCode,
                     float                   endKE,
                     int                     nSteps,
                     float                  trackLength){
      _endPosition     = aendPosition;
      _endMomentum     = aendMomentum;
      _endGlobalTime   = aendGlobalTime;
      _endProperTime   = aendProperTime;
      _endVolumeIndex  = aendVolumeIndex;
      _endG4Status     = aendG4Status;
      _stoppingCode    = astoppingCode;
      _preLastStepKE   = -1.0;
      _endKE           = endKE;
      _nSteps          = nSteps;
      _trackLength     = trackLength;
    }

    void addDaughter( art::Ptr<SimParticle> const& p ){
      _daughterSims.push_back(p);
    }

    // Some Accessors/Modifier pairs.
    // The modifiers are needed by the event mixing code.

    // Key of this track within the SimParticleCollection.  See notes 1 and 2.
    key_type  id() const {return _id;}
    key_type& id()       { return _id;}

    unsigned simStage() const { return _simStage; }

    // The parent of this track; may be null.
    art::Ptr<SimParticle> const& parent() const { return _parentSim; }
    art::Ptr<SimParticle>&       parent()       { return _parentSim; }

    // work back through the genealogy to find the earliest representation of this
    // physical particle. This bridges the gaps created in staged MC production, where particles
    // are stopped at a detector volume.
    SimParticle const& originParticle() const {
      return selfParent() ? parent()->originParticle() : *this;
    }

    // The genparticle corresponding to this track; may be null.
    art::Ptr<GenParticle> const& genParticle() const { return _genParticle;}
    art::Ptr<GenParticle>&       genParticle()       { return _genParticle;}

    // Most members have only accessors

    // PDG particle ID code.
    PDGCode::type pdgId() const {return _pdgId;}

    // Where was this particle created: in the event generator or in G4?
    bool isSecondary()   const { return _parentSim.isNonnull(); }
    bool isPrimary()     const { return _genParticle.isNonnull() && _creationCode == ProcessCode::mu2ePrimary; }
    bool selfParent()    const { return _parentSim.isNonnull() && _creationCode == ProcessCode::mu2ePrimary; }

    // Some synonyms for the previous two accessors.
    bool hasParent()     const { return _parentSim.isNonnull(); }
    bool fromGenerator() const { return _genParticle.isNonnull(); }
    bool madeInG4()      const { return _genParticle.isNull();    }

    // Information at the start of the track.
    CLHEP::Hep3Vector startPosition()       const { return GenVector::Hep3Vec(_startPosition);}
    CLHEP::HepLorentzVector startMomentum() const { return GenVector::HepLorentzVec(_startMomentum);}
    XYZVectorD const& startPosXYZ() const { return _startPosition;}
    XYZTVectorF const& startMomXYZT() const { return _startMomentum;}
    float      startGlobalTime()  const { return _startGlobalTime;}
    float&     startGlobalTime()        { return _startGlobalTime;}
    float      startProperTime()  const { return _startProperTime;}
    unsigned    startVolumeIndex() const { return _startVolumeIndex;}
    unsigned    startG4Status()    const { return _startG4Status;}
    ProcessCode creationCode()     const { return _creationCode;   }
    bool isTruncated() const { return (_parentSim.isNull() && _creationCode != ProcessCode::mu2ePrimary); }

    // the following is for excited ions
    IonDetail const& ion()                      const { return _ion; }
    float           startExcitationEnergy()    const { return _ion.excitationEnergy;}
    int              startFloatLevelBaseIndex() const { return _ion.floatLevelBaseIndex;};

    // Information at the end of the track.
    CLHEP::Hep3Vector endPosition() const { return GenVector::Hep3Vec(_endPosition);}
    CLHEP::HepLorentzVector endMomentum() const { return GenVector::HepLorentzVec(_endMomentum);}
    XYZVectorD const& endPosXYZ() const { return _endPosition;}
    XYZTVectorF const& endMomXYZT() const { return _endMomentum;}
    float       endGlobalTime()  const { return _endGlobalTime; }
    float&      endGlobalTime()        { return _endGlobalTime; }
    float       endProperTime()  const { return _endProperTime; }
    unsigned     endVolumeIndex() const { return _endVolumeIndex;}
    unsigned     endG4Status()    const { return _endG4Status;   }
    ProcessCode  stoppingCode()   const { return _stoppingCode;  }
    float preLastStepKineticEnergy() const { return _preLastStepKE; }
    float        endKineticEnergy() const { return _endKE; }
    int          nSteps()         const { return _nSteps;        }
    float       trackLength()    const { return _trackLength;   }

    // SimParticle daughters of this track.
    std::vector<art::Ptr<SimParticle> > const& daughters()   const { return _daughterSims; }
    std::vector<art::Ptr<SimParticle> >&       daughters()         { return _daughterSims; }

    // SimParticle indices of daughters of this track.
    // DO NOT USE - this is an expensive (at run time) crutch for legacy code.
    std::vector<key_type>                      daughterIds() const;

    // Is the second half defined?
    bool endDefined() const { return _stoppingCode != ProcessCode::uninitialized; }

    // Modifiers;
    void setDaughterPtrs  ( std::vector<art::Ptr<SimParticle> > const& ptr){
      _daughterSims.clear();
      _daughterSims.reserve(ptr.size());
      _daughterSims.insert( _daughterSims.begin(), ptr.begin(), ptr.end() );
    }

    // Two older accessors that will soon be removed from the interface.

    // Index of the parent of this track; may be null.
    key_type                     parentId() const {
      return ( _parentSim.isNonnull()) ? key_type(_parentSim.key()) : key_type(0);
    }

    // Index into the container of generated tracks;
    // -1 if there is no corresponding generated track.
    int                          generatorIndex() const {
      return ( _genParticle.isNonnull()) ? _genParticle.key() : -1;
    }

  private:
    // G4 ID number of this track and of its parent.
    // See notes 1 and 2.
    key_type _id;

    // The sequential simulation stage number.
    // Simulations starting with GenParticles get simStage=0,
    // each next stage increments the number by one.
    unsigned _simStage;

    art::Ptr<SimParticle> _parentSim;

    // PDG particle ID code.  See note 1.
    PDGCode::type _pdgId;

    art::Ptr<GenParticle>  _genParticle;

    // Information at the start of the track.
    XYZVectorD                    _startPosition;
    XYZTVectorF             _startMomentum;
    float                    _startGlobalTime;
    float                    _startProperTime;
    unsigned                _startVolumeIndex;
    unsigned                _startG4Status;
    ProcessCode             _creationCode;
    IonDetail               _ion;

    // Information at the end of the track.
    XYZVectorD              _endPosition;
    XYZTVectorF                    _endMomentum;
    float                  _endGlobalTime;
    float                  _endProperTime;
    unsigned                _endVolumeIndex;
    unsigned                _endG4Status;
    ProcessCode             _stoppingCode;
    float                    _preLastStepKE;
    float                   _endKE;
    int                     _nSteps;
    float                   _trackLength;

    // SimParticle IDs of daughters of this track.
    std::vector<art::Ptr<SimParticle> > _daughterSims;

  };
  typedef cet::map_vector<mu2e::SimParticle> SimParticleCollection;
  typedef art::Ptr<mu2e::SimParticle> SimParticlePtr;
  typedef std::vector<mu2e::SimParticlePtr> SimParticlePtrCollection;

}

#endif /* MCDataProducts_SimParticle_hh */
