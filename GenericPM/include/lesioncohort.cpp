#include "lesioncohort.h"
#include "cropinterface.h"
#include<string>
#include<sstream>
#include<iostream>

int LesionCohort::qtd = 0;

void LesionCohort::integration() {

    Disease *disease = cloudo->getDisease();

    if (getOrganHealthAreaProportion() > 0.01) {      
        if(dailyVisibleAreaGrow>0) {
            visibleArea = dailyVisibleAreaGrow * lesionsInThisCohort;
        }
        if(dailyInvisibleAreaGrow>0) {
            invisibleArea = (dailyInvisibleAreaGrow-dailyVisibleAreaGrow) * lesionsInThisCohort;
        }
        totalArea = visibleArea + invisibleArea;

        if (isLatentPeriod()) {
            latentArea = totalArea;
            infectionArea = necroticArea = 0;
        } else if (isInfectionPeriod()) {
            infectionArea = totalArea;
            latentArea = necroticArea = 0;
        } else {
            necroticArea = totalArea;
            infectionArea = latentArea = 0;
        }

        cloudo->addSporesCreated(newSpores);

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
    //double dailyVisibleGrowRate = 0, dailyInvisibleGrowRate = 0;
    Disease *disease = cloudo->getDisease();
    physiologicalDay = util.temperatureFavorability(
                            Basic::getWeather()->getTMean(),
                            disease->getTemperatureFavorabilitySet());
    // Thinking on: cumsum(runif(25, min = 0.01, max = 0.1))
    dailyInvisibleAreaGrow  = util.growthFunction(disease->getInvisibleGrowthFunction(), 
                                                  getPhysiologicalDaysAcumm()) 
/*                              * disease->getHostFactor() 
                              * totalArea 
                              * getOrganHealthAreaProportion()*/;
    dailyVisibleAreaGrow    = util.growthFunction(disease->getVisibleGrowthFunction(), 
                                                  getPhysiologicalDaysAcumm()) 
/*                              * disease->getHostFactor()
                              * totalArea
                              * getOrganHealthAreaProportion()*/;
    if (isLatentPeriod()) { 
        dailyVisibleAreaGrow = 0;
    } else if (isNecroticPeriod()) { 
        dailyVisibleAreaGrow = dailyInvisibleAreaGrow = 0;
    }
    
    newSpores = 0;
    if (getOrganHealthAreaProportion() > 0.01 && isInfectionPeriod() &&
            Basic::getWeather()->getWetDur() >= disease->getWetnessThreshold()) // && Basic::getWeather()->getTMean() > 20
    {
        newSpores = (lesionsInThisCohort * disease->getDailySporeProductionPerLesion() * 
                     util.trapezoidalFunction(getAge(), disease->getCohortAgeSet()) *
                     disease->getSporulationCrowdingFactor(getOrganDiseasedAreaProportion()));
    }
    if(organHealthAreaProportion < 0.7) {
        dailyVisibleAreaGrow = dailyInvisibleAreaGrow = 0; // newSpores = 0;
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