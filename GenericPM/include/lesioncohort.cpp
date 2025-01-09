/**
 * @file lesioncohort.cpp
 * @brief LesionCohort class source file
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "lesioncohort.h"
#include "cropinterface.h"

#include <string>
#include <sstream>
#include <iostream>

int LesionCohort::qtd = 0;

void LesionCohort::integration() {
    Disease *disease = cloudo->getDisease();

    if (getOrganHealthAreaProportion() > 0.01) {      
        if(dailyInvisibleAreaGrow>0) {
            invisibleArea += dailyInvisibleAreaGrow; //(dailyInvisibleAreaGrow-dailyVisibleAreaGrow) * lesionsInThisCohort;
        }
        
        totalArea = invisibleArea;

        // Se tonar infec 10% da area inv passa a ser visivel e continua crescendo ambas. Visivel nunca será maior que invisivel
        if (isLatentPeriod()) {
            latentArea = invisibleArea;
            visibleArea = infectionArea = necroticArea = 0;
        } else if (isInfectionPeriod()) {
            visibleArea = invisibleArea * 1/2.5;  // virtualRatio
            infectionArea = visibleArea; // totalArea;
            latentArea = necroticArea = 0;
        } else {
            necroticArea = visibleArea;
            infectionArea = latentArea = 0;
        }

        if(newSpores > 0) {
            cloudo->addSporesCreated(newSpores);
        }        

        physiologicalDaysAcumm += physiologicalDay;

        std::ostringstream convert;
        convert << Basic::getWeather()->getYearDoy() << "," << totalArea << "," << lesionsInThisCohort << "," << getPhysiologicalDaysAcumm() << ","
                << getOrganDiseasedAreaProportion() << "," << latentArea << "," << infectionArea << "," << necroticArea << ","
                << newSpores << "," << util.temperatureFavorability(Basic::getWeather()->getTMean(),
                disease->getTemperatureFavorabilitySet()) << "," << dailyVisibleAreaGrow << "," << dailyInvisibleAreaGrow;
        Basic::output.push_back(convert.str());

        newSpores=0;
    }

}

int LesionCohort::getVisibleLesions() {
    if (isInfectionPeriod() || isNecroticPeriod())
        return this->lesionsInThisCohort;
    else
        return 0;
}

void LesionCohort::output() {
    std::ostringstream convert;
    convert << "Cpp_LesionCohort_" << getID() << ".txt";
    Basic::getOutput(convert.str());
}

void LesionCohort::rate() {
    Disease *disease = cloudo->getDisease();

    physiologicalDay = util.temperatureFavorability(
                            Basic::getWeather()->getTMean(),
                            disease->getTemperatureFavorabilitySet());
    // Thinking on: cumsum(runif(25, min = 0.01, max = 0.1))
    dailyInvisibleAreaGrow  = util.growthFunction(getPhysiologicalDaysAcumm(),
                                                  disease->getInvisibleGrowthFunction()) 
                              * disease->getHostFactor() 
                              //* totalArea 
                              * getOrganHealthAreaProportion();
    dailyInvisibleAreaGrow *= lesionsInThisCohort;

    /*std::cout << "growthFunction: " << util.growthFunction(disease->getInvisibleGrowthFunction(), getPhysiologicalDaysAcumm()) << 
              " lesionsInThisCohort: " << lesionsInThisCohort <<
              " dailyInvisibleAreaGrow: " << dailyInvisibleAreaGrow <<
              " HostFactor: " << disease->getHostFactor() << 
              " totalArea: " << totalArea << " HealthAreaProportion: " << 
              getOrganHealthAreaProportion() << std::endl; */
    /*dailyVisibleAreaGrow    = util.growthFunction(disease->getVisibleGrowthFunction(), 
                                                  getPhysiologicalDaysAcumm()) 
                              * disease->getHostFactor()
                              // * totalArea
                              * getOrganHealthAreaProportion();
    if (isLatentPeriod()) { 
        dailyVisibleAreaGrow = 0;
    } else */ 
    if (isNecroticPeriod()) { 
        dailyVisibleAreaGrow = dailyInvisibleAreaGrow = 0;
    }
    
    newSpores = 0;
    if (getOrganHealthAreaProportion() > 0.01 && isInfectionPeriod() &&
            Basic::getWeather()->getWetDur() >= disease->getWetnessThreshold()) // && Basic::getWeather()->getTMean() > 20
    {
        newSpores = (lesionsInThisCohort * disease->getDailySporeProductionPerLesion() * 
                     util.trapezoidalFunction(getAge(), disease->getCohortAgeSet()) *
                     disease->getSporulationCrowdingFactor(getOrganDiseasedAreaProportion()) *
                     util.temperatureFavorability(
                            Basic::getWeather()->getTMean(),
                            disease->getTemperatureFavorabilitySet()));
    }
}

bool LesionCohort::isLatentPeriod() {
    Disease *disease = cloudo->getDisease();
    if (getPhysiologicalDaysAcumm() <= disease->getLatentPeriod()) {
        return true;
    }
    return false;
}

bool LesionCohort::isInfectionPeriod() {
    Disease *disease = cloudo->getDisease();
    if (getPhysiologicalDaysAcumm() > disease->getLatentPeriod() && getPhysiologicalDaysAcumm() <= (disease->getLatentPeriod() + disease->getInfectionPeriod())) {
        return true;
    }
    return false;
}

bool LesionCohort::isNecroticPeriod() {
    Disease *disease = cloudo->getDisease();
    if (getPhysiologicalDaysAcumm() > (disease->getLatentPeriod() + disease->getInfectionPeriod())) {
        return true;
    }
    return false;
}
