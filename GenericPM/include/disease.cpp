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
#include "./project_config.h"

#include <cmath>
#include <iostream>

std::vector<Disease*> Disease::listDiseases;

float Disease::getSporulationCrowdingFactor(float proportionDiseaseArea) {
    float a = (1 / (sporulationCrowdingFactorsSet[0] + sporulationCrowdingFactorsSet[1] * pow(proportionDiseaseArea, sporulationCrowdingFactorsSet[2])));
    return (fmin(a,1));
}

float Disease::newLesions(float cloudDensity, float healthyValue) {
    FlexibleIO *fio = FlexibleIO::getInstance();
    Utilities util;
    int newLesionsInt = 0;
    float newLesionsFloat = 0.0f;
    float fitWetnessThreshold = getWetnessThreshold();

    // std::cout << " Healthy Area Proportion: " << healthyAreaProportion << " Dur | Threshold: " << Basic::getWeather()->getRain() << " | " << fitWetnessThreshold << std::endl;
    if (healthyValue > 0) {
        // NOTE: newLesions is the equal to the dI/dt value * 1 day. We
        //       only need to consider removing this equation if it 
        //       doesn't work for FHB.
        //       ******************************************************
        //       dI/dt in Mauricio's code is:
        //       (k_inf * W * is_rainy_day * anther_prop_t) - (k_rec * I)
        //       *w/o recovery*
        //
        //       k_inf          => IE
        //       W              => cloudDensity
        //       is_rainy_day   => wetnessFavorability
        //       anther_prop_t  => ??? (this could maybe be linked to a
        //                         scaled coupling point value)
        //       ???            => temperatureFavorability
        //       ???            => healthyAreaProportion

        // newLesions = fmax(0.0, (cloudDensity * healthyAreaProportion * getInfectionEfficiency() *
        //         util.temperatureFavorability(Basic::getWeather()->getTMean(),
        //                                      getTemperatureFavorabilitySet()) *
        //         util.wetnessFavorability(Basic::getWeather()->getWetDur(),
        //                                  getWetnessFunction()) *
        //         biologicalFactor));

        // New version to represent lag phase (infection is in injection code stored in fio)
        //std::cout << "INFECTIVE_SPORES (INIT): " << fio->getReal("PEST", "INFECTIVE_SPORES") << std::endl;
        
        if (!createdSpores) {
            newLesionsFloat = fmax(0.0f, fio->getReal("PEST", "INFECTIVE_SPORES"));
            fio->setRealMemory("PEST", "INFECTIVE_SPORES", 0.0f);
            createdSpores = true;
        } else {
            newLesionsFloat = 0.0f;
        }
        //std::cout << "---------------- (END) : " << fio->getReal("PEST", "INFECTIVE_SPORES") << std::endl;

        // R Code version
        // newLesionsFloat = fmax(0.0f, K_INF * cloudDensity * biologicalFactor * util.temperatureFavorability(Basic::getWeather()->getTMean(),
        //                                                         getTemperatureFavorabilitySet()));
        
        // std::cout <<
        // " K_INF: " << K_INF << std::endl <<
        // " cloudValue: " << cloudDensity << std::endl <<
        // " bio_factor: " << biologicalFactor << std::endl <<
        // " temp_factor: " << util.temperatureFavorability(Basic::getWeather()->getTMean(),
        //                                                          getTemperatureFavorabilitySet()) << std::endl <<
        // " ... newLesions: " << newLesionsFloat << std::endl;
                                                                 
        
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
    //newLesionsInt = static_cast<int>(newLesionsFloat);
    // return newLesionsInt
    return newLesionsFloat;
}
