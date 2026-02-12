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
#include "project_config.h"

#include <string>
#include <sstream>
#include <iostream>

int LesionCohort::qtd = 0;

void LesionCohort::rate() {
    Disease *disease = cloudo->getDisease();

    if (REMOVAL_METHOD == 0) {
        double age_factor = 1.0f / (1.0f + std::exp(-LAG_SLOPE * (getAge() - T_LAG)));

        // Growth should be limited to the remaining healthy value of 
        // the organ divided up proportionally to total lesion biomass.
        double growthLimit = getOrganHealthyValue() * (getTotalValue() / getOrganDiseaseValue());
        double dailyTissueConsumed = growthLimit * R_MAX * age_factor;

        // Partitioning visible and invisible growth
        // new fungal biomass is sent to invisible value
        // consumed tissue is considered visible value (necrotic)
        // thus, visible and invisible values are combined into a WSDD component
        dailyInvisibleValue = Y * dailyTissueConsumed;
        dailyVisibleValue = (1 - Y) * dailyTissueConsumed;

        // Run Diagnostic
        // std::cout << "[DIAG] " << std::endl <<
        // "  - Growth limit: " << getOrganHealthyValue() << " * " << getTotalValue() << " / " << getOrganDiseaseValue() << std::endl <<
        // "  - Tissue Consumed: " << growthLimit << " * " << R_MAX << " * " << age_factor << " = " << dailyTissueConsumed << std::endl <<
        // "  - Invisible Value: " << dailyInvisibleValue << std::endl <<
        // "  - Visible Value: " << dailyVisibleValue << std::endl <<
        // "------------------------------------------" << std::endl;
        // No rate calculation for new spores in this method
        
    } else if (REMOVAL_METHOD == 1) {
        // NOTE: The healthy area proportion of before was very hamfistedly replaced 
        // with the getOrganHealthyValue() calls and should be revisited to ensure correctness.

        physiologicalDay = util.temperatureFavorability(Basic::getWeather()->getTMean(),
                                                        disease->getTemperatureFavorabilitySet());
        // Thinking on: cumsum(runif(25, min = 0.01, max = 0.1))
        dailyInvisibleValue  = util.growthFunction(getPhysiologicalDaysAcumm(),
                                                    disease->getInvisibleGrowthFunction()) *
                                disease->getHostFactor() *
                                getOrganHealthyValue(); 
                                //* totalArea 

        dailyInvisibleValue *= lesionsInThisCohort;
        
        newSpores = 0;
        if (getOrganHealthyValue() > 0.01 && isInfectionPeriod() &&
                Basic::getWeather()->getWetDur() >= disease->getWetnessThreshold()) {
            newSpores = (lesionsInThisCohort * 
                        disease->getDailySporeProductionPerLesion() * 
                        util.trapezoidalFunction(getAge(), disease->getCohortAgeSet()) *
                        disease->getSporulationCrowdingFactor(getOrganHealthyValue()) *
                        util.temperatureFavorability(Basic::getWeather()->getTMean(),
                                                    disease->getTemperatureFavorabilitySet()));
        }
    }
}

void LesionCohort::integration() {
    Disease *disease = cloudo->getDisease();

    if (getOrganHealthyValue() > 0) {      
        if(dailyInvisibleValue>0) {
            // std::cout << "invis before: " << invisibleValue;
            invisibleValue += dailyInvisibleValue; //(dailyInvisibleAreaGrow-dailyVisibleAreaGrow) * lesionsInThisCohort;
            // std::cout << " invis after: " << invisibleValue << std::endl;
        }
        if(dailyVisibleValue>0) {
            visibleValue += dailyVisibleValue; //dailyVisibleAreaGrow * lesionsInThisCohort;
        }
        

        if(newSpores > 0) {
            cloudo->addSporesCreated(newSpores);
        }        

        physiologicalDaysAcumm += physiologicalDay;

        std::ostringstream convert;
        convert << Basic::getWeather()->getYearDoy() << "," << getTotalValue() << "," << lesionsInThisCohort << "," << getPhysiologicalDaysAcumm() << ","
                << getOrganDiseasedValueProportion() << "," << getLatentValue() << "," << getInfectionValue() << "," << getNecroticValue() << ","
                << newSpores << "," << util.temperatureFavorability(Basic::getWeather()->getTMean(),
                disease->getTemperatureFavorabilitySet()) << "," << dailyVisibleValue << "," << dailyInvisibleValue;
        Basic::output.push_back(convert.str());

        newSpores=0;
    }
}

int LesionCohort::getVisibleLesions() {
    if (isInfectionPeriod() || isNecroticPeriod()){
        return this->lesionsInThisCohort;
    } else {
        return 0;
    }
}

void LesionCohort::output() {
    std::ostringstream convert;
    convert << "Cpp_LesionCohort_" << getID() << ".txt";
    Basic::getOutput(convert.str());
}

bool LesionCohort::isLatentPeriod() const {
    Disease *disease = cloudo->getDisease();
    if (getPhysiologicalDaysAcumm() <= disease->getLatentPeriod()) {
        return true;
    }
    return false;
}

bool LesionCohort::isInfectionPeriod() const {
    Disease *disease = cloudo->getDisease();
    if (getPhysiologicalDaysAcumm() > disease->getLatentPeriod() && getPhysiologicalDaysAcumm() <= (disease->getLatentPeriod() + disease->getInfectionPeriod())) {
        return true;
    }
    return false;
}

bool LesionCohort::isNecroticPeriod() const {
    Disease *disease = cloudo->getDisease();
    if (getPhysiologicalDaysAcumm() > (disease->getLatentPeriod() + disease->getInfectionPeriod())) {
        return true;
    }
    return false;
}
