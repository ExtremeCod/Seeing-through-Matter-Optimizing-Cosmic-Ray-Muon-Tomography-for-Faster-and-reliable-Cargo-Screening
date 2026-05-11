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

#include <iostream>

#include "G4RunManager.hh"
#include "G4UIManager.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4MTRunManager.hh"
#include "G4RandomTools.hh"
#include <ctime>

#include "construction.hh"
#include "physics.hh"
#include "action.hh"

void RunTomography(DetectorConstruction* det, G4int nAngles, G4int eventsPerAngle, G4double maxAngle)
{
    // Scan from -maxAngle to +maxAngle
    G4double angleStep = 2.0 * maxAngle / (nAngles - 1);  // step size

    auto runAction = (RunAction*) G4RunManager::GetRunManager()->GetUserRunAction();

    for(G4int i = 0; i < nAngles; i++)
    {
        // Current angle: start from -maxAngle
        G4double angle = -maxAngle + i * angleStep;
        det->RotateObjectY(angle);

        G4cout << "Running angle " << angle << " deg around Y-axis" << G4endl;

        runAction->SetFileIndex(i);

        // Run beam for this angle
        G4RunManager::GetRunManager()->BeamOn(eventsPerAngle);
    }
}

void PassRotate(DetectorConstruction *det, G4double angle)
{
    det->RotateObjectY(angle);
}

int main(int argc, char** argv) 
{
    G4RunManager *runManager = new G4RunManager();

    //G4MTRunManager *runManager = new G4MTRunManager();
    //runManager->SetNumberOfThreads(1);

    G4long seed = time(NULL);
    G4Random::setTheSeed(seed);

    auto det = new DetectorConstruction();

    runManager->SetUserInitialization(det);
    runManager->SetUserInitialization(new PhysicsList());
    runManager->SetUserInitialization(new ActionInitialization());

    runManager->Initialize();

    G4UIExecutive *ui = new G4UIExecutive(argc, argv);

    G4VisManager *visManager = new G4VisExecutive();
    visManager->Initialize();

    G4UImanager *UImanager = G4UImanager::GetUIpointer();

    UImanager->ApplyCommand("/vis/open OGL"); 
    UImanager->ApplyCommand("/vis/viewer/set/viewpointVector 1 1 1");
    UImanager->ApplyCommand("/vis/drawVolume");
    UImanager->ApplyCommand("/vis/viewer/set/autoRefresh true");
    UImanager->ApplyCommand("/vis/scene/add/trajectories smooth");
    UImanager->ApplyCommand("/vis/scene/endOfEventAction accumulate");
    UImanager->ApplyCommand("/vis/scene/add/scale 10 cm");
    UImanager->ApplyCommand("/vis/scene/add/eventID");
    /*UImanager->ApplyCommand("/vis/disable");
    UImanager->ApplyCommand("/tracking/verbose 0");
    UImanager->ApplyCommand("/tracking/storeTrajectory 0");
    UImanager->ApplyCommand("/event/verbose 0");
    UImanager->ApplyCommand("/run/verbose 0");*/

    //PassRotate(det, 60);

    //RunTomography(det, 120, 50000, 0); // Layout: (det, numOfProjections, numOfBeamOn, numOfRange(e.g. 40 = -40° to +40°))

    ui->SessionStart();

    return 0;
}