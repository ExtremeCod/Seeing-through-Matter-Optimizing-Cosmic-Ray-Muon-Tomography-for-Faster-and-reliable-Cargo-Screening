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

#ifndef DETECTOR_HH
#define DETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"

#include "G4AnalysisManager.hh"
#include <cmath>

class SensitiveDetector : public G4VSensitiveDetector
{
public:
    SensitiveDetector(G4String);
    ~SensitiveDetector();

    G4ThreeVector entryPos1;
    G4ThreeVector entryPos2;
    G4ThreeVector exitPos1;
    G4ThreeVector exitPos2;

    // Ensure one POCA calculation per muon
    G4bool entryRecorded;   // entry recorded
    G4bool exitRecorded;    // exit recorded

private:
    virtual G4bool ProcessHits(G4Step *, G4TouchableHistory *);

    virtual void Initialize(G4HCofThisEvent *hitCollection);
};
#endif