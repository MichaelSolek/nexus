//
// Created by ilker on 9/2/21.
//

#include "CRAB.h"

#include "MaterialsList.h"
#include "OpticalMaterialProperties.h"
#include "UniformElectricDriftField.h"
#include "IonizationSD.h"
#include "FactoryBase.h"
#include "XenonProperties.h"
#include <G4Box.hh>
#include <G4SubtractionSolid.hh>
#include <G4GenericMessenger.hh>
#include <G4Tubs.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4SDManager.hh>
#include <G4NistManager.hh>
#include <G4VisAttributes.hh>
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include <G4UnionSolid.hh>
#include <G4MultiUnion.hh>
#include <G4ExtrudedSolid.hh>
#include <G4IntersectionSolid.hh>

#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>


namespace nexus{
    using namespace CLHEP;
    REGISTER_CLASS(CRAB, GeometryBase)


            CRAB::CRAB():
            GeometryBase(),
            msg_(nullptr),
            Lab_size(3. *m),
            chamber_diam   (19.24 * 2.54 * cm),  //based on model dimensions, not drawings, may need revision. Seems odd to not be 19.25
            chamber_length (50.125 * 2.54 * cm),//based on model dimensions, not drawings, may need revision
            chamber_thickn (0.76 * 25.4 * mm),         //based on model dimensions, not drawings, may need revision. Seems odd to not be 0.75
            Active_diam    (482.7 * mm),               //based on inner diameter of XON POLY OUTER WRAP.pdf drawing, may need revision
            Active_length  (1051. * mm),               //based on length of staves in drawings, may need revision
            SourceEn_offset (5.2 *cm),
            SourceEn_diam   (1. * cm),
            SourceEn_length (1 * cm),
            SourceEn_thickn (2. * mm),
            SourceEn_holedia (2. * mm),
            gas_pressure_(10. * bar),
            vtx_(0,0,0)


    {
        msg_ = new G4GenericMessenger(this, "/Geometry/CRAB/","Control commands of geometry of CRAB TPC");
        G4GenericMessenger::Command&  Pressure_cmd =msg_->DeclarePropertyWithUnit("gas_pressure","bar",gas_pressure_,"Pressure of Gas");
        Pressure_cmd.SetParameterName("XeNonPressure", false);


        G4GenericMessenger::Command&  chamber_diam_cmd =msg_->DeclarePropertyWithUnit("chamber_diam","cm",chamber_diam,"ChamberDiam");
        chamber_diam_cmd.SetParameterName("chamberdiam", false);

        G4GenericMessenger::Command&  chamber_length_cmd =msg_->DeclarePropertyWithUnit("chamber_length","cm",chamber_length,"Chamberlength");
        chamber_length_cmd.SetParameterName("chamberlength", false);

        G4GenericMessenger::Command&  chamber_thickn_cmd =msg_->DeclarePropertyWithUnit("chamber_thickn","mm",chamber_thickn,"Chamberthickn");
        chamber_thickn_cmd .SetParameterName("chamberthickn", false);


        G4GenericMessenger::Command&  source_position_cmd =msg_->DeclarePropertyWithUnit("SourcePosition","cm",vtx_,"vtx");
        source_position_cmd.SetParameterName("vtx", false);

        G4GenericMessenger::Command&  SourceEn_offset_cmd =msg_->DeclarePropertyWithUnit("SourceEn_offset","cm",SourceEn_offset,"SourceEnDiam");
        SourceEn_offset_cmd.SetParameterName("SourceEnoffset", false);

        G4GenericMessenger::Command&  SourceEn_diam_cmd =msg_->DeclarePropertyWithUnit("SourceEn_diam","cm",SourceEn_diam,"SourceEnDiam");
        SourceEn_diam_cmd.SetParameterName("SourceEndiam", false);

        G4GenericMessenger::Command&  SourceEn_length_cmd =msg_->DeclarePropertyWithUnit("SourceEn_length","cm",SourceEn_length,"SourceEnlength");
        SourceEn_length_cmd.SetParameterName("SourceEnlength", false);

        G4GenericMessenger::Command&  SourceEn_holedi_cmd =msg_->DeclarePropertyWithUnit("SourceEn_holedi","cm",SourceEn_holedia,"SourceEnholedi");
        SourceEn_holedi_cmd.SetParameterName("SourceEnholedi", false);

        G4GenericMessenger::Command&  Active_diam_cmd =msg_->DeclarePropertyWithUnit("Active_diam","cm",Active_diam,"ActiveDiam");
        Active_diam_cmd.SetParameterName("Activediam", false);

        G4GenericMessenger::Command&  Active_length_cmd =msg_->DeclarePropertyWithUnit("Active_length","cm",Active_length,"Activelength");
        Active_length_cmd.SetParameterName("Activelength", false);

    }

