/**
 * @file disease.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "disease.h"
#include "../../FlexibleIO/Data/FlexibleIO.hpp"
#include "./utilities.h"

#include <cmath>
#include <iostream>

std::vector<Disease*> Disease::listDiseases;

float Disease::getSporulationCrowdingFactor(float proportionDiseaseArea) {
    float a = (1 / (sporulationCrowdingFactorsSet[0] + sporulationCrowdingFactorsSet[1] * pow(proportionDiseaseArea, sporulationCrowdingFactorsSet[2])));
    return (fmin(a,1));
}

int Disease::newLesions(float cloudDensity, float healthyAreaProportion) {
    Utilities util;
    float newLesions = 0;
    float fitWetnessThreshold = getWetnessThreshold();
    if (healthyAreaProportion > 0 && Basic::getWeather()->getWetDur() >= fitWetnessThreshold) {
        newLesions = fmax(0.0, (cloudDensity * healthyAreaProportion * getInfectionEfficiency() *
                util.temperatureFavorability(Basic::getWeather()->getTMean(),
                                             getTemperatureFavorabilitySet()) *
                util.wetnessFavorability(Basic::getWeather()->getWetDur(),
                                         getWetnessFunction())));

            //std::cout << 
            //" 1: " << newLesions << 
            //" 2: " << cloudDensity << 
            //" 3: " << healthyAreaProportion << 
            //" 4: " << getInfectionEfficiency() <<
            //" 5: " << util.temperatureFavorability(Basic::getWeather()->getTMean(),getTemperatureFavorabilitySet()) << 
            //" 6: " <<
            //    util.wetnessFavorability(Basic::getWeather()->getWetDur(),
            //                             getWetnessFunction()) << 
            //" 7: " << Basic::getWeather()->getWetDur()<< std::endl; 
    //newLesions = newLesions * Utilities::runExpressionFunction(Basic::getWeather()->getRh(),getRhFactor());
    //newLesions= newLesions *  Utilities::runExpressionFunction(Basic::getWeather()->getRh(),getRhFactor());
    //std::cout<<"newLesions: "<<newLesions<<" getRH: "<<Basic::getWeather()->getRh() <<" getRhFactor()): "<< getRhFactor()<<" exp: "<<newLesions *  Utilities::runExpressionFunction(Basic::getWeather()->getRh(),getRhFactor()) <<std::endl; 
    //std::cout<<"rhfacetor "<<getRhFactor()<<" RH : "<<Basic::getWeather()->getRh()<<std::endl;
    }
    return newLesions;
}
