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

#include "construction.hh"
#include "G4Trap.hh"

// IMPORTANT LINE IN GEANT4 WINDOW BEFORE RUNNING BEAM ON:
//THIS IS THE COMMAND: /process/had/rdm/thresholdForVeryLongDecayTime 1.0e+60 year

DetectorConstruction::DetectorConstruction()
{
    nCols = 10;
    nRows = 10;
    
    fMessenger = new G4GenericMessenger(this, "/detector/", "Detector Construction");

    fMessenger->DeclareProperty("nCols", nCols, "Number of Columns");
    fMessenger->DeclareProperty("nRows", nRows, "Number of Rows");
    fMessenger->DeclareProperty("isCherenkov", isCherenkov, "Starts cherenkov");
    fMessenger->DeclareProperty("isScintillator", isScintillator, "Starts scintillator");

    DefineMaterial();

    xWorld = 0.5 * m * f;
    yWorld = 0.5 * m * f;
    zWorld = 0.5 * m * f;

    isCherenkov = false;
    isScintillator = true;
}

DetectorConstruction::~DetectorConstruction()
{}

void DetectorConstruction::DefineMaterial()
{
    G4NistManager *nist = G4NistManager::Instance();

    Si02 = new G4Material("Si02", 2.201 * g/cm3, 2);
    Si02->AddElement(nist->FindOrBuildElement("Si"), 1);
    Si02->AddElement(nist->FindOrBuildElement("O"), 2);

    H20 = new G4Material("H20", 1.000 * g/cm3, 2);
    H20->AddElement(nist->FindOrBuildElement("H"), 2);
    H20->AddElement(nist->FindOrBuildElement("O"), 1);

    C = nist->FindOrBuildElement("C");

    Aerogel = new G4Material("Aerogel", 0.200 * g/cm3, 3);
    Aerogel->AddMaterial(Si02, 62.5 * perCent);
    Aerogel->AddMaterial(H20, 37.4 * perCent);
    Aerogel->AddElement(C, 0.1 * perCent);

    G4double energy[2] = {1.239841939 * eV/0.9, 1.239841939 * eV/0.2};
    G4double rindexAerogel[2] = {1.1, 1.1};
    G4double rindexWorld[2] = {1.0, 1.0};
    G4double rindexNaI[2] = {1.78, 1.78};
    G4double reflectivity[2] = {1.0, 1.0};

    G4MaterialPropertiesTable *mptAerogel = new G4MaterialPropertiesTable();
    mptAerogel->AddProperty("RINDEX", energy, rindexAerogel, 2);

    G4MaterialPropertiesTable *mptWorld = new G4MaterialPropertiesTable();
    mptWorld->AddProperty("RINDEX", energy, rindexWorld, 2);

    Aerogel->SetMaterialPropertiesTable(mptAerogel);

    worldMat = nist->FindOrBuildMaterial("G4_AIR");
    worldMat->SetMaterialPropertiesTable(mptWorld);

    Na = nist->FindOrBuildElement("Na");
    I= nist->FindOrBuildElement("I");
    NaI = new G4Material("NaI", 3.67 * g / cm3, 2);
    NaI->AddElement(Na, 1);
    NaI->AddElement(I, 1);

    G4double fraction[2] = {1.0, 1.0};

    G4MaterialPropertiesTable *mptNaI = new G4MaterialPropertiesTable();
    mptNaI->AddProperty("RINDEX", energy, rindexNaI, 2);
    mptNaI->AddProperty("SCINTILLATIONCOMPONENT1", energy, fraction, 2);
    mptNaI->AddConstProperty("SCINTILLATIONYIELD", 38./keV);
    mptNaI->AddConstProperty("RESOLUTIONSCALE", 1.0);
    mptNaI->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 250*ns);
    mptNaI->AddConstProperty("SCINTILLATIONYIELD1", 1.);

    NaI->SetMaterialPropertiesTable(mptNaI);

    mirrorSurface = new G4OpticalSurface("mirrorSurface");

    mirrorSurface->SetType(dielectric_metal);
    mirrorSurface->SetFinish(ground);
    mirrorSurface->SetModel(unified);
    
    G4MaterialPropertiesTable *mptMirror = new G4MaterialPropertiesTable();
    mptMirror->AddProperty("REFLECTIVITY", energy, reflectivity, 2);

    mirrorSurface->SetMaterialPropertiesTable(mptMirror);
}