    CRAB::~CRAB()
    {
        //delete msg_;
    }
    
    G4MultiUnion* getReflectivePanelArray();
    G4SubtractionSolid* getFieldCageStaves();
    G4MultiUnion* getBufferDriftCopperRingAssembly();
    G4MultiUnion* getChamber(G4double chamber_diam, G4double chamber_thickn, G4double chamber_length);

    void CRAB::Construct(){

        //Define required unimplemented materials
        G4Element* elCu = new G4Element("Copper", "Cu", 29, 63.546*g/mole);
        G4Material* Copper = new G4Material("Copper", 8.960*g/cm3, 1);
        Copper->AddElement(elCu, 1);
        G4Material* teflon = G4NistManager::Instance()->FindOrBuildMaterial("G4_TEFLON");

        //Constructing Lab Space
        G4String lab_name="LAB";
        G4Box * lab_solid_volume = new G4Box(lab_name,Lab_size/2,Lab_size/2,Lab_size/2);
        G4LogicalVolume * lab_logic_volume= new G4LogicalVolume(lab_solid_volume,G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR"),lab_name);
        new G4PVPlacement(0, G4ThreeVector(),lab_logic_volume,lab_logic_volume->GetName(),0,false,0, true);

        //lab_logic_volume->SetVisAttributes(G4VisAttributes::Invisible);


        //Creating the Steel Cylinder that we use
        G4Tubs* chamber_solid = new G4Tubs("CHAMBER", 0, (chamber_diam/2. + chamber_thickn),(chamber_length/2. + chamber_thickn), 0.,twopi);
        G4LogicalVolume* chamber_logic = new G4LogicalVolume(chamber_solid,materials::Steel(), "CHAMBER");
        new G4PVPlacement(0, G4ThreeVector(0., 0., 0.) ,chamber_logic, chamber_solid->GetName(), lab_logic_volume, false, 0,true);

        // Placing the gas in the chamber
        G4Tubs* gas_solid = new G4Tubs("GAS", 0., chamber_diam/2., chamber_length/2., 0., twopi);
        G4Material* gxe = materials::GXe(gas_pressure_);
        gxe->SetMaterialPropertiesTable(opticalprops::GXe(gas_pressure_, 68));
        G4LogicalVolume* gas_logic = new G4LogicalVolume(gas_solid, gxe, "GAS");
        new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), gas_logic, gas_solid->GetName(),chamber_logic, false, 0, true);

        // Defining regions
        // Drift region
        G4Tubs* drift_solid = new G4Tubs("DRIFT", 0., Active_diam/2., (807.157 * mm)/2., 0., twopi); //length is model distance from edge of cathode to edge of nearest EL ring
        G4LogicalVolume* drift_logic = new G4LogicalVolume(drift_solid, gxe, "DRIFT");
        new G4PVPlacement(0, G4ThreeVector(0., 0., -15.8094 * mm ), drift_logic, drift_solid->GetName(),gas_logic, false, 0, true); // offset calculated based off model
        // EL region
        G4double el_gap_distance = 19.628*mm; // This is ring width (13mm) plus actual gap distance (6.628mm)
        G4double el_cathode_zpos = -427.8875*mm, el_anode_zpos = el_cathode_zpos - el_gap_distance;
        G4Tubs* EL_solid = new G4Tubs("EL_GAP", 0., (382. * mm)/2., el_gap_distance/2., 0, twopi);
        G4LogicalVolume* EL_logic = new G4LogicalVolume(EL_solid, gxe, "EL_GAP");
        new G4PVPlacement(0, G4ThreeVector(0., 0., (el_cathode_zpos + el_anode_zpos)/2), EL_logic, EL_solid->GetName(), gas_logic, false, 0, true);
        // Beyond EL region
        G4Tubs* beyondEL_solid = new G4Tubs("BEYOND_EL", 0., Active_diam/2., (7.022 * 25.4 * mm)/2., 0., twopi);
        G4LogicalVolume* beyondEL_logic = new G4LogicalVolume(beyondEL_solid, gxe, "BEYOND_EL");
        new G4PVPlacement(0, G4ThreeVector(0., 0., -545.3015 * mm), beyondEL_logic, beyondEL_solid->GetName(), gas_logic, false, 0, true);

        // EL rings
        G4Tubs* el_cathode_ring_solid = new G4Tubs("EL_CATHODE_RING", (382. * mm)/2., (437. * mm)/2., (13. * mm)/2., 0., twopi);
        G4Tubs* el_anode_ring_solid = new G4Tubs("EL_ANODE_RING", (382. * mm)/2., (437. * mm)/2., (13. * mm)/2., 0., twopi);
        G4LogicalVolume* el_cathode_ring_logic = new G4LogicalVolume(el_cathode_ring_solid, materials::Steel(), "EL_CATHODE_RING");
        G4LogicalVolume* el_anode_ring_logic = new G4LogicalVolume(el_anode_ring_solid, materials::Steel(), "EL_ANODE_RING");
        new G4PVPlacement(0, G4ThreeVector(0., 0., el_anode_zpos), el_anode_ring_logic, el_anode_ring_solid->GetName(), gas_logic, false, 0, true);
        new G4PVPlacement(0, G4ThreeVector(0., 0., el_cathode_zpos), el_cathode_ring_logic, el_cathode_ring_solid->GetName(), gas_logic, false, 0, true);

        // Cathode ring
        G4double cathode_ring_zpos = 392.7695 * mm;
        G4Tubs* cathode_solid = new G4Tubs("CATHODE", (343. * mm)/2., (373. * mm)/2., (10. * mm)/2., 0., twopi);
        G4LogicalVolume* cathode_logic = new G4LogicalVolume(cathode_solid, materials::Steel(), "CATHODE");
        new G4PVPlacement(0, G4ThreeVector(0., 0., cathode_ring_zpos), cathode_logic, cathode_solid->GetName(), gas_logic, false, 0, true);

        // Reflective panel array
        G4MultiUnion* reflectors_solid = getReflectivePanelArray();
        G4LogicalVolume* reflectors_logic = new G4LogicalVolume(reflectors_solid, teflon, "REFLECTIVE_PANELS");
        new G4PVPlacement(0, G4ThreeVector(0., 0., -250.0455 * mm), reflectors_logic, reflectors_solid->GetName(), gas_logic, false, 0, true); // Note: calculating this from the cathode decreases z-component by ~0.1 mm

        // Field cage staves
        G4SubtractionSolid* fieldCageStaves_solid = getFieldCageStaves();
        G4LogicalVolume* fieldCageStaves_logic = new G4LogicalVolume(fieldCageStaves_solid, materials::Steel(), "FIELD_CAGE_STAVES");
        new G4PVPlacement(0, G4ThreeVector(0,0,121.2685*mm), fieldCageStaves_logic, fieldCageStaves_logic->GetName(), gas_logic, false, 0, true);
    
        // Buffer-Drift Copper Ring Assembly
        G4MultiUnion* bufferDriftCopperRingAssembly_solid = getBufferDriftCopperRingAssembly();
        G4LogicalVolume* bufferDriftCopperRingAssembly_logic = new G4LogicalVolume(bufferDriftCopperRingAssembly_solid, Copper, "BUFFER-DRIFT_COPPER_RING_ASSEMBLY"); 
        new G4PVPlacement(0, G4ThreeVector(0,0,0), bufferDriftCopperRingAssembly_logic, bufferDriftCopperRingAssembly_logic->GetName(), gas_logic, false, 0, true);

        // Poly insulation assembly
        G4MultiUnion* polyInsulationAssembly_solid = new G4MultiUnion("POLY_INSULATION_ASSEMBLY");
        G4Tubs* innerPolyInsulation = new G4Tubs("POLY_INSULATION_INNER", 470/2*mm, (470/2+6.35)*mm, 960/2*mm, twopi*(1-0.9933)/2, twopi*0.9933);
        G4Tubs* outerPolyInsulation = new G4Tubs("POLY_INSULATION_OUTER", 482.7/2*mm, (482.6/2+6.35)*mm, 960/2*mm, twopi*(1-0.9933)/2, twopi*0.9933);
        G4RotationMatrix* innerPolyRot = new G4RotationMatrix();
        innerPolyRot->rotateZ(twopi/8);
        G4RotationMatrix* outerPolyRot = new G4RotationMatrix();
        outerPolyRot->rotateZ(twopi*5/8);
        polyInsulationAssembly_solid->AddNode(innerPolyInsulation, G4Transform3D(*innerPolyRot, G4ThreeVector(0,0,0)));
        polyInsulationAssembly_solid->AddNode(outerPolyInsulation, G4Transform3D(*outerPolyRot, G4ThreeVector(0,0,0)));
        polyInsulationAssembly_solid->Voxelize();
        G4LogicalVolume* polyInsulationAssembly_logic = new G4LogicalVolume(polyInsulationAssembly_solid, materials::HDPE(), "POLY_INSULATION_ASSEMBLY");
        new G4PVPlacement(0, G4ThreeVector(0,0,166.7685*mm), polyInsulationAssembly_logic, polyInsulationAssembly_logic->GetName(), gas_logic, false, 0, true);

		// Defining electric fields
		// Drift region
		UniformElectricDriftField* drift_field = new UniformElectricDriftField();
        drift_field->SetCathodePosition(cathode_ring_zpos);
        drift_field->SetAnodePosition(el_cathode_zpos);
        drift_field->SetDriftVelocity(1.*mm/microsecond); // Testing value borrowed from Next100FieldCage.cc
        drift_field->SetTransverseDiffusion(1.*mm/sqrt(cm)); //Testing value borrowed from Next100FieldCage.cc
        drift_field->SetLongitudinalDiffusion(.3*mm/sqrt(cm)); // testing value borrowed from Next100FieldCage.cc
        G4Region* drift_region = new G4Region("DRIFT_REGION");
        drift_region->SetUserInformation(drift_field);
        drift_region->AddRootLogicalVolume(drift_logic);
		// EL region
        UniformElectricDriftField* el_field = new UniformElectricDriftField();
        el_field->SetCathodePosition(el_cathode_zpos);
        el_field->SetAnodePosition(el_anode_zpos);
        el_field->SetDriftVelocity(2.5*mm/microsecond); // Testing value borrowed from Next100FieldCage.cc
        el_field->SetTransverseDiffusion(0.*mm/sqrt(cm)); // Testing value borrowed from Next100FieldCage.cc
        el_field->SetLongitudinalDiffusion(0.*mm/sqrt(cm)); //Testing Value borrowed from Next100FieldCage.cc
        el_field->SetLightYield(10./cm);
	//el_field->SetLightYield(XenonELLightYield(34.5*kilovolt/cm, 10.*bar)); // EL field strength borrowed from Next100FieldCage.cc, estimated to be 22 kV
        G4Region* el_region = new G4Region("EL_REGION");
        el_region->SetUserInformation(el_field);
        el_region->AddRootLogicalVolume(EL_logic);

        // Radioactive Source Encloser
        //Source
        //G4Tubs* SourceHolChamber_solid =new G4Tubs("SourceHolChamber", SourceEn_holedia/2, (SourceEn_diam/2. + SourceEn_thickn),(SourceEn_length/2. + SourceEn_thickn),0,twopi);
        //G4Tubs* SourceHolChamberBlock_solid =new G4Tubs("SourceHolChBlock",0,(SourceEn_holedia/2),( SourceEn_thickn/2), 0.,twopi);


        //G4VSolid *SourceHolderGas_solid= new G4SubtractionSolid("SourceHolderGas",Source_Chm_solid,SourceHolder_solid);


        //G4Material* gxe = materials::GXe(gas_pressure_);
        //gxe->SetMaterialPropertiesTable(opticalprops::GXe(gas_pressure_, 68));

        //G4LogicalVolume* SourceHolderGas_logic = new G4LogicalVolume(SourceHolderGas_solid, gxe, "SourceHolderGAS_logic");

        //G4LogicalVolume* SourceHolChamber_logic = new G4LogicalVolume(SourceHolChamber_solid,materials::Steel(), "SourceHolChamber_logic");
        //G4LogicalVolume* SourceHolChamberBlock_logic = new G4LogicalVolume(SourceHolChamberBlock_solid,materials::Steel(), "SourceHolChBlock_logic");

        //Rotation Matrix
        G4RotationMatrix* rm = new G4RotationMatrix();
        rm->rotateY(90.*deg);
        //rm->rotateX(-45.*deg);

