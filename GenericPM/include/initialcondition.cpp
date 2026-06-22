/**
 * @file initialcondition.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "initialcondition.h"
#include "debug_control.h"

#include <sstream>
#include <iostream>
#include <fstream>

int InitialCondition::qtd = 0;

/*
 * Disease variables must come from different locations in memory to 
 * accomodate multiple diseases. Temperature and wetness favorability
 * are shared within the cloud, but the favorability threshold and 
 * initial inoculum added are specific to the diseases in the yaml file
 */

void InitialCondition::rate() {
    if (!favorabilityAccumulated && cloudF) {
        try {
            dailyFavorability = cloudF->getDisease()->getII_AGE()->evaluate();
        } catch (const std::runtime_error& e) {
            std::cerr << "Error evaluating II_AGE expression for DiseaseID: " << cloudF->getDisease()->getDiseaseID() << std::endl << "Exception: " << e.what() << std::endl;
            dailyFavorability = 0.0f; // Default to 0 favorability if
        }
        
        // dailyFavorability = tempFactor * wetnessFactor;      cloudF->getDisease()->getWetnessFunction());
    }
}

void InitialCondition::integration() {
    if (cloudF) {
        integration(cloudF->getDisease());
    }
}

void InitialCondition::integration(std::shared_ptr<Disease> disease) {
    if (!favorabilityAccumulated) {
        int yearDoy = Manager::getInstance()->getCurrentSimDate();
        acumulateFavorability += dailyFavorability;
        // NOTE: If we have one simulator for each disease, should the line below
        //       be getting the disease from a disease object or simulator object instead 
        //       of the current implementation?
        if (acumulateFavorability >= disease->getAcumulateFavorability() && yearDoy >= 0) {
#if GENERICPM_DEBUG_ENABLED
            printf("Accumulated Favorability reached: %.2f on YEARDOY: %d\n", acumulateFavorability, yearDoy);
#endif
            DormantInoculum* dormantInoc = DormantInoculum::getInstance();
            float dormantInoculum = dormantInoc->getDiseaseInoculum(disease->getDiseaseID());
            if (dormantInoculum > 0) {
                cloudF->setFirstSporeCloud(dormantInoculum);
                dormantInoc->clear(disease->getDiseaseID());
#if (GENERICPM_DEBUG_ENABLED)
                printf("Using dormant inoculum for disease %s: %.2f\n", disease->getDiseaseID().c_str(), dormantInoculum);
#endif
            } else {
                cloudF->setFirstSporeCloud(disease->getInitialInoculum());
#if (GENERICPM_DEBUG_ENABLED)
                printf("No dormant inoculum for disease %s. Using initial inoculum: %.2f\n", disease->getDiseaseID().c_str(), disease->getInitialInoculum());
#endif
            }
            favorabilityAccumulated = true;
        }

        std::ostringstream convert;
        convert << yearDoy << "," << acumulateFavorability;
        Basic::output.push_back(convert.str());
    }
}

void InitialCondition::output() {
    std::ostringstream convert;
    convert << "Cpp_InitialCondition_" << getID() << ".txt";
    Basic::getOutput(convert.str());

    // Speedup the model removing outputs
    #ifdef OUTPUT
    std::cout << "\nInitialCondition " << getID() << ":";
    for(unsigned int i=0; i<Basic::output.size(); i++) {
       std::cout << Basic::output[i] << std::endl;
    }
    #endif // OUTPUT

    // Run the cloud integration only once per day (handled by cloudF)
    if (cloudF) cloudF->output();
}
