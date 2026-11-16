/**
 * @file cloudo.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * @author Virginia L. Covert (virginiacovert454@gmail.com)
 * 
 * @copyright Copyright (c) 2017–2026, University of Florida
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "cloudo.h"
#include "project_config.h"
#include "debug_control.h"
//#include "../../FlexibleIO/Data/FlexibleIO.hpp"
#include "numericstringcache.h"

#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

//extern FlexibleIO *flexibleio;

int CloudO::qtd = 0;
int CloudO::firstOutputCall = 0;

void CloudO::rate() {
    // Set current cloud for context
    gEqContext->cloud = shared_from_this();

    // Determine the daily inoculum removal from this cloud
    float inocRemovalValue = this->getDisease()->getINOC_REM()->evaluate();
#if GENERICPM_DEBUG_ENABLED
    std::cout << "[DIAG] CloudO INOC_REM on YEARDOY " << FlexibleIO::getInstance()->getReal("CONTROL", "YEARDOY") << ": " << inocRemovalValue << std::endl;
#endif
    this->addInoculumRemoved(inocRemovalValue);
    this->disease->reporter.track_inoculum_removed(inocRemovalValue);

    // Run the generic cloud rate function
    Cloud::rate();

    // Free the context
    gEqContext->cloud = nullptr;
}

void CloudO::integration() {
    // Refactor
    if (REMOVAL_METHOD == 1) {
        if (values.size() == (unsigned) disease->getVectorSizeCloudO()) {
#if GENERICPM_DEBUG_ENABLED
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
#if GENERICPM_DEBUG_ENABLED
    std::cout << "[DIAG] CloudO::addInoculumCreated total=" << activeInoculumCreated << " toSelf=" << toSelf << " toParent=" << toParent << std::endl;
#endif
    Cloud::activeInoculumCreated += toSelf;
    if (cloudP) {
        cloudP->addInoculumCreated(toParent);
    }
}

void CloudO::addInoculumCreated(float inoculumCreated, int destination) {
    float toParent, toSelf;
    switch ((InoculumDestination)destination) {
        case InoculumDestination::DORMANT:
#if GENERICPM_DEBUG_ENABLED
            std::cout << "DORMANT" << std::endl;
#endif
            DormantInoculum::getInstance()->addDormantInoculum(inoculumCreated, this->disease->getFamily());
            break;
        case InoculumDestination::INFECTIVE:
            toParent = inoculumCreated * disease->getProportionFromOrganToPlantCloud();
            this->activeInoculumCreated += inoculumCreated - toParent;
#if GENERICPM_DEBUG_ENABLED
            std::cout << "[DIAG] CloudO::addInoculumCreated total=" << activeInoculumCreated << " toSelf=" << inoculumCreated - toParent << " toParent=" << toParent << std::endl;
#endif
            cloudP->addInoculumCreated(toParent, destination);
            break;
        default:
            FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();
            std::string destinationStr = converter->decode(destination);
            if (destinationStr == "DORMANT") {
                DormantInoculum::getInstance()->addDormantInoculum(inoculumCreated, this->disease->getFamily());
            } else if (destinationStr == "INFECTIVE") {
                toParent = inoculumCreated * disease->getProportionFromOrganToPlantCloud();
                this->activeInoculumCreated += inoculumCreated - toParent;
#if GENERICPM_DEBUG_ENABLED
                std::cout << "[DIAG] CloudO::addInoculumCreated total=" << activeInoculumCreated << " toSelf=" << inoculumCreated - toParent << " toParent=" << toParent << std::endl;
#endif
                cloudP->addInoculumCreated(toParent, destination);
            } else {
                throw std::runtime_error("Unknown InoculumDestination: " + destinationStr);
            }
            break;
    }
}

void CloudO::addInoculumRemoved(float activeInoculumRemoved) {
#if GENERICPM_DEBUG_ENABLED
    FlexibleIO* fio = FlexibleIO::getInstance();
    std::cout << "[DIAG] YEARDOY:" << fio->getReal("CONTROL", "YEARDOY") << " CloudO::addInoculumRemoved activeInoculumRemoved=" << activeInoculumRemoved << std::endl;
#endif
    this->activeInoculumRemoved += activeInoculumRemoved;
}

void CloudO::addInoculumRemoved(float inoculumRemoved, int destination) {
    switch ((InoculumDestination)destination) {
        case InoculumDestination::DORMANT:
            DormantInoculum::getInstance()->removeDormantInoculum(inoculumRemoved, this->disease->getFamily());
            break;
        case InoculumDestination::INFECTIVE:
            this->activeInoculumRemoved += inoculumRemoved;
            break;
    }
}
