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

#include "generator.hh"
#include "G4IonTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ParticleTable.hh"
#include "G4RandomTools.hh"

PrimaryGenerator::PrimaryGenerator()
{
    fParticleGun = new G4ParticleGun(1);

    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition *particle = particleTable->FindParticle("mu-");

    /*G4double randX = G4RandFlat::shoot(-60 * cm, 60 * cm);
    G4double randY = G4RandFlat::shoot(-60 * cm, 60 * cm);

    G4ThreeVector pos(randX, randY, -200 * cm);*/
    
    fParticleGun->SetParticlePosition(G4ThreeVector(0., 200 * cm, 0.));
    fParticleGun->SetParticleMomentum(4. * GeV);
    fParticleGun->SetParticleDefinition(particle);
}

PrimaryGenerator::~PrimaryGenerator()
{
    delete fParticleGun;
}

void PrimaryGenerator::GeneratePrimaries(G4Event* anEvent)
{
    G4ParticleDefinition *particle = fParticleGun->GetParticleDefinition();

    G4double randX = G4RandFlat::shoot(-40 * cm, 40 * cm);
    G4double randZ = G4RandFlat::shoot(-100 * cm, 100 * cm);

    G4ThreeVector pos(randX, 200 * cm, randZ);
    // Muons are mostly parallel to the Y-axis (theta is the polar angle relative to the Y-axis)
    G4double thetaMin = 0.0*deg;
    G4double thetaMax = 60.0*deg; // Max deviation from the vertical Y-axis (10 degrees)

    // Use G4UniformRand() to sample cos(theta) uniformly between cos(thetaMax) and cos(thetaMin)
    // theta is the angle from the positive Y-axis.
    G4double cosTheta = G4UniformRand()*(std::cos(thetaMin)-std::cos(thetaMax)) + std::cos(thetaMax);
    G4double theta = std::acos(cosTheta);
    
    // Azimuthal angle (phi) is uniformly random from 0 to 360 degrees
    G4double phi = G4UniformRand() * 2.*CLHEP::pi;

    // The direction vector components for a polar angle 'theta' relative to the Y-axis:
    G4ThreeVector dir(std::sin(theta)*std::cos(phi),
                      -std::cos(theta), // Negated Y-component to direct particles downwards
                      std::sin(theta)*std::sin(phi));
    
    fParticleGun->SetParticlePosition(pos);
    fParticleGun->SetParticleMomentumDirection(dir);
    fParticleGun->GeneratePrimaryVertex(anEvent);
}
