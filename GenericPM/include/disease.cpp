#include "disease.h"
#include<cmath>
#include<iostream>
#include "../../FlexibleIO/Data/FlexibleIO.hpp"
#include "./utilities.h"

std::vector<Disease*> Disease::listDiseases;



double Disease::getSporulationCrowdingFactor(double proportionDiseaseArea) {
    double a = (1 / (sporulationCrowdingFactorsSet[0] + sporulationCrowdingFactorsSet[1] * pow(proportionDiseaseArea, sporulationCrowdingFactorsSet[2])));
    return (fmin(a,1));
}

double Disease::newLesions(double cloudDensity, double healthyAreaProportion) {
    Utilities util;
    double newLesions = 0, rI, tempFavorability = 0, infectionFHB = 0;
    double fitWetnessThreshold = getWetnessThreshold();
    //double ZSTAGE = FlexibleIO::getInstance()->getReal("PEST", "ZSTAGE");
    int day;
    int FSEED = FlexibleIO::getInstance()->getReal("PEST", "FSEED");
    int YRDOY = FlexibleIO::getInstance()->getReal("PEST", "YRDOY");
    
    //std::cout<<"FSEED "<<FSEED<< " YRDOY "<<YRDOY << " " <<YRDOY - FSEED <<std::endl;
    day = YRDOY - FSEED;
    if (healthyAreaProportion > 0 && Basic::getWeather()->getWetDur() >= fitWetnessThreshold) {
        
        // Windows susceptibility 
        rI = 0.022900 * pow(day, 3.612468) * exp(-0.464022 * day);

        // Review (Mauricio)
        tempFavorability = util.temperatureFavorability(
                            Basic::getWeather()->getTMean(),
                            getTemperatureFavorabilitySet());

        newLesions = cloudDensity * rI * (Basic::getWeather()->getRain() > 2 ? 1 : 0) * 
                     getInfectionEfficiency() * getHostFactor() * tempFavorability;
        // ******* Host Factor???
        // ******* Infection efficiency???
        
        /*std::cout <<YRDOY<< " " << id << " cloudDensity "<< cloudDensity << 
                    " rI " << rI << 
                    " TF " << tempFavorability <<
                    " IE " << getInfectionEfficiency() <<
                    " HF " << getHostFactor() <<
                    " Rain " << Basic::getWeather()->getRain() << 
                    " newLesions "<<newLesions<<std::endl; */
    /*std::cout << " 1: " << newLesions << " 2: " << cloudDensity << " 3: " << healthyAreaProportion << " 4: " << getInfectionEfficiency() <<
        " 5: " << util.temperatureFavorability(Basic::getWeather()->getTMean(),getTemperatureFavorabilitySet()) << " 6: " <<
        util.wetnessFavorability(Basic::getWeather()->getWetDur()) << " 7: " << Basic::getWeather()->getWetDur()<< std::endl; */
    //newLesions = newLesions * Utilities::runExpressionFunction(Basic::getWeather()->getRh(),getRhFactor());
    //newLesions= newLesions *  Utilities::runExpressionFunction(Basic::getWeather()->getRh(),getRhFactor());
    //std::cout<<"newLesions: "<<newLesions<<" getRH: "<<Basic::getWeather()->getRh() <<" getRhFactor()): "<< getRhFactor()<<" exp: "<<newLesions *  Utilities::runExpressionFunction(Basic::getWeather()->getRh(),getRhFactor()) <<std::endl; 
   // std::cout<<"rhfacetor "<<getRhFactor()<<" RH : "<<Basic::getWeather()->getRh()<<std::endl;
    }
    return newLesions;
}
