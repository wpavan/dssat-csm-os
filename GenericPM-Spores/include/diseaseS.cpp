#include "diseaseS.h"
#include<cmath>
#include<iostream>
#include "../../FlexibleIO/Data/FlexibleIO.hpp"
#include "./utilitiesS.h"

std::vector<DiseaseS*> DiseaseS::listDiseasesS;

double DiseaseS::getSporulationCrowdingFactorS(double proportionDiseaseArea) {
    double a = (1 / (sporulationCrowdingFactorsSet[0] + sporulationCrowdingFactorsSet[1] * pow(proportionDiseaseArea, sporulationCrowdingFactorsSet[2])));
    return (fmin(a,1));
}

int DiseaseS::newLesionsS(double cloudDensity, double healthyAreaProportion) {
    UtilitiesS util;
    double newLesionsS = 0;
    double fitWetnessThreshold = getWetnessThreshold();

    if (healthyAreaProportion > 0 && BasicS::getWeather()->getWetDur() >= fitWetnessThreshold) {
        newLesionsS = (cloudDensity * healthyAreaProportion * getInfectionEfficiency() *
                util.temperatureFavorabilityS(BasicS::getWeather()->getTMean(),
                                             getTemperatureFavorabilitySet()) *
                util.wetnessFavorabilityS(BasicS::getWeather()->getWetDur(),getWetnessFunction())) >0 ? (cloudDensity * healthyAreaProportion * getInfectionEfficiency() *
                util.temperatureFavorabilityS(BasicS::getWeather()->getTMean(),
                                             getTemperatureFavorabilitySet()) *
                util.wetnessFavorabilityS(BasicS::getWeather()->getWetDur(),getWetnessFunction())) : 0;
            /*std::cout << " 1: " << newLesionsS << " 2: " << cloudDensity << " 3: " << healthyAreaProportion << " 4: " << getInfectionEfficiency() <<
                " 5: " << util.temperatureFavorabilityS(BasicS::getWeather()->getTMean(),getTemperatureFavorabilitySet()) << " 6: " <<
                util.wetnessFavorabilityS(BasicS::getWeather()->getWetDur()) << " 7: " << BasicS::getWeather()->getWetDur()<< std::endl; */
    //newLesionsS = newLesionsS * UtilitiesS::runExpressionFunctionS(BasicS::getWeather()->getRh(),getRhFactor());
    //newLesionsS= newLesionsS *  UtilitiesS::runExpressionFunctionS(BasicS::getWeather()->getRh(),getRhFactor());
    //std::cout<<newLesionsS<< " exp : "<<newLesionsS *  UtilitiesS::runExpressionFunctionS(BasicS::getWeather()->getRh(),getRhFactor()) <<std::endl; 
   // std::cout<<"rhfacetor "<<getRhFactor()<<" RH : "<<BasicS::getWeather()->getRh()<<std::endl;
    }
    return newLesionsS;
}
