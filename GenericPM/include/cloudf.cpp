/**
 * @file cloudf.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "cloudf.h"
#include "project_config.h"
#include "debug_control.h"
#include "numericstringcache.h"

#include <iostream>
#include <sstream>
#include <cmath>

int CloudF::firstOutputCall = 0;

void CloudF::rate() {
    FlexibleIO *fio = FlexibleIO::getInstance();
    if (lastRate != fio->getInteger("CONTROL", "YEARDOY")) {
        // Set current cloud for context
        gEqContext->cloud = shared_from_this();

        // Determine the daily inoculum removal from this cloud
        float inocRemovalValue = disease->getINOC_REM()->evaluate();
#if GENERICPM_DEBUG_ENABLED
        std::cout << "[DIAG] CloudF INOC_REM on YEARDOY " << fio->getInteger("CONTROL", "YEARDOY") << ": " << inocRemovalValue << std::endl;
#endif
        this->addInoculumRemoved(inocRemovalValue);
        this->disease->reporter.track_inoculum_removed(inocRemovalValue);

        // Run the generic cloud rate function
        Cloud::rate();

        // Ensure we only run this once per day
        lastRate = fio->getInteger("CONTROL", "YEARDOY");

        // Add the inoculum value into FlexibleIO
        FlexibleIO *fio = FlexibleIO::getInstance();
        fio->setRealMemory(disease->getDiseaseID(), "INOCULUM", getValue());
        
        // Free the context
        gEqContext->cloud = nullptr;
    }
}

void CloudF::integration() {
    FlexibleIO *fio = FlexibleIO::getInstance();
    if (lastIntegration != fio->getInteger("CONTROL", "YEARDOY")) {
        if (firstSporeCloud > 0) {
#if GENERICPM_DEBUG_ENABLED
            std::cout << "[DIAG] YEARDOY:" << fio->getReal("CONTROL", "YEARDOY") << " CloudF::integration seeding firstSporeCloud=" << firstSporeCloud << std::endl;
#endif
            values.push_back(firstSporeCloud);
            firstSporeCloud = 0;
        }

        // Refactor
        else if (REMOVAL_METHOD == 1) {
            if (values.size() == (unsigned) disease->getVectorSizeCloudF()) {
#if GENERICPM_DEBUG_ENABLED
                std::cout << "[DIAG] YEARDOY:" << fio->getReal("CONTROL", "YEARDOY") << " CloudF::integration queueing age removal (size==vectorSizeCloudF)" << std::endl;
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
        //     addSporesToBeRemoved(getValue() * (K_G + K_D));
        // } else if (REMOVAL_METHOD == 1) {
        //     if (values.size() > (unsigned) disease->getVectorSizeCloudF()) {
        //         values.erase(values.begin());
        //     }

        //     if (getValue() > disease->getMaxSporeCloudsDensity()) {
        //         addSporesToBeRemoved(getValue() - disease->getMaxSporeCloudsDensity());
        //     }

        //     if (getRain() >= disease->getMRRS()) {
        //         float percent = 1 - exp(-0.035f * getRain());
        //         float removed = getValue() - (getValue() * percent);
        //         addSporesToBeRemoved(removed);
        //     }
        // }
        
        std::ostringstream convert;
        convert << fio->getReal("CONTROL", "YEARDOY") << "," << getValue();
        for (auto& value : values) {
            convert << "," << value;
        }
        Basic::output.push_back(convert.str());
        
        // Ensure we only run this once per day
        lastIntegration = fio->getInteger("CONTROL", "YEARDOY");
    }
}

void CloudF::output() {
    FlexibleIO *fio = FlexibleIO::getInstance();
    if (lastOutput != fio->getInteger("CONTROL", "YEARDOY")) {
        Cloud::output();

        std::ostringstream convert;
        convert << "Cpp_CloudF_" << getID() << ".txt";
        Basic::getOutput(convert.str(),this->firstOutputCall);
        this->firstOutputCall++;

        // Speedup the model
        #ifdef OUTPUT
        std::cout << "\nCloudF" << ID << ":";
        for(unsigned int i=0; i<Basic::output.size(); i++) {
           std::cout << Basic::output[i] << std::endl;
        }
        #endif // OUTPUT

        // Ensure we only run this once per day
        lastOutput = fio->getInteger("CONTROL", "YEARDOY");
    }
    
}

void CloudF::addInoculumCreated(float activeInoculumCreated) {
    FlexibleIO* fio = FlexibleIO::getInstance();
#if GENERICPM_DEBUG_ENABLED
    std::cout << "[DIAG] YEARDOY:" << fio->getReal("CONTROL", "YEARDOY") << " CloudF::addInoculumCreated activeInoculumCreated=" << activeInoculumCreated << std::endl;
#endif
    this->activeInoculumCreated += activeInoculumCreated;
}

void CloudF::addInoculumCreated(float inoculumCreated, int destination) {
#if GENERICPM_DEBUG_ENABLED
    std::cerr << "[CLOUDF] addInoculumCreated called for family=" << this->disease->getDiseaseID() << ", inoculumCreated=" << inoculumCreated << ", destination=" << destination << ", this=" << (void*)this << std::endl << std::flush;
#endif
    switch ((InoculumDestination)destination) {
        case InoculumDestination::DORMANT:
            DormantInoculum::getInstance()->addDormantInoculum(inoculumCreated, this->disease->getDiseaseID());
#if GENERICPM_DEBUG_ENABLED
            std::cerr << "[CLOUDF] Added to DORMANT inoculum" << std::endl << std::flush;
#endif
            break;
        case InoculumDestination::INFECTIVE: {
            FlexibleIO* fio = FlexibleIO::getInstance();
#if GENERICPM_DEBUG_ENABLED
            std::cout << "[DIAG] YEARDOY:" << fio->getReal("CONTROL", "YEARDOY") << " CloudF::addInoculumCreated activeInoculumCreated=" << activeInoculumCreated << std::endl;
#endif
            this->activeInoculumCreated += inoculumCreated;
#if GENERICPM_DEBUG_ENABLED
            std::cerr << "[CLOUDF] After increment, activeInoculumCreated=" << this->activeInoculumCreated << std::endl << std::flush;
#endif
            break;
        }
        default:
            FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();
            std::string destinationStr = converter->decode(destination);
            if (destinationStr == "DORMANT") {
                DormantInoculum::getInstance()->addDormantInoculum(inoculumCreated, this->disease->getFamily());
            } else if (destinationStr == "INFECTIVE") {
                this->activeInoculumCreated += inoculumCreated;
                std::cout << "[DIAG] CloudF::addInoculumCreated total=" << activeInoculumCreated << std::endl;
            } else {
                throw std::runtime_error("Unknown InoculumDestination: " + destinationStr);
            }
            break;
    }
}

void CloudF::addInoculumRemoved(float activeInoculumRemoved) {
#if GENERICPM_DEBUG_ENABLED
    FlexibleIO* fio = FlexibleIO::getInstance();
    std::cout << "[DIAG] YEARDOY:" << fio->getReal("CONTROL", "YEARDOY") << " CloudF::addInoculumRemoved activeInoculumRemoved=" << activeInoculumRemoved << std::endl;
#endif
    this->activeInoculumRemoved += activeInoculumRemoved;
}

void CloudF::addInoculumRemoved(float inoculumRemoved, int destination) {
    switch ((InoculumDestination)destination) {
        case InoculumDestination::DORMANT:
            DormantInoculum::getInstance()->removeDormantInoculum(inoculumRemoved, this->disease->getDiseaseID());
            break;
        case InoculumDestination::INFECTIVE:
            this->activeInoculumRemoved += inoculumRemoved;
            break;
    }
}