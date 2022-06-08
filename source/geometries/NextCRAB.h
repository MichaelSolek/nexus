//
// Created by Michael Solek on 6/8/22
//

#ifndef NEXT_CRAB_H
#define NEXT_CRAB_H
#include "GeometryBase.h"

class G4GenericMessenger;

namespace nexus {
    class NextCRAB: public GeometryBase
    {
        public:
            // Constructor
            NextCRAB();
            // Destructor
            ~NextCRAB();

            // Return vertex within region <region> of the chamber
            virtual void Construct();
            virtual G4ThreeVector GenerateVertex(const G4String& region) const;
    
        private:
            /// Messenger for the definition of control commands
            G4GenericMessenger* msg_;
            G4double Lab_size;
            G4double chamber_diam   ;
            G4double chamber_length ;
            G4double chamber_thickn ;
            G4double SourceEn_offset ;
            G4double SourceEn_diam   ;
            G4double SourceEn_length ;
            G4double SourceEn_thickn ;
            G4double SourceEn_holedia ;
            G4double gas_pressure_;
            G4ThreeVector vtx_;
            G4double Active_diam;
            G4double Active_length;

            G4ThreeVector vertex;
            void ConstructLab();
            void PlaceVolumes();
            void AssignVisuals();
            void PrintParam();
    };
}

#endif // NEXT_CRAB_H