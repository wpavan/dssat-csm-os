/**
 * @file cloudp.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */
#include "cloudp.h"
#include<iostream>
#include<sstream>
//#include "../Coupling-Interface/CPP/coupling_IO_cpp.h"
//#include "../../FlexibleIO/Data/FlexibleIO.hpp"

//extern FlexibleIO *flexibleio;
int CloudP::qtd = 0;
int CloudP::firstOutputCall = 0;

void CloudP::integration() {
    Cloud::integration();

    //std::cout << values.size()<<std::endl;
    if (values.size() > (unsigned) disease->getVectorSizeCloudP()) {
        values.erase(values.begin());
    }
    Cloud::removeSporesCloudPByAge();

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

void CloudP::output() {
    Cloud::output();

    std::ostringstream convert;
    convert << "Cpp_CloudP_" << getID() << ".txt";
    Basic::getOutput(convert.str(),this->firstOutputCall);
    this->firstOutputCall++;

    // Speedup the model removing outputs
    //std::cout << "\nCloudP" << getID() << ":";
    //for(unsigned int i=0; i<Basic::output.size(); i++)
    //    std::cout << Basic::output[i] << std::endl;
}

void CloudP::addSporesCreated(double sporesCreated) {
    Cloud::sporesCreated += (sporesCreated * (1 - disease->getProportionFromPlantToFieldCloud()));
    cloudF->addSporesCreated(sporesCreated * disease->getProportionFromPlantToFieldCloud());
}
