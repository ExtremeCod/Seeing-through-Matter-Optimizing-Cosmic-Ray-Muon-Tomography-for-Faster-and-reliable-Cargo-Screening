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

#include "run.hh"
#include "G4AnalysisManager.hh"
#include "G4Threading.hh"
#include "G4Run.hh"

RunAction::RunAction()
{
    // Nothing heavy here. You can set analysis manager options that
    // should be present for all threads:
    auto man = G4AnalysisManager::Instance();
    man->SetNtupleMerging(true);        // enable merging (important in MT)
    man->SetVerboseLevel(1);

    if ( G4Threading::IsMasterThread() )
    {
        man->CreateNtuple("Muons", "Muon Track Data");

        // Incoming track point (Top detector)
        man->CreateNtupleDColumn("in_x"); // x0
        man->CreateNtupleDColumn("in_y"); // y0
        man->CreateNtupleDColumn("in_z"); // z0

        // Incoming direction
        man->CreateNtupleDColumn("in_dx"); // dx0
        man->CreateNtupleDColumn("in_dy"); // dy0
        man->CreateNtupleDColumn("in_dz"); // dz0

        // Outgoing track point (Bottom detector)
        man->CreateNtupleDColumn("out_x"); // x1
        man->CreateNtupleDColumn("out_y"); // y1
        man->CreateNtupleDColumn("out_z"); // z1

        // Outgoing direction
        man->CreateNtupleDColumn("out_dx"); // dx1
        man->CreateNtupleDColumn("out_dy"); // dy1
        man->CreateNtupleDColumn("out_dz"); // dz1

        // POCA point
        man->CreateNtupleDColumn("poca_x"); // POCA x
        man->CreateNtupleDColumn("poca_y"); // POCA y
        man->CreateNtupleDColumn("poca_z"); // POCA z

        // Scattering angle
        man->CreateNtupleDColumn("angle");  // angle in degrees

        man->FinishNtuple();

        man->CreateH2("angle_vs_x", "Angle vs X;X;Angle",
                      100, -5*cm, 5*cm,
                      100, 0, 10*deg);

        man->CreateH2("angle_vs_y", "Angle vs Y;Y;Angle",
                      100, -5*cm, 5*cm,
                      100, 0, 10*deg);

        man->CreateH2("scatterAngle_x_y", "Orig XY vs Angle;X;Y;COL:Angle", 200, -100, 100, 200, -100, 100);

        man->CreateH2("orig_xy", "Original X vs Y;X (mm);Y (mm)",
                            200, -100, 100,   // X bins/min/max
                            200, -100, 100);  // Y bins/min/max
                      
    }
}

RunAction::~RunAction()
{}

void RunAction::BeginOfRunAction(const G4Run* run)
{
    G4AnalysisManager *man = G4AnalysisManager::Instance();

    /*G4int runID = run->GetRunID();

    std::stringstream strRunID;
    strRunID << runID;*/

    // Only master thread should create file + objects
    if ( G4Threading::IsMasterThread() )
    {
        std::ostringstream filename;
        filename << "output" << fFileIndex << ".root";
        man->OpenFile(filename.str());
    }
}

void RunAction::EndOfRunAction(const G4Run* /*run*/)
{
    G4AnalysisManager *man = G4AnalysisManager::Instance();

    // Only master writes and closes
    if ( G4Threading::IsMasterThread() )
    {
        man->Write();
        man->CloseFile();
    }
}
