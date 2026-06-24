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
#include "debug_control.h"
#include "numericstringcache.h"
//#include "../../FlexibleIO/Data/FlexibleIO.hpp"

#include <iostream>
#include <sstream>
#include <cmath>

//extern FlexibleIO *flexibleio;

int CloudP::qtd = 0;
int CloudP::firstOutputCall = 0;

void CloudP::rate() {
    // Set current cloud for context
    gEqContext->cloud = shared_from_this();

    // Determine the daily inoculum removal from this cloud
    float inocRemovalValue = this->getDisease()->getINOC_REM()->evaluate();
#if GENERICPM_DEBUG_ENABLED
    std::cout << "[DIAG] CloudP INOC_REM on YEARDOY " << FlexibleIO::getInstance()->getReal("CONTROL", "YEARDOY") << ": " << inocRemovalValue << std::endl;
#endif
    this->addInoculumRemoved(inocRemovalValue);
    this->disease->reporter.track_inoculum_removed(inocRemovalValue);

    // Run the generic cloud rate function
    Cloud::rate();

    // Free the context
    gEqContext->cloud = nullptr;
}

void CloudP::integration() {
    // Refactor
    if (REMOVAL_METHOD == 1) {
        if (values.size() == (unsigned) disease->getVectorSizeCloudP()) {
#if GENERICPM_DEBUG_ENABLED
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

    // if rain > threshold:
    // - queue removal of 1 - exp(-0.035f * getRain()) %

    // // Update values according to additions and removals queued
    // Cloud::integration();

    std::ostringstream convert;
    convert << FlexibleIO::getInstance()->getReal("CONTROL", "YEARDOY") << "," << getValue();
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

void CloudP::addInoculumCreated(float activeInoculumCreated) {
    // Partition spores between this plant cloud and the parent field cloud.
    float toParent = activeInoculumCreated * disease->getProportionFromPlantToFieldCloud();
    float toSelf = activeInoculumCreated - toParent;
#if GENERICPM_DEBUG_ENABLED
    std::cout << "[DIAG] CloudP::addInoculumCreated called total=" << activeInoculumCreated << " toSelf=" << toSelf << " toParent=" << toParent << std::endl;
#endif
    Cloud::activeInoculumCreated += toSelf;
    if (cloudF) {
        cloudF->addInoculumCreated(toParent);
    }
}

void CloudP::addInoculumCreated(float inoculumCreated, int destination) {
    float toSelf, toParent;
    switch ((InoculumDestination)destination) {
        case InoculumDestination::DORMANT:
            DormantInoculum::getInstance()->addDormantInoculum(inoculumCreated, this->disease->getFamily());
            break;
        case InoculumDestination::INFECTIVE:
            toParent = inoculumCreated * disease->getProportionFromPlantToFieldCloud();
            this->activeInoculumCreated += inoculumCreated - toParent;
#if GENERICPM_DEBUG_ENABLED
            std::cout << "[DIAG] CloudP::addInoculumCreated called total=" << inoculumCreated << " toSelf=" << inoculumCreated - toParent << " toParent=" << toParent << std::endl;
#endif
            cloudF->addInoculumCreated(toParent);
            break;
        default:
            FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();
            std::string destinationStr = converter->decode(destination);
            if (destinationStr == "DORMANT") {
                DormantInoculum::getInstance()->addDormantInoculum(inoculumCreated, this->disease->getFamily());
            } else if (destinationStr == "INFECTIVE") {
                toParent = inoculumCreated * disease->getProportionFromOrganToPlantCloud();
                this->activeInoculumCreated += inoculumCreated - toParent;
                std::cout << "[DIAG] CloudP::addInoculumCreated total=" << activeInoculumCreated << " toSelf=" << inoculumCreated - toParent << " toParent=" << toParent << std::endl;
                cloudF->addInoculumCreated(toParent, destination);
            } else {
                throw std::runtime_error("Unknown InoculumDestination: " + destinationStr);
            }
            break;
    }
}

void CloudP::addInoculumRemoved(float activeInoculumRemoved) {
#if GENERICPM_DEBUG_ENABLED
    FlexibleIO* fio = FlexibleIO::getInstance();
    std::cout << "[DIAG] YEARDOY:" << fio->getReal("CONTROL", "YEARDOY") << " CloudP::addInoculumRemoved activeInoculumRemoved=" << activeInoculumRemoved << std::endl;
#endif
    this->activeInoculumRemoved += activeInoculumRemoved;
}

void CloudP::addInoculumRemoved(float inoculumRemoved, int destination) {
    switch ((InoculumDestination)destination) {
        case InoculumDestination::DORMANT:
            DormantInoculum::getInstance()->removeDormantInoculum(inoculumRemoved, this->disease->getFamily());
            break;
        case InoculumDestination::INFECTIVE:
            this->activeInoculumRemoved += inoculumRemoved;
            break;
    }
}