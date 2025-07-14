/**
 * @file simulator.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "basicinterface.h"
#include "cropinterface.h"
#include "initialcondition.h"
#include "plant.h"

#include <vector>

class Simulator : virtual public BasicInterface {
private:
    Utilities util;

protected:
    int currentYearDoy = 0;
    Injection *rateInj;
    Injection *integrationInj;
    CropInterface *cropinterface;
    InitialCondition initialCondition;
    std::vector<Plant> plants;

public:
    Simulator();
    Simulator(Disease *disease) : initialCondition(disease){
        cropinterface = CropInterface::newInstance();
        cropinterface->start();
        inputPST();

        // Compile and load injections
        //Injection* rateInjection = getRateInjection();
        //rateInjection->compile("RATE.cpp", "RATE.dll");
        //rateInjection->load("RATE.dll");
    };

    //static Simulator* getInstance();
    //static Simulator* newInstance();
    void inputPST_FromYaml();
    void inputPST();
    void integration();
    void output();
    void rate();
    void updateCurrentYearDoy(int yearDoy);
    bool allPlantsSenesced();

    std::vector<Plant>& getPlants() {
        return plants;
    }

    InitialCondition& getInitialCondition() {
        return initialCondition;
    }

    CropInterface* getCropInterface() {
        return cropinterface;
    }

    void setCurrentYearDoy(int currentYearDoy) {
        this->currentYearDoy = currentYearDoy;
    }

    int getCurrentYearDoy() const {
        return currentYearDoy;
    }

    void setRateInjection(Injection *inj) {
        rateInj = inj;
    }

    Injection* getRateInjection() {
        if (rateInj == nullptr) {
            rateInj = new Injection();
        }
        return rateInj;
    }
};

#endif // SIMULATOR_H