void DetectorConstruction::Cherenkov()
{
    solidRadiator = new G4Box("solidRadiator", 0.4 * m, 0.4 * m, 0.01 * m);

    logicRadiator = new G4LogicalVolume(solidRadiator, Aerogel, "logicRadiator");

    physRadiator = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.25 * m), logicRadiator, "physRadiator", logicWorld, false, 0, true);

    solidDetector = new G4Box("solidDetector", xWorld / nRows, yWorld / nCols, 0.01 * m);

    logicDetector = new G4LogicalVolume(solidDetector, worldMat, "logicDetector");

    for(G4int j = 0; j < nRows; j++)
    {
        for(G4int k = 0; k < nCols; k++)
        {
            physDetector = new G4PVPlacement(0, G4ThreeVector(-0.5 * m + (j + 0.5) * m/nRows, -0.5 * m + (k + 0.5) * m/nCols, 0.49 * m), logicDetector, "physDetector", logicWorld, false, k+j*nCols, true);
        }
    }
}

void DetectorConstruction::Scintillator()
{
    G4NistManager *nist = G4NistManager::Instance();

    // --- 1. Material Definitions ---
    G4Element* H = nist->FindOrBuildElement("H");
    G4Element* C = nist->FindOrBuildElement("C");
    G4Element* N = nist->FindOrBuildElement("N");
    G4Element* O = nist->FindOrBuildElement("O");

    // Metals and Nuclear
    G4Material* leadMat = nist->FindOrBuildMaterial("G4_Pb");
    G4Material* steelMat = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
    G4Material* cargoMatIron = nist->FindOrBuildMaterial("G4_Fe");
    U = nist->FindOrBuildMaterial("G4_U");
    G4Material* Pu = nist->FindOrBuildMaterial("G4_Pu");

    // Organics and Cargo
    G4double woodDensity = 0.70 * g/cm3;
    Wood_Aprox = new G4Material("Wood_Aprox", woodDensity, 3);
    Wood_Aprox->AddElement(H, 10); Wood_Aprox->AddElement(C, 6); Wood_Aprox->AddElement(O, 5);

    G4Material* TNT = new G4Material("TNT", 1.65*g/cm3, 4);
    TNT->AddElement(C, 7); TNT->AddElement(H, 5); TNT->AddElement(N, 3); TNT->AddElement(O, 6);

    G4Material* Cocaine = new G4Material("Cocaine", 1.21*g/cm3, 4);
    Cocaine->AddElement(C, 17); Cocaine->AddElement(H, 21); Cocaine->AddElement(N, 1); Cocaine->AddElement(O, 4);

    G4Material* BananaMat = new G4Material("Banana", 1.03*g/cm3, 3);
    BananaMat->AddElement(H, 11); BananaMat->AddElement(C, 6); BananaMat->AddElement(O, 5);

    G4Material* cargoMatWater = nist->FindOrBuildMaterial("G4_WATER");
    G4Material *detMat = nist->FindOrBuildMaterial("G4_AIR");

    // --- 2. Detector setup ---
    solidDetector = new G4Box("solidDetector", 10*cm * f, 22.5*cm * f, 0.5*cm * f);
    logicDetector  = new G4LogicalVolume(solidDetector, detMat, "logicDetector");
    logicDetector2 = new G4LogicalVolume(solidDetector, detMat, "logicDetector2");
    logicDetector3 = new G4LogicalVolume(solidDetector, detMat, "logicDetector3");
    logicDetector4 = new G4LogicalVolume(solidDetector, detMat, "logicDetector4");
    logicDetector5 = new G4LogicalVolume(solidDetector, detMat, "logicDetector5");
    logicDetector6 = new G4LogicalVolume(solidDetector, detMat, "logicDetector6");
    logicDetector7 = new G4LogicalVolume(solidDetector, detMat, "logicDetector7");
    logicDetector8 = new G4LogicalVolume(solidDetector, detMat, "logicDetector8");

    // --- 3. Container and Inner Air Space ---
    G4double container_dx = 5. * cm * f; // 40 cm
    G4double container_dy = 20. * cm * f; // 160 cm
    G4double container_dz = 10. * cm * f; // 80 cm
    G4double wallThickness = 0.5 * cm * f; // 2 cm

    G4Box *solidOuterContainer = new G4Box("solidOuterContainer", container_dx, container_dy, container_dz);
    logicObj = new G4LogicalVolume(solidOuterContainer, steelMat, "logicObj");

    G4double air_dx = container_dx - wallThickness;
    G4double air_dy = container_dy - wallThickness;
    G4double air_dz = container_dz - wallThickness;

    G4Box *solidInnerAir = new G4Box("solidInnerAir", air_dx, air_dy, air_dz);
    G4LogicalVolume *logicInnerAir = new G4LogicalVolume(solidInnerAir, worldMat, "logicInnerAir");
    new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicInnerAir, "physInnerAir", logicObj, false, 0, true);

    // --- 4. Central Uranium Object (u=5) ---
    G4double uranium_dx = 2.0*cm * u;
    G4double uranium_dy = 5.0*cm * u;
    G4double uranium_dz = 2.5*cm * u;

    G4Box* solidUranium = new G4Box("solidUranium", uranium_dx, uranium_dy, uranium_dz); 
    G4LogicalVolume *logicUranium = new G4LogicalVolume(solidUranium, U, "logicUranium");
    G4VisAttributes *uraniumVisAtt = new G4VisAttributes(G4Color(0.0, 1.0, 0.0, 1.0)); // Semi-transparent green
    uraniumVisAtt->SetForceSolid(true);
    logicUranium->SetVisAttributes(uraniumVisAtt);
    new G4PVPlacement(0, G4ThreeVector(0.*cm*f, 0.*cm*f, 0.*cm*f), logicUranium, "physUranium", logicInnerAir, false, 0, true);

    // --- 5. Cargo Placements (Positioned to avoid central Uranium) ---

    // Original Iron Box - Top Front
    G4Box *solidCargo1 = new G4Box("solidCargo1", 1*cm * f, 4*cm * f, 2*cm * f);
    G4LogicalVolume *logicCargo1 = new G4LogicalVolume(solidCargo1, cargoMatIron, "logicCargo1");
    G4VisAttributes *ironVisAtt = new G4VisAttributes(G4Color(0.7, 0.7, 0.7, 1.0));
    ironVisAtt->SetForceSolid(true);
    logicCargo1->SetVisAttributes(ironVisAtt);
    new G4PVPlacement(0, G4ThreeVector(-3.5*cm*f, 14.0*cm*f, -6.0*cm*f), logicCargo1, "physCargo1", logicInnerAir, false, 0, true);

    // Lead Shielding - Bottom Front
    G4Box *solidLead = new G4Box("solidLead", 1.0*cm*f, 2*cm*f, 2.0*cm*f);
    G4LogicalVolume *logicLead = new G4LogicalVolume(solidLead, leadMat, "logicLead");
    G4VisAttributes *leadVisAtt = new G4VisAttributes(G4Color(0.3, 0.3, 0.3));
    leadVisAtt->SetForceSolid(true);
    logicLead->SetVisAttributes(leadVisAtt);
    new G4PVPlacement(0, G4ThreeVector(3.5*cm*f, 15.0*cm*f, 0.0*cm*f), logicLead, "physLead", logicInnerAir, false, 0, true);

    // TNT Explosives - Top Back
    G4Box *solidTNT = new G4Box("solidTNT", 1.0*cm*f, 2.0*cm*f, 1.5*cm*f);
    G4LogicalVolume *logicTNT = new G4LogicalVolume(solidTNT, TNT, "logicTNT"); 
    G4VisAttributes *tntVisAtt = new G4VisAttributes(G4Color(1.0, 0.0, 0.0));
    tntVisAtt->SetForceSolid(true);
    logicTNT->SetVisAttributes(tntVisAtt);
    new G4PVPlacement(0, G4ThreeVector(-3.5*cm*f, -15.0*cm*f, 6.0*cm*f), logicTNT, "physTNT", logicInnerAir, false, 0, true);

    // Cocaine - Bottom Back
    G4Box *solidCocaine = new G4Box("solidCocaine", 1.0*cm*f, 2.0*cm*f, 1.5*cm*f);
    G4LogicalVolume *logicCocaine = new G4LogicalVolume(solidCocaine, Cocaine, "logicCocaine");
    G4VisAttributes *cocaineVisAtt = new G4VisAttributes(G4Color(1.0, 1.0, 1.0));
    cocaineVisAtt->SetForceSolid(true);
    logicCocaine->SetVisAttributes(cocaineVisAtt);
    new G4PVPlacement(0, G4ThreeVector(3.5*cm*f, -15.0*cm*f, 0.0*cm*f), logicCocaine, "physCocaine", logicInnerAir, false, 0, true);

    // Bananas - Center Top
    G4Box *solidBanana = new G4Box("solidBanana", 1.5*cm*f, 2.0*cm*f, 1.5*cm*f);
    G4LogicalVolume *logicBanana = new G4LogicalVolume(solidBanana, BananaMat, "logicBanana");
    G4VisAttributes *bananaVisAtt = new G4VisAttributes(G4Color(1.0, 1.0, 0.0));
    bananaVisAtt->SetForceSolid(true);
    logicBanana->SetVisAttributes(bananaVisAtt);
    new G4PVPlacement(0, G4ThreeVector(0, 15.0*cm*f, 6.0*cm*f), logicBanana, "physBanana", logicInnerAir, false, 0, true);

    // Plutonium - Center Bottom
    G4Box *solidPu = new G4Box("solidPu", 0.5*cm*f*p, 0.5*cm*f*p, 0.5*cm*f*p);
    G4LogicalVolume *logicPu = new G4LogicalVolume(solidPu, Pu, "logicPu");
    G4VisAttributes *puVisAtt = new G4VisAttributes(G4Color(0.5, 0.0, 0.5));
    puVisAtt->SetForceSolid(true);
    logicPu->SetVisAttributes(puVisAtt);
    //new G4PVPlacement(0, G4ThreeVector(-3.5*cm*f, 14.0*cm*f, -6.0*cm*f), logicPu, "physPu", logicInnerAir, false, 0, true);
    new G4PVPlacement(0, G4ThreeVector(0, -15.0*cm*f, -6.0*cm*f), logicPu, "physPu", logicInnerAir, false, 0, true);

    // --- 6. Final Placement and Rotation ---
    rot = new G4RotationMatrix();
    rot->rotateY(0. * deg);
    curAngle = 0.0;
    physObj = new G4PVPlacement(rot, G4ThreeVector(0., 0., 0.), logicObj, "physObj", logicWorld, false, 0, true);

    // Detector Visuals and Placement
    G4VisAttributes *detVisAtt = new G4VisAttributes(G4Color(0.984, 0.256, 0.256, 0.0));
    detVisAtt->SetForceSolid(true);
    logicDetector->SetVisAttributes(detVisAtt); logicDetector2->SetVisAttributes(detVisAtt);
    logicDetector3->SetVisAttributes(detVisAtt); logicDetector4->SetVisAttributes(detVisAtt);
    logicDetector5->SetVisAttributes(detVisAtt); logicDetector6->SetVisAttributes(detVisAtt);
    logicDetector7->SetVisAttributes(detVisAtt); logicDetector8->SetVisAttributes(detVisAtt);

    physDetector = new G4PVPlacement(0, G4ThreeVector(0., 0., -0.25 * m * f), logicDetector, "physDetector", logicWorld, false, 0, true);
    physDetector2 = new G4PVPlacement(0, G4ThreeVector(0., 0., -0.23 * m * f), logicDetector2, "physDetector2", logicWorld, false, 0, true);
    physDetector3 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.23 * m * f), logicDetector3, "physDetector3", logicWorld, false, 0, true);
    physDetector4 = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.25 * m * f), logicDetector4, "physDetector4", logicWorld, false, 0, true);

    G4RotationMatrix *rotDet = new G4RotationMatrix();
    rotDet->rotateX(90 * deg);
    physDetector5 = new G4PVPlacement(rotDet, G4ThreeVector(0., -0.25 * m * f, 0.), logicDetector5, "physDetector5", logicWorld, false, 0, true);
    physDetector6 = new G4PVPlacement(rotDet, G4ThreeVector(0., -0.23 * m * f, 0.), logicDetector6, "physDetector6", logicWorld, false, 0, true);
    physDetector7 = new G4PVPlacement(rotDet, G4ThreeVector(0., 0.23 * m * f, 0.), logicDetector7, "physDetector7", logicWorld, false, 0, true);
    physDetector8 = new G4PVPlacement(rotDet, G4ThreeVector(0., 0.25 * m * f, 0.), logicDetector8, "physDetector8", logicWorld, false, 0, true);
}