        //new G4PVPlacement(rm, G4ThreeVector(-SourceEn_offset,-SourceEn_offset,-SourceEn_offset), SourceHolChamber_logic, SourceHolChamber_solid->GetName(),gas_logic, false, 0, true);
        //new G4PVPlacement(rm, G4ThreeVector(-SourceEn_offset-SourceEn_length/2,-SourceEn_offset-SourceEn_length/2,-SourceEn_offset), SourceHolChamberBlock_logic, SourceHolChamberBlock_solid->GetName(),gas_logic, false, 0, true);
        //new G4PVPlacement(rm, G4ThreeVector(-SourceEn_offset,0,0), SourceHolChamber_logic, SourceHolChamber_solid->GetName(),gas_logic, false, 0, true);
        //new G4PVPlacement(rm, G4ThreeVector(-SourceEn_offset-SourceEn_length/2,0,0), SourceHolChamberBlock_logic, SourceHolChamberBlock_solid->GetName(),gas_logic, false, 0, true);


        // Define this volume as an ionization sensitive detector
        IonizationSD* sensdet_fc = new IonizationSD("/CRAB/DRIFT");
        IonizationSD* sensdet_el = new IonizationSD("/CRAB/EL_GAP");
        IonizationSD* sensdet_bel = new IonizationSD("/CRAB/BEYOND_EL");
        //IonizationSD* sensdet = new IonizationSD("/CRAB/GAS");
        drift_logic->SetSensitiveDetector(sensdet_fc);
        EL_logic->SetSensitiveDetector(sensdet_el);
        beyondEL_logic->SetSensitiveDetector(sensdet_bel);
        G4SDManager::GetSDMpointer()->AddNewDetector(sensdet_fc);
        G4SDManager::GetSDMpointer()->AddNewDetector(sensdet_el);
        G4SDManager::GetSDMpointer()->AddNewDetector(sensdet_bel);

