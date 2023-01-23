#include "cloudo.h"
#include<iostream>
#include<sstream>
#include<string>
//#include "../Coupling-Interface/CPP/coupling_IO_cpp.h"
//#include "../../FlexibleIO/Data/FlexibleIO.hpp"

//extern FlexibleIO *flexibleio;

int CloudO::qtd = 0;
int CloudO::firstOutputCall = 0;

void CloudO::integration() {
    Cloud::integration();

    if (values.size() > (unsigned) disease->getVectorSizeCloudO()) {
        values.erase(values.begin());
    }
    Cloud::removeSporesCloudOByAge();


    if (getValue() > disease->getMaxSporeCloudsDensity()) {
        Cloud::removeSporesCloud(getValue() - disease->getMaxSporeCloudsDensity()); 
    }
    if (Basic::getWeather()->getRain() >= 20) {
        Cloud::removeSporesCloudByRain(0.5);
    }

    std::ostringstream convert;
    convert << Basic::getWeather()->getYearDoy() << "," << getValue();
    for (unsigned int i = 0; i < values.size(); i++) {
        convert << "," << values[i];
    }
    Basic::output.push_back(convert.str());
}

void CloudO::output() {
    Cloud::output();

    std::ostringstream convert;
    convert << "Cpp_CloudO_" << getID() << ".txt";
    Basic::getOutput(convert.str(),this->firstOutputCall);
    this->firstOutputCall++;

    // Speedup the model removing outputs
    //std::cout << "\nCloudO" << ID << ":";
    //for(unsigned int i=0; i<Basic::output.size(); i++)
    //    std::cout << Basic::output[i] << std::endl;
}

void CloudO::addSporesCreated(double sporesCreated) {
    Cloud::sporesCreated += (sporesCreated * (1 - disease->getProportionFromOrganToPlantCloud()));
    cloudP->addSporesCreated(sporesCreated * disease->getProportionFromOrganToPlantCloud());
}
