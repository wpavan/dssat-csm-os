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
        
        // dailyFavorability = Utilities::temperatureFavorability(Basic::getWeather()->getTMean(),
        //                                                        cloudF->getDisease()->getTemperatureFavorabilitySet()) 
        //                     * Utilities::wetnessFavorability(Basic::getWeather()->getWetDur(), 
        //                                                      cloudF->getDisease()->getWetnessFunction());
    }
}

void InitialCondition::integration() {
    if (cloudF) {
        integration(cloudF->getDisease());
    }
}

void InitialCondition::integration(Disease *disease) {
    if (!favorabilityAccumulated) {
        acumulateFavorability += dailyFavorability;
        // NOTE: If we have one simulator for each disease, should the line below
        //       be getting the disease from a disease object or simulator object instead 
        //       of the current implementation?
        if (acumulateFavorability >= disease->getAcumulateFavorability() && Basic::getWeather()->getDoy() >= 0) {
            printf("Accumulated Favorability reached: %.2f on day %d\n", acumulateFavorability, Basic::getWeather()->getDoy());
            cloudF->setFirstSporeCloud(disease->getInitialInoculum());
            favorabilityAccumulated = true;
        }

        std::ostringstream convert;
        convert << Basic::getWeather()->getYearDoy() << "," << acumulateFavorability;
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