        AssignVisuals();
        //this->SetLogicalVolume(lab_logic_volume);
        this->SetLogicalVolume(chamber_logic);

    }


    void CRAB::AssignVisuals() {
        // Define references to geometry needing visualization settings
        G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
        G4LogicalVolume* Chamber = lvStore->GetVolume("CHAMBER");
        G4LogicalVolume* Lab = lvStore->GetVolume("LAB");
        G4LogicalVolume* Active = lvStore->GetVolume("DRIFT");
        G4LogicalVolume* EL = lvStore->GetVolume("EL_GAP");
        G4LogicalVolume* Beyond = lvStore->GetVolume("BEYOND_EL");
        G4LogicalVolume* Gas = lvStore->GetVolume("GAS");
        G4LogicalVolume* Cathode = lvStore->GetVolume("CATHODE");
        G4LogicalVolume* EL_Cathode = lvStore->GetVolume("EL_CATHODE_RING");
        G4LogicalVolume* EL_Anode = lvStore->GetVolume("EL_ANODE_RING");
        G4LogicalVolume* Panels = lvStore->GetVolume("REFLECTIVE_PANELS");
        G4LogicalVolume* FieldCageStaves = lvStore->GetVolume("FIELD_CAGE_STAVES");
        G4LogicalVolume* CopperRings = lvStore->GetVolume("BUFFER-DRIFT_COPPER_RING_ASSEMBLY");
        G4LogicalVolume* HDPEWrap = lvStore->GetVolume("POLY_INSULATION_ASSEMBLY");

        // Define and configure visual attributes
        G4VisAttributes *WireframeVa=new G4VisAttributes(G4Colour(1,1,1));
        WireframeVa->SetForceWireframe(true);

        G4VisAttributes *LabVa=new G4VisAttributes(G4Colour(2,2,2));
        LabVa->SetForceWireframe(false);

        G4VisAttributes *ActiveVa=new G4VisAttributes(G4Colour(1,1,1));
        ActiveVa->SetForceCloud(false);

        G4VisAttributes *GasVa=new G4VisAttributes(G4Colour(2,2,2));
        GasVa->SetForceWireframe(true);

        G4VisAttributes *SteelVa = new G4VisAttributes(G4Colour(1,1,1));
        SteelVa->SetForceSolid(true);

        G4VisAttributes *CopperVa = new G4VisAttributes(G4Colour(0.71875, 0.449219, 0.199219)); //Copper color, RGB #b87333
        CopperVa->SetForceSolid(true);

        G4VisAttributes *TeflonVa = new G4VisAttributes(G4Color(0.820313, 0.703125, 0.546875)); //Brown color, RBG #d2b48c
        TeflonVa->SetForceSolid(true);

        //Assign visual attributes
        Lab->SetVisAttributes(G4VisAttributes::GetInvisible());
        Chamber->SetVisAttributes(WireframeVa);
        HDPEWrap->SetVisAttributes(G4VisAttributes::GetInvisible());
        Cathode->SetVisAttributes(SteelVa);
        EL_Cathode->SetVisAttributes(SteelVa);
        EL_Anode->SetVisAttributes(SteelVa);
        Panels->SetVisAttributes(TeflonVa);
        FieldCageStaves->SetVisAttributes(SteelVa);
        CopperRings->SetVisAttributes(CopperVa);
        Active->SetVisAttributes(ActiveVa);
        Beyond->SetVisAttributes(ActiveVa);
        EL->SetVisAttributes(ActiveVa);
        Gas->SetVisAttributes(GasVa);
    }

    void CRAB::PrintParam() {

    }

    G4ThreeVector CRAB::GenerateVertex(const G4String& region) const
    {

        if(!(region=="LAB" || region=="GAS" || region=="DRIFT" || region=="SourceHolChamber")){

            G4Exception("[CRAB]", "GenerateVertex()", FatalException,
                        "Unknown vertex generation region.");
        }
        return vtx_;
    }

    G4MultiUnion* getReflectivePanelArray() {
        G4MultiUnion* panel_array = new G4MultiUnion("REFLECTIVE_PANELS");
        G4RotationMatrix* rm[12];
        G4Transform3D tr[12][4];
        G4double initialRotation = 19.60 * deg;
        G4double radius = (370.592/2. + (5.004+6.756)/2.)*mm;//~191.176mm
        
        std::vector<G4TwoVector> polygon = {G4TwoVector(9.85*mm,11.76*mm/2.), G4TwoVector(4*mm,-0.876*mm), G4TwoVector(101.*mm/2.,-0.876*mm),
                                    G4TwoVector(98.318*mm/2.,-11.76*mm/2.), G4TwoVector(-98.317*mm/2.,-11.76*mm/2.), G4TwoVector(-101.*mm/2.,-0.876*mm),
                                    G4TwoVector(-4*mm,-0.876*mm), G4TwoVector(-9.85*mm,11.76*mm/2.)};
        for(int i=0; i<12; i++){
            rm[i] = new G4RotationMatrix();
            G4double rotation = initialRotation + i*30.*deg;
            rm[i]->rotateZ(rotation);
            for(int j=0; j<4; j++){
                G4double zpos, halfz;
                zpos = j*(252.+2.)*mm;
                halfz = 252.*mm/2.;
                if (j==3){
                    halfz = 245.*mm/2.; // last panels beyond cathode are a bit shorter
                    zpos += (17.814-2.-(252.-245.)/2.)*mm; // Adjust for gap across cathode and change in panel length
                }
                G4ThreeVector position = G4ThreeVector((-radius)*cos(90.*deg-rotation), radius*sin(90.*deg-rotation), zpos);
                tr[i][j] = G4Transform3D(*rm[i], position);
                G4ExtrudedSolid* panel = new G4ExtrudedSolid("PANEL_" + std::to_string(j+1) + "-" + std::to_string(i+1), polygon, halfz, G4TwoVector(0,0), 1, G4TwoVector(0,0), 1);
                panel_array->AddNode(panel, tr[i][j]);
            }
        }

        panel_array->Voxelize();
        return panel_array;
        }

    G4SubtractionSolid* getFieldCageStaves(){
        // Define shapes used  to construct the field cage staves
        G4MultiUnion* blankFieldCageStaves = new G4MultiUnion("BLANK_FIELD_CAGE_STAVES");
        G4MultiUnion* subRings = new G4MultiUnion("SUBTRACTION_RINGS");
        G4SubtractionSolid* baseFieldCageStaves;
        G4SubtractionSolid* fieldCageStaves;

        G4RotationMatrix* rm[12];
        G4Transform3D tr[12];
        std::vector<G4TwoVector> polygon = {G4TwoVector(30*mm,50*mm), G4TwoVector(30*mm, 0), G4TwoVector(4.54585*mm, 0), G4TwoVector(10.5*mm, 7*mm),
                        G4TwoVector(-10.5*mm, 7*mm), G4TwoVector(-4.54585*mm, 0), G4TwoVector(-30*mm, 0), G4TwoVector(-30*mm, 50*mm)};
        G4double outerEdgeRadius = 235 * mm;
        G4double grooveTopRadius = 202.8 * mm;
        G4double staveWidth = 60 * mm;
        G4double staveHeight = 44.7 * mm;
        G4double staveLength = 1051 * mm;
        G4double initialRotation = 19.60 * deg;
        G4double placementRadius = (185.296 + 5.004) * mm;
        //placementRadius *= 1.5;

        // Build the blank staves with no notches and place them. No further nodes required, so voxelize it.
        for(int i=0; i<12; i++){
            rm[i] = new G4RotationMatrix();
            // These must be placed in specific positions as certain staves require certain unique modifications, unlike the reflective panels.
            G4double rotation = initialRotation + (i-4)*30*deg;
            rm[i]->rotateZ(rotation);
            G4ThreeVector position = G4ThreeVector(0,0,0);//(-placementRadius*cos(90.*deg-rotation), placementRadius*sin(90.*deg-rotation), 0);
            tr[i] = G4Transform3D(*rm[i], position);
            G4ExtrudedSolid* staveOutline = new G4ExtrudedSolid("STAVE_OUTLINE_" + std::to_string(i+1), polygon, staveLength/2, G4TwoVector(0,0), 1, G4TwoVector(0,0), 1);
            G4Tubs* staveBase = new G4Tubs("STAVE_BASE_" + std::to_string(i+1), outerEdgeRadius-(50*mm), outerEdgeRadius, (staveLength/2) + 10*mm, 0, twopi); //twopi*23/24, twopi/12);
            //This intesection solid needs a transformation to move the outline so it can properly intersect
            G4ThreeVector intersectionPos = G4ThreeVector(outerEdgeRadius-staveHeight,0,0);
            G4RotationMatrix* intersectionRot = new G4RotationMatrix();
            intersectionRot->rotateZ(90*deg);
            G4IntersectionSolid* ungroovedStave = new G4IntersectionSolid("STAVE_UNGROOVED_" + std::to_string(i+1), staveBase, staveOutline, intersectionRot, intersectionPos);
            blankFieldCageStaves->AddNode(ungroovedStave, tr[i]);
            //fieldCageStaves->AddNode(staveBase, G4Transform3D(G4RotationMatrix(), G4ThreeVector(0,0,0)));
            //fieldCageStaves->AddNode(staveOutline, tr[i]);
        }
        blankFieldCageStaves->Voxelize();
    
        // Make a set of rings to  use in a subtraction to cut all the notches that appear in all staves
        // 4 widely spaced notches towards right end of design drawing
        for(int i=0; i<4; i++){
            G4double zpos = (1051*mm)/2. - 69*mm - i*48*mm;
            G4Tubs* ring = new G4Tubs("SUBTRACTION_RING_" + std::to_string(i), grooveTopRadius, outerEdgeRadius + 5*mm, 10.5*mm/2., 0, twopi);
            //G4RotationMatrix* testRot = new G4RotationMatrix();
            //G4Transform3D testTrans = G4Transform3D(testRot, G4ThreeVector(0,0,0));
            subRings->AddNode(ring, G4Transform3D(G4RotationMatrix(), G4ThreeVector(0,0,zpos))); 
        }
        // 31 tightly spaced notches finishing near the left end of the design drawing
        for(int i=0; i<31; i++){
            G4double zpos = (1051*mm)/2. - 285*mm - i*24*mm;
            G4Tubs* ring = new G4Tubs("SUBTRACTION_RING_" + std::to_string(i), grooveTopRadius, outerEdgeRadius + 5*mm, 10.5*mm/2., 0, twopi);
            subRings->AddNode(ring, G4Transform3D(G4RotationMatrix(), G4ThreeVector(0,0,zpos)));
        }
        subRings->Voxelize();
    
        // Make the subtraction and return it
        baseFieldCageStaves = new G4SubtractionSolid("FIELD_CAGE_STAVE_BASES", blankFieldCageStaves, subRings);
    
        // Add the stave-specific subtractions
        G4Tubs* notch82_5 = new G4Tubs("STAVE_NOTCH_82_5_MM", grooveTopRadius, outerEdgeRadius + 5*mm, 82.5*mm/2., twopi*39/40, twopi/20);
        G4Tubs* notch58_5 = new G4Tubs("STAVE_NOTCH_58_5_MM", grooveTopRadius, outerEdgeRadius + 5*mm, 58.5*mm/2., twopi*39/40, twopi/20);
        G4Tubs* notch51 =   new G4Tubs("STAVE_NOTCH_51_MM",   grooveTopRadius, outerEdgeRadius + 5*mm, 51.0*mm/2., twopi*39/40, twopi/20);
        G4Tubs* notch106_5 =new G4Tubs("STAVE_NOTCH_106_5_MM",grooveTopRadius, outerEdgeRadius + 5*mm, 106.5*mm/2., twopi*39/40, twopi/20);
        G4MultiUnion* uniqueNotches = new G4MultiUnion("PER_STAVE_SUBTRACTIONS");
        // Placement is not predictably even; have to place each manually
        uniqueNotches->AddNode(notch58_5, G4Transform3D(*rm[0], G4ThreeVector(0,0,staveLength/2.-(663.8+58.5/2)*mm)));
        uniqueNotches->AddNode(notch82_5, G4Transform3D(*rm[1], G4ThreeVector(0,0,staveLength/2.-(591.8+82.5/2)*mm)));
        uniqueNotches->AddNode(notch82_5, G4Transform3D(*rm[2], G4ThreeVector(0,0,staveLength/2.-(519.8+82.5/2)*mm)));
        uniqueNotches->AddNode(notch82_5, G4Transform3D(*rm[3], G4ThreeVector(0,0,staveLength/2.-(447.8+82.5/2)*mm)));
        uniqueNotches->AddNode(notch82_5, G4Transform3D(*rm[4], G4ThreeVector(0,0,staveLength/2.-(375.8+82.5/2)*mm)));
        uniqueNotches->AddNode(notch58_5, G4Transform3D(*rm[5], G4ThreeVector(0,0,staveLength/2.-(327.8+58.5/2)*mm)));
        uniqueNotches->AddNode(notch51  , G4Transform3D(*rm[6], G4ThreeVector(0,0,staveLength/2.-(263.3+51.0/2)*mm)));
        uniqueNotches->AddNode(notch106_5,G4Transform3D(*rm[8], G4ThreeVector(0,0,staveLength/2.-(903.8+106.5/2)*mm)));
        uniqueNotches->AddNode(notch58_5, G4Transform3D(*rm[9], G4ThreeVector(0,0,staveLength/2.-(855.8+58.5/2)*mm)));
        uniqueNotches->AddNode(notch82_5, G4Transform3D(*rm[10], G4ThreeVector(0,0,staveLength/2.-(738.8+82.5/2)*mm)));
        uniqueNotches->AddNode(notch82_5, G4Transform3D(*rm[11], G4ThreeVector(0,0,(staveLength/2.-711.8+82.5/2)*mm)));
        uniqueNotches->Voxelize();

        fieldCageStaves = new G4SubtractionSolid("FIELD_CAGE_STAVES", baseFieldCageStaves, uniqueNotches);


        return fieldCageStaves;
    }

    // The origin of the resulting piece should be the origin of the overall space. Consider rewriting a bit to place origin relative to some
    // piece here like the first ring or something. Sort of odd to be the only piece placed at the origin, and those sorts of inconsistencies
    // make it hard to read and maintain the code.
    G4MultiUnion* getBufferDriftCopperRingAssembly(){
        // Define key dimensions and geometric components
        G4double innerRadius = 203 * mm;
        G4double outerRadius = 215 * mm;
        G4double thickness = 10 * mm;
        G4double zpos;
        G4Transform3D tr[35];
        G4MultiUnion* assembly = new G4MultiUnion("BUFFER-DRIFT_COPPER_RING_ASSEMBLY");

        // Place the tightly packed rings towards th/e EL end
        zpos = (232.356 + 46 - 50.125*25.4/2) * mm; // distance wall to staves + distance stave start to first ring - origin to wall
        for(int i=0; i<31; i++){
            zpos = (232.356 + 46 - 50.125*25.4/2) * mm + i*24*mm; // distance wall to staves + distance stave start to first ring - origin to wall, plus ring-specific offset
            G4Tubs* copperRing = new G4Tubs("COPPER_RING_" + std::to_string(i+1), innerRadius, outerRadius, thickness/2., 0, twopi);
            tr[i] = G4Transform3D(G4RotationMatrix(), G4ThreeVector(0,0,zpos));
            assembly->AddNode(*copperRing, tr[i]);        
        }

        // Place the widely spaced rings
        for(int i=31; i < 35; i++){
            zpos = (232.356 + 838 - 50.125*25.4/2) * mm + (i-31)*48*mm; // distance wall to staves + distance stave start to first wide-spaced ring - origin to wall
            G4Tubs* copperRing = new G4Tubs("COPPER_RING_" + std::to_string(i+1), innerRadius, outerRadius, thickness/2., 0, twopi);
            tr[i] = G4Transform3D(G4RotationMatrix(), G4ThreeVector(0,0,zpos));
            assembly->AddNode(*copperRing, tr[i]);
        }
        assembly->Voxelize();
        return assembly;
    }
}
