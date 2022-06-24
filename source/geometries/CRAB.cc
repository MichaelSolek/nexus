//
// Created by ilker on 9/2/21.
//

#include "CRAB.h"

#include "MaterialsList.h"
#include "OpticalMaterialProperties.h"
#include "UniformElectricDriftField.h"
#include "IonizationSD.h"
#include "FactoryBase.h"
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

#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>


namespace nexus{
    using namespace CLHEP;
    REGISTER_CLASS(CRAB, GeometryBase)


             CRAB::CRAB():
             GeometryBase(),
             msg_(nullptr),
             Lab_size(3. *m),
             chamber_diam   (19.24 * 2.54 * cm), //based on model dimensions, not drawings, may need revision. Seems odd to not be 19.25
             chamber_length (50.125 * 2.54 * cm),//based on model dimensions, not drawings, may need revision
             chamber_thickn (0.76 * 25.4 * mm),  //based on model dimensions, not drawings, may need revision. Seems odd to not be 0.75
	     Active_diam    (482.7 * mm),	 //based on inner diameter of XON POLY OUTER WRAP.pdf drawing, may need revision
	     Active_length  (1051. * mm),	 //based on length of staves in drawings, may need revision
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

    void CRAB::Construct(){


        //Constructing Lab Space
        G4String lab_name="LAB";
        G4Box * lab_solid_volume = new G4Box(lab_name,Lab_size/2,Lab_size/2,Lab_size/2);
        G4LogicalVolume * lab_logic_volume= new G4LogicalVolume(lab_solid_volume,G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR"),lab_name) ;

        //lab_logic_volume->SetVisAttributes(G4VisAttributes::Invisible);


        //Creating the Steel Cylinder that we use
        G4Tubs* chamber_solid = new G4Tubs("CHAMBER", chamber_diam/2, (chamber_diam/2. + chamber_thickn),(chamber_length/2. + chamber_thickn), 0.,twopi);
        G4LogicalVolume* chamber_logic = new G4LogicalVolume(chamber_solid,materials::Steel(), "CHAMBER"); //

	// Placing the gas in the chamber and define different field regions
        G4Tubs* gas_solid = new G4Tubs("GAS", 0., chamber_diam/2., chamber_length/2., 0., twopi);
        G4Tubs* fieldCage_solid = new G4Tubs("FIELD_CAGE", 0., Active_diam/2., (807.157 * mm)/2., 0., twopi); //length is model distance from edge of cathode to edge of nearest EL ring
	G4Tubs* EL_solid = new G4Tubs("EL_GAP", 0., (382. * mm)/2., (6.628 * mm)/2., 0, twopi);
	G4Tubs* beyondEL_solid = new G4Tubs("BEYOND_EL", 0., Active_diam/2., (7.022 * 25.4 * mm)/2., 0., twopi);

        // Radioactive Source Encloser
        //Source
        //G4Tubs* SourceHolChamber_solid =new G4Tubs("SourceHolChamber", SourceEn_holedia/2, (SourceEn_diam/2. + SourceEn_thickn),(SourceEn_length/2. + SourceEn_thickn),0,twopi);
	//G4Tubs* SourceHolChamberBlock_solid =new G4Tubs("SourceHolChBlock",0,(SourceEn_holedia/2),( SourceEn_thickn/2), 0.,twopi);


        //G4VSolid *SourceHolderGas_solid= new G4SubtractionSolid("SourceHolderGas",Source_Chm_solid,SourceHolder_solid);


        //G4Material* gxe = materials::GXe(gas_pressure_);
        //gxe->SetMaterialPropertiesTable(opticalprops::GXe(gas_pressure_, 68));

        G4Material* gxe = materials::GXe(gas_pressure_);

        gxe->SetMaterialPropertiesTable(opticalprops::GXe(gas_pressure_, 68));
        G4LogicalVolume* gas_logic = new G4LogicalVolume(gas_solid, gxe, "GAS");
        G4LogicalVolume* fieldCage_logic = new G4LogicalVolume(fieldCage_solid, gxe, "FIELD_CAGE");
	G4LogicalVolume* EL_logic = new G4LogicalVolume(EL_solid, gxe, "EL_GAP");
	G4LogicalVolume* beyondEL_logic = new G4LogicalVolume(beyondEL_solid, gxe, "BEYOND_EL");


        //G4LogicalVolume* SourceHolderGas_logic = new G4LogicalVolume(SourceHolderGas_solid, gxe, "SourceHolderGAS_logic");

        //G4LogicalVolume* SourceHolChamber_logic = new G4LogicalVolume(SourceHolChamber_solid,materials::Steel(), "SourceHolChamber_logic");
        //G4LogicalVolume* SourceHolChamberBlock_logic = new G4LogicalVolume(SourceHolChamberBlock_solid,materials::Steel(), "SourceHolChBlock_logic");



        //Rotation Matrix
        G4RotationMatrix* rm = new G4RotationMatrix();
        rm->rotateY(90.*deg);
        //rm->rotateX(-45.*deg);

        // Place the Volumes
        new G4PVPlacement(0, G4ThreeVector(),lab_logic_volume,lab_logic_volume->GetName(),0,false,0, true);
        new G4PVPlacement(0, G4ThreeVector(0., 0., 0.            ) ,chamber_logic, chamber_solid->GetName(), lab_logic_volume, false, 0,true);
        new G4PVPlacement(0, G4ThreeVector(0., 0., 0.            ), gas_logic, gas_solid->GetName(),chamber_logic, false, 0, true);
        new G4PVPlacement(0, G4ThreeVector(0., 0., -15.8094 * mm ), fieldCage_logic, fieldCage_solid->GetName(),gas_logic, false, 0, true); // offset calculated based off model
	new G4PVPlacement(0, G4ThreeVector(0., 0., -436.7015 * mm), EL_logic, EL_solid->GetName(), gas_logic, false, 0, true);
	new G4PVPlacement(0, G4ThreeVector(0., 0., -545.3015 * mm), beyondEL_logic, beyondEL_solid->GetName(), gas_logic, false, 0, true);
        //new G4PVPlacement(rm, G4ThreeVector(-SourceEn_offset,-SourceEn_offset,-SourceEn_offset), SourceHolChamber_logic, SourceHolChamber_solid->GetName(),gas_logic, false, 0, true);
        //new G4PVPlacement(rm, G4ThreeVector(-SourceEn_offset-SourceEn_length/2,-SourceEn_offset-SourceEn_length/2,-SourceEn_offset), SourceHolChamberBlock_logic, SourceHolChamberBlock_solid->GetName(),gas_logic, false, 0, true);
       // new G4PVPlacement(rm, G4ThreeVector(-SourceEn_offset,0,0), SourceHolChamber_logic, SourceHolChamber_solid->GetName(),gas_logic, false, 0, true);
        //new G4PVPlacement(rm, G4ThreeVector(-SourceEn_offset-SourceEn_length/2,0,0), SourceHolChamberBlock_logic, SourceHolChamberBlock_solid->GetName(),gas_logic, false, 0, true);


        // Define this volume as an ionization sensitive detector
        IonizationSD* sensdet_fc = new IonizationSD("/CRAB/FIELD_CAGE");
	IonizationSD* sensdet_el = new IonizationSD("/CRAB/EL_GAP");
	IonizationSD* sensdet_bel = new IonizationSD("/CRAB/BEYOND_EL");
        //IonizationSD* sensdet = new IonizationSD("/CRAB/GAS");
        fieldCage_logic->SetSensitiveDetector(sensdet_fc);
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
        // Chamber
        G4LogicalVolumeStore* lvStore = G4LogicalVolumeStore::GetInstance();
        G4LogicalVolume* Chamber = lvStore->GetVolume("CHAMBER");
        G4LogicalVolume* Lab = lvStore->GetVolume("LAB");
        G4LogicalVolume* Active = lvStore->GetVolume("FIELD_CAGE");
        G4LogicalVolume* Gas = lvStore->GetVolume("GAS");

        //G4LogicalVolume* SourceHolder = lvStore->GetVolume("SourceHolChamber_logic");
        //G4LogicalVolume* SourceHolderBlock = lvStore->GetVolume("SourceHolChBlock_logic");
        //G4VisAttributes *SourceHolderVa=new G4VisAttributes(G4Colour(2,2,2));
        G4VisAttributes *ChamberVa=new G4VisAttributes(G4Colour(1,1,1));
        G4VisAttributes *LabVa=new G4VisAttributes(G4Colour(2,2,2));
        G4VisAttributes *ActiveVa=new G4VisAttributes(G4Colour(1,1,1));
        G4VisAttributes *GasVa=new G4VisAttributes(G4Colour(2,2,2));
        ChamberVa->SetForceWireframe(true);
        //ChamberVa->SetLineStyle(G4VisAttributes::unbroken);
        Chamber->SetVisAttributes(ChamberVa);

        LabVa->SetForceWireframe(false);
        //GasVa->SetForceWireframe(false);
        GasVa->SetForceWireframe(true);
        ActiveVa->SetForceCloud(true);
        Active->SetVisAttributes(ActiveVa);
        Gas->SetVisAttributes(GasVa);
        //SourceHolderVa->SetForceSolid(true);
        //SourceHolderVa->SetForceSolid(true);

        //SourceHolder->SetVisAttributes(SourceHolderVa);
        //SourceHolderBlock->SetVisAttributes(SourceHolderVa);
        Lab->SetVisAttributes(G4VisAttributes::GetInvisible());

    }

    void CRAB::PrintParam() {

    }

    G4ThreeVector CRAB::GenerateVertex(const G4String& region) const
    {

        if(!(region=="LAB" || region=="GAS" || region=="FIELD_CAGE" || region=="SourceHolChamber")){

            G4Exception("[CRAB]", "GenerateVertex()", FatalException,
                        "Unknown vertex generation region.");
        }
        return vtx_;
    }


}
