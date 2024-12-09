#include "cloudf.h"
#include<iostream>
#include<sstream>
#include <cmath>

int CloudF::qtd = 0;
int CloudF::firstOutputCall = 0;

void CloudF::integration() {
    Cloud::integration();
    double porcent=0;
    if (values.size() > (unsigned) disease->getVectorSizeCloudF()) {
        values.erase(values.begin());
    }
    if (getValue() > disease->getMaxSporeCloudsDensity()) {
        Cloud::removeSporesCloud(getValue() - disease->getMaxSporeCloudsDensity());
    }
    if (Basic::getWeather()->getRain() >= disease->getMRRS()) {
        Cloud::removeSporesCloudByRain(1-exp(-0.035*Basic::getWeather()->getRain()));
    }
        

    std::ostringstream convert;
    convert << Basic::getWeather()->getYearDoy() << "," << getValue();
    for (unsigned int i = 0; i < values.size(); i++) {
        convert << "," << values[i];
    }
    Basic::output.push_back(convert.str());
}

void CloudF::output() {
    Cloud::output();

    std::ostringstream convert;
    convert << "Cpp_CloudF_" << getID() << ".txt";
    Basic::getOutput(convert.str(),this->firstOutputCall);
    this->firstOutputCall++;

    // Speedup the model
    //std::cout << "\nCloudF" << ID << ":";
    //for(unsigned int i=0; i<Basic::output.size(); i++)
    //    std::cout << Basic::output[i] << std::endl;
}

