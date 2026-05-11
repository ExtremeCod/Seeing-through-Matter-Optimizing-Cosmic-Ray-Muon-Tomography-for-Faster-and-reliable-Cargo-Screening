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

#include "event.hh"

EventAction::EventAction(RunAction*) 
{
    fEdep = 0.;
}

EventAction::~EventAction()
{}

void EventAction::BeginOfEventAction(const G4Event*)
{
    fEdep = 0.;
}

void EventAction::EndOfEventAction(const G4Event*)
{
    //G4cout << "Energy Deposition: " << fEdep << G4endl;
}