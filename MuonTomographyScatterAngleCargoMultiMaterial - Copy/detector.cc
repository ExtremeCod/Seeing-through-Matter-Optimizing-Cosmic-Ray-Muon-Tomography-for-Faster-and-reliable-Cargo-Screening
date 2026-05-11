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

#include "detector.hh"

SensitiveDetector::SensitiveDetector(G4String name) : G4VSensitiveDetector(name), entryRecorded(false), exitRecorded(false)
{}

SensitiveDetector::~SensitiveDetector()
{}

void SensitiveDetector::Initialize(G4HCofThisEvent*)
{
    entryRecorded = false;
    exitRecorded  = false;
}

G4ThreeVector FitLineDirection(const G4ThreeVector& p1, const G4ThreeVector& p2)
{
    return (p2 - p1).unit();
}

G4bool SensitiveDetector::ProcessHits(G4Step* step,
                                      G4TouchableHistory*)
{
    G4Track* track = step->GetTrack();

    // ---- Only muons ----
    auto pname = track->GetDefinition()->GetParticleName();
    if(pname != "mu-" && pname != "mu+")
        return false;

    if(!step->IsFirstStepInVolume())
        return false;

    G4String vname =
        step->GetPreStepPoint()->GetTouchableHandle()
        ->GetVolume()->GetName();

    G4ThreeVector pos = track->GetPosition();
    auto man = G4AnalysisManager::Instance();

    // ================= ENTRY =================
    if(!entryRecorded &&
       (vname == "physDetector"  ||
        vname == "physDetector2" ||
        vname == "physDetector3" ||
        vname == "physDetector4" ||
        vname == "physDetector7" ||
        vname == "physDetector8"))
    {
        entryPos1 = pos;
        entryRecorded = true;
        return true;
    }

    // second entry hit
    if(entryRecorded && !exitRecorded &&
       (vname == "physDetector"  ||
        vname == "physDetector2" ||
        vname == "physDetector3" ||
        vname == "physDetector4" ||
        vname == "physDetector7" ||
        vname == "physDetector8"))
    {
        entryPos2 = pos;
        return true;
    }

    // ================= EXIT =================
    if(entryRecorded && !exitRecorded &&
       (vname == "physDetector5" ||
        vname == "physDetector6" ||
        vname == "physDetector"  ||
        vname == "physDetector2" ||
        vname == "physDetector3" ||
        vname == "physDetector4"))
    {
        exitPos1 = pos;
        exitRecorded = true;
        return true;
    }

    // second exit hit → reconstruct
    if(entryRecorded && exitRecorded &&
       (vname == "physDetector5" ||
        vname == "physDetector6" ||
        vname == "physDetector"  ||
        vname == "physDetector2" ||
        vname == "physDetector3" ||
        vname == "physDetector4"))
    {
        exitPos2 = pos;

        G4ThreeVector dEntry = (entryPos2 - entryPos1).unit();
        G4ThreeVector dExit  = (exitPos2  - exitPos1 ).unit();

        G4ThreeVector dEntryPOCA = FitLineDirection(entryPos1, entryPos2);
        G4ThreeVector dExitPOCA  = FitLineDirection(exitPos1, exitPos2);

        // -------------------- POCA calculation --------------------
        G4ThreeVector w0 = entryPos1 - exitPos1;
        double a = dEntry.dot(dEntry);
        double b = dEntry.dot(dExit);
        double c = dExit.dot(dExit);
        double d = dEntry.dot(w0);
        double e = dExit.dot(w0);

        double denom = a*c - b*b;
        if(std::fabs(denom) < 1e-8) return true; // skip nearly parallel

        double sc = (b*e - c*d)/denom;
        double tc = (a*e - b*d)/denom;

        G4ThreeVector poca1 = entryPos1 + sc*dEntry;
        G4ThreeVector poca2 = exitPos1 + tc*dExit;
        G4ThreeVector POCA  = 0.5*(poca1 + poca2);

        G4double angleRadPOCA = dEntryPOCA.angle(dExitPOCA);
        G4double angleDegPOCA = angleRadPOCA * 180.0 / CLHEP::pi;

        G4double angleDeg = dEntry.angle(dExit) * 180.0 / CLHEP::pi;

        man->FillNtupleDColumn(0, 0, entryPos1.x()/cm);
        man->FillNtupleDColumn(0, 1, entryPos1.y()/cm);
        man->FillNtupleDColumn(0, 2, entryPos1.z()/cm);
        man->FillNtupleDColumn(0, 3, dEntry.x());
        man->FillNtupleDColumn(0, 4, dEntry.y());
        man->FillNtupleDColumn(0, 5, dEntry.z());

        man->FillNtupleDColumn(0, 6, exitPos2.x()/cm);
        man->FillNtupleDColumn(0, 7, exitPos2.y()/cm);
        man->FillNtupleDColumn(0, 8, exitPos2.z()/cm);
        man->FillNtupleDColumn(0, 9, dExit.x());
        man->FillNtupleDColumn(0,10, dExit.y());
        man->FillNtupleDColumn(0,11, dExit.z());
        man->FillNtupleDColumn(0,12, POCA.x()/cm);
        man->FillNtupleDColumn(0,13, POCA.y()/cm);
        man->FillNtupleDColumn(0,14, POCA.z()/cm);
        man->FillNtupleDColumn(0,15, angleDeg);

        man->AddNtupleRow(0);

        // reset for next muon
        entryRecorded = false;
        exitRecorded  = false;

        return true;
    }

    return true;
}
