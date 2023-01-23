#include "cloudfS.h"
#include<iostream>
#include<sstream>
//#include "../../FlexibleIO/Data/FlexibleIO.hpp"

//FlexibleIO *flexibleioS = FlexibleIO::getInstance();

int CloudFS::qtdS = 0;
int CloudFS::firstOutputCallS = 0;

void CloudFS::integrationS() {
    CloudS::integrationS();
    float porcent=0;
    if (values.size() > (unsigned) disease->getVectorSizeCloudF()) {
        values.erase(values.begin());
    }

    if (getValueS() > disease->getMaxSporeCloudsDensity()) {
        CloudS::removeSporesCloudS(getValueS() - disease->getMaxSporeCloudsDensity());
    }
    if (BasicS::getWeather()->getRain() >= 20) {
        porcent = BasicS::getWeather()->getRain() / 80;
        porcent = porcent>1?1:porcent;
        //std::cout<< "porcent : "<<porcent << std::endl;
        CloudS::removeSporesCloudByRainS(porcent);

        
    }

    std::ostringstream convert;
    convert << BasicS::getWeather()->getYearDoy() << "," << getValueS();
    for (unsigned int i = 0; i < values.size(); i++) {
        convert << "," << values[i];
    }
    BasicS::output.push_back(convert.str());
}

void CloudFS::outputS() {
    CloudS::outputS();

    std::ostringstream convert;
    convert << "Cpp_CloudF_" << getID() << ".txt";
    BasicS::getOutput(convert.str(),this->firstOutputCallS);
    this->firstOutputCallS++;

    // Speedup the model
    //std::cout << "\nCloudF" << ID << ":";
    //for(unsigned int i=0; i<BasicS::output.size(); i++)
    //    std::cout << BasicS::output[i] << std::endl;
}