G4VPhysicalVolume *DetectorConstruction::Construct()
{
    solidWorld = new G4Box("solidWorld", xWorld, yWorld, zWorld);

    logicWorld = new G4LogicalVolume(solidWorld, worldMat, "logicWorld");

    physWorld = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true);

    if(isCherenkov)
        Cherenkov();
    if(isScintillator)
        Scintillator();

    return physWorld;
}

void DetectorConstruction::ConstructSDandField() 
{
    SensitiveDetector *sensDet = new SensitiveDetector("SensitiveDetector");

    G4SDManager::GetSDMpointer()->AddNewDetector(sensDet);

    G4cout << "Registered SD name = " << sensDet->GetName() << G4endl;

    //if(logicDetector != NULL)
    logicDetector->SetSensitiveDetector(sensDet);
    logicDetector2->SetSensitiveDetector(sensDet);
    logicDetector3->SetSensitiveDetector(sensDet);
    logicDetector4->SetSensitiveDetector(sensDet);
    logicDetector5->SetSensitiveDetector(sensDet);
    logicDetector6->SetSensitiveDetector(sensDet);
    logicDetector7->SetSensitiveDetector(sensDet);
    logicDetector8->SetSensitiveDetector(sensDet);
}

void DetectorConstruction::RotateObjectY(G4double angleDeg)
{
    rot->rotateY(-curAngle * deg);

    rot->rotateY(angleDeg * deg);

    curAngle = angleDeg;

    physObj->SetRotation(rot);
}