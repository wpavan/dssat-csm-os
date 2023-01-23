#include "initialconditionS.h"

#include<sstream>
#include<iostream>
#include<fstream>

int InitialConditionS::qtdS = 0;

/** Calculate the daily favorability based on temp * wetness favorability*/
void InitialConditionS::rateS() {
    if (!stop) {
        dailyFavorability = UtilitiesS::temperatureFavorabilityS(BasicS::getWeather()->getTMean(),
                                                               cloudf.getDisease()->getTemperatureFavorabilitySet()) 
                            *
                            UtilitiesS::wetnessFavorabilityS(BasicS::getWeather()->getWetDur(), cloudf.getDisease()->getWetnessFunction()); //
    }
    cloudf.rateS();
}

/** Accumulate the daily favorability. If the this value hits the pre-determinated value, stop the process */
void InitialConditionS::integrationS() {
    if (!stop) {
        acumulateFavorability += dailyFavorability;
        if (acumulateFavorability >= cloudf.getDisease()->getAcumulateFavorability()) {
            cloudf.setFirstSporeCloud(cloudf.getDisease()->getInitialInoculum());
            stop = true;
        }

        std::ostringstream convert;
        convert << BasicS::getWeather()->getYearDoy() << "," << acumulateFavorability;
        BasicS::output.push_back(convert.str());

    }
    cloudf.integrationS();

}

void InitialConditionS::outputS() {
    std::ostringstream convert;
    convert << "Cpp_InitialCondition_" << getID() << ".txt";
    BasicS::getOutput(convert.str());

    // Speedup the model removing outputs
    //std::cout << "\nInitialCondition " << getID() << ":";
    //for(unsigned int i=0; i<BasicS::output.size(); i++)
    //{
    //    std::cout << BasicS::output[i] << std::endl;
    //}
    cloudf.outputS();
}
