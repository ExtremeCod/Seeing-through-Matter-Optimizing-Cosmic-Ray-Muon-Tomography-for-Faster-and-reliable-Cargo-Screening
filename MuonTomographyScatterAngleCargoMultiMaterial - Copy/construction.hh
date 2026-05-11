         //|                                                    
        // |                                                    
       //  |  ||  ______  ||  ______       _______              
      //   |  || /        || /      \    //       \    ||      /
     //    |  ||/         ||/        \             |   ||     / 
    ///////|  ||          ||         |             |   ||    /  
   //      |  ||          ||         |    _________|   ||   /   
  //       |  ||          ||         |  //         \   ||  /    
 //        |  ||          ||         |  ||         |   || /     
//         |  ||          ||         |  \\_________/\  ||/       

#ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4GenericMessenger.hh"
#include "G4Tubs.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4Color.hh"
#include "G4VisAttributes.hh"
#include "G4SubtractionSolid.hh"
#include "G4SDManager.hh"

#include "detector.hh"

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction();
    ~DetectorConstruction();

    G4LogicalVolume *GetScoringVolume() const { return fScoringVolume; }

    virtual G4VPhysicalVolume *Construct();

    G4int f = 4;

    void RotateObjectY(G4double angleDeg);

    G4double u = 5;
    G4double p = 2;
private:
    G4LogicalVolume *logicDetector, *logicDetector2, *logicDetector3, *logicDetector4, *logicDetector5, *logicDetector6, *logicDetector7, *logicDetector8;
    virtual void ConstructSDandField();

    G4int nCols, nRows;

    G4Box *solidWorld, *solidRadiator, *solidDetector, *solidScintillator, *solidObj;
    G4LogicalVolume *logicWorld, *logicRadiator, *logicScintillator, *logicObj;
    G4VPhysicalVolume *physWorld, *physRadiator, *physDetector, *physScintillator, *physObj, *physDetector2, *physDetector3, *physDetector4, *physDetector5, *physDetector6, *physDetector7, *physDetector8;

    G4GenericMessenger *fMessenger;

    G4LogicalVolume *fScoringVolume;

    G4Material *Si02, *H20,  *Aerogel, *worldMat, *NaI, *leadMat, *U, *Wood_Aprox;
    G4Element *C, *Na, *I;

    G4RotationMatrix *rot;

    G4double curAngle;

    void DefineMaterial();

    void Cherenkov();
    void Scintillator();

    G4double xWorld, yWorld, zWorld;

    G4bool isCherenkov, isScintillator;

    G4OpticalSurface *mirrorSurface;
};

#endif