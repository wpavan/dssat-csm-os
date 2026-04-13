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
#include "project_config.h"
//#include "../../FlexibleIO/Data/FlexibleIO.hpp"

#include <iostream>
#include <sstream>
#include <cmath>

//extern FlexibleIO *flexibleio;

int CloudP::qtd = 0;
int CloudP::firstOutputCall = 0;

void CloudP::rate() {
    // Set current cloud for context
    gEqContext->cloud = this;

    // Run the generic cloud rate function
    Cloud::rate();

    // Free the context
    gEqContext->cloud = nullptr;
}

void CloudP::integration() {
    // Refactor
    if (REMOVAL_METHOD == 1) {
        if (values.size() == (unsigned) disease->getVectorSizeCloudP()) {
            #if DIAG_SPORES
            std::cout << "[DIAG] CloudP::integration queueing age removal (size==vectorSizeCloudP)" << std::endl;
            #endif
            queueAgeRemoval();
        }
    }

    Cloud::integration();

    // // Original 
    // if (REMOVAL_METHOD == 0) {
    //     // This is the method that is used in the R code. This 
    //     // method involves proportional removal of cloud spores each
    //     // day based on constants k_g and k_d.
    //     // Queue proportional removal instead of directly modifying values
    //     addSporesToBeRemoved(getValue() * (K_G + K_D));
    // } else if (REMOVAL_METHOD == 1) {
    //     if (values.size() > (unsigned) disease->getVectorSizeCloudF()) {
    //         queueAgeRemoval();
    //     }

    //     if (getValue() > disease->getMaxSporeCloudsDensity()) {
    //         addSporesToBeRemoved(getValue() - disease->getMaxSporeCloudsDensity());
    //     }

    //     if (Basic::getWeather()->getRain() >= disease->getMRRS()) {
    //         float percent = 1 - exp(-0.035f * Basic::getWeather()->getRain());
    //         // removed amount = oldTotal - newTotal = oldTotal * (1 - percent)
    //         float removed = getValue() - (getValue() * percent);
    //         addSporesToBeRemoved(removed);
    //     }
    // }

    // // Update values according to additions and removals queued
    // Cloud::integration();

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
    #ifdef OUTPUT
    std::cout << "\nCloudP" << getID() << ":";
    for(unsigned int i=0; i<Basic::output.size(); i++) {
       std::cout << Basic::output[i] << std::endl;
    }
    #endif // OUTPUT
}

void CloudP::addSporesCreated(float sporesCreated) {
    // Partition spores between this plant cloud and the parent field cloud.
    float toParent = sporesCreated * disease->getProportionFromPlantToFieldCloud();
    float toSelf = sporesCreated - toParent;
    #if DIAG_SPORES
    std::cout << "[DIAG] CloudP::addSporesCreated called total=" << sporesCreated << " toSelf=" << toSelf << " toParent=" << toParent << std::endl;
    #endif
    Cloud::sporesCreated += toSelf;
    if (cloudF) {
        cloudF->addSporesCreated(toParent);
    }
}
