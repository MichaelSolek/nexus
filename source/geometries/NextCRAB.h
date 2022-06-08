// ----------------------------------------------------------------------------
// nexus | NextCRAB.h
//
// Main class that constructs the geometry of the NEXT-CRAB detector.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef NEXT_CRAB_H
#define NEXT_CRAB_H

#include "GeometryBase.h"

class G4GenericMessenger;


namespace nexus {

  class NextCRAB: public GeometryBase
  {
  public:
    /// Constructor
    NextCRAB();
    /// Destructor
    ~NextCRAB();

    /// Return vertex within region <region> of the chamber
    virtual G4ThreeVector GenerateVertex(const G4String& region) const;

    virtual void Construct();

  private:
    /// Messenger for the definition of control commands
    G4GenericMessenger* msg_;
  };

} // end namespace nexus

#endif
