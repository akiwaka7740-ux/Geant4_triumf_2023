#include "PhysicsList.hh"

PhysicsList::PhysicsList()
{
    //EM physics
    RegisterPhysics(new G4EmStandardPhysics());

   
}

PhysicsList::~PhysicsList()
{

}