#include "manager.h"

Manager::Manager() {
    // Initialize the manager

}

Manager* Manager::instance = nullptr;

Manager* Manager::getInstance() {
    if (instance == nullptr) {
        instance = new Manager();
    }
    return instance;
}

Manager* Manager::newInstance() {
    instance = nullptr;
    return getInstance();
}

void Manager::pushSimulator(Simulator* simulator) {
    simulators.push_back(simulator);
}

Simulator* Manager::getSimulator(int index) {
    if (index < 0 || index >= simulators.size()) {
        return nullptr; // or throw an exception
    }
    return simulators[index];
}

void Manager::setCurrentSimDate(int yearDoy) {
    for (auto& simulator : simulators) {
        simulator->setCurrentYearDoy(yearDoy);
    }
}

void Manager::setPlantingDate(int yearPlt) {
    for (auto& simulator : simulators) {
        simulator->getCropInterface()->setPlantingDate(yearPlt);
    }
}

/*
Execution workflow:
0. Coupling functions are the only ones that are called by fortran.
   These functions have the capability to create and manage the 
   (static) manager instance.
1. Singleton manager instance is created by couplingInit (can we 
   determine if this is the first time?).
2. The manager loads all of the diseases into fio ONCE (during an init 
   ideally).
3. The manager creates a list of simulators and holds onto them in an
   order defined by the user (in the YAML file).
4. The manager can loop through the simulators and propagate the rate,
   integration, and output functions down to lower objects.
NOTES: No objects will be passed back to DSSAT to maintain modularity.
       One simulator contains one initial condition/disease.
*/