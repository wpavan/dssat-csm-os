#ifndef SIMULATORS_H
#define SIMULATORS_H

#include "basicinterfaceS.h"
#include "cropinterfaceS.h"
#include "initialconditionS.h"
#include "plantS.h"


#include<vector>

class SimulatorS : virtual public BasicInterfaceS {
private:
    UtilitiesS util;

protected:
    SimulatorS();
    static SimulatorS *instance;
    int currentYearDoy = 0;

    CropInterfaceS *cropinterface;
    std::vector<InitialConditionS> initialConditions;
    std::vector<PlantS> plants;

public:
    static SimulatorS* getInstanceS();
    static SimulatorS* newInstanceS();
    void inputPSTS();
    void inicializationS();
    void integrationS();
    void outputS();
    void rateS();
    void updateCurrentYearDoyS(int yearDoy);
    bool allPlantsSenescedS();

    std::vector<PlantS>& getPlants() {
        return plants;
    }

    std::vector<InitialConditionS>& getInitialConditions() {
        return initialConditions;
    }

    CropInterfaceS* getCropInterface() {
        return cropinterface;
    }

    void setCurrentYearDoy(int currentYearDoy) {
        this->currentYearDoy = currentYearDoy;
    }

    int getCurrentYearDoy() const {
        return currentYearDoy;
    }
};

#endif // SIMULATOR_H
