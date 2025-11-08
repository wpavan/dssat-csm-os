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
#include "injection.h"
#include "plant.h"

#include <vector>

struct Output {
    std::string varName;
    float value;
};

class Simulator : virtual public BasicInterface {
private:
    Utilities util;

protected:
    int currentYearDoy = 0;
    InitialCondition initialCondition;
    Disease *disease;
    CropInterface *cropinterface;
    std::vector<Output> loggedOutputs;

public:
    Simulator();
    Simulator(Disease *dis, CropInterface *ci) : initialCondition(dis->getFamily()), disease(dis), cropinterface(ci) {
        //cropinterface->start();

        // Compile and load injections ?
        //Injection* rateInjection = getRateInjection();
        //rateInjection->compile("RATE.cpp", "RATE.dll");
        //rateInjection->load("RATE.dll");
    };

    void inputPST_FromYaml();
    void inputPST();
    void integration();
    void output();
    void rate();
    void updateCurrentYearDoy(int yearDoy);
    bool allPlantsSenesced();

    void clearOutputLog();
    void logOutput(std::string varName, float value);

    Plant* getPlant() {
        return Plant::getInstance();
    }

    InitialCondition* getInitialCondition() {
        InitialCondition* ic = &initialCondition;
        return ic;
    }

    void setCurrentYearDoy(int currentYearDoy) {
        this->currentYearDoy = currentYearDoy;
    }

    int getCurrentYearDoy() const {
        return currentYearDoy;
    }

    Disease* getDisease() {
        return disease;
    }

    void setDisease(Disease *disease) {
        this->disease = disease;
        initialCondition = InitialCondition(disease->getFamily());
    }

    CropInterface* getCropInterface() {
        return cropinterface;
    }

    /*void resetCropInterface() {
        delete cropinterface;
        cropinterface = new CropInterface();
        cropinterface->start();
    }*/

    /*void setRateInjection(Injection *inj) {
        rateInj = inj;
    }

    Injection* getRateInjection() {
        if (rateInj == nullptr) {
            rateInj = new Injection();
        }
        return rateInj;
    }*/
};

#endif // SIMULATOR_H
