/**
 * @file cloudo.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "cloudo.h"
#include "project_config.h"
//#include "../../FlexibleIO/Data/FlexibleIO.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

//extern FlexibleIO *flexibleio;

int CloudO::qtd = 0;
int CloudO::firstOutputCall = 0;

void CloudO::rate() {
    // Set current cloud for context
    gEqContext->cloud = this;

    // Run the generic cloud rate function
    Cloud::rate();

    // Free the context
    gEqContext->cloud = nullptr;
}

void CloudO::integration() {
    // Refactor
    if (REMOVAL_METHOD == 1) {
        if (values.size() == (unsigned) disease->getVectorSizeCloudO()) {
            #if DIAG_SPORES
            std::cout << "[DIAG] CloudO::integration queueing age removal (size==vectorSizeCloudO)" << std::endl;
            #endif
            queueAgeRemoval();
        }
    }

    Cloud::integration();

    // Cloud::integration();

    // if (REMOVAL_METHOD == 0) {
    //     // This is the method that is used in the R code. This 
    //     // method involves proportional removal of cloud spores each
    //     // day based on constants k_g and k_d.
    //     // Queue proportional removal instead of directly modifying values
    //     addSporesToBeRemoved(getValue() * (K_G + K_D));
    // } else if (REMOVAL_METHOD == 1) {
    //     if (values.size() > (unsigned) disease->getVectorSizeCloudF()) {
    //         values.erase(values.begin());
    //     }

    //     if (getValue() > disease->getMaxSporeCloudsDensity()) {
    //         addSporesToBeRemoved(getValue() - disease->getMaxSporeCloudsDensity());
    //     }

    // if rain > threshold:
    // - queue removal of 1 - exp(-0.035f * getRain()) %

    std::ostringstream convert;
    convert << FlexibleIO::getInstance()->getReal("CONTROL", "YEARDOY") << "," << getValue();
    for (auto& value : values) {
        convert << "," << value;
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
    #ifdef OUTPUT
    std::cout << "\nCloudO" << ID << ":";
    for(unsigned int i=0; i<Basic::output.size(); i++) {
        std::cout << Basic::output[i] << std::endl;
    }
    #endif // OUTPUT
}

void CloudO::addInoculumCreated(float activeInoculumCreated) {
    float toParent = activeInoculumCreated * disease->getProportionFromOrganToPlantCloud();
    float toSelf = activeInoculumCreated - toParent;
    #if DIAG_SPORES
    std::cout << "[DIAG] CloudO::addInoculumCreated total=" << activeInoculumCreated << " toSelf=" << toSelf << " toParent=" << toParent << std::endl;
    #endif
    Cloud::activeInoculumCreated += toSelf;
    if (cloudP) {
        cloudP->addInoculumCreated(toParent);
    }
}

void CloudO::addInoculumCreated(float inoculumCreated, InoculumDestination destination) {
    float toParent, toSelf;
    switch (destination) {
        case InoculumDestination::DORMANT:
            DormantInoculum::getInstance()->addDormantInoculum(inoculumCreated, this->disease->getFamily());
            break;
        case InoculumDestination::INFECTIVE:
            toParent = inoculumCreated * disease->getProportionFromOrganToPlantCloud();
            this->activeInoculumCreated += inoculumCreated - toParent;
            cloudP->addInoculumCreated(toParent, destination);
            break;
    }
}
