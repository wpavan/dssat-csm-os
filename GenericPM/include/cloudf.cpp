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

#include <iostream>
#include <sstream>
#include <cmath>

int CloudF::firstOutputCall = 0;

void CloudF::rate() {
    if (lastRate != Basic::getWeather()->getDoy()) {
        Cloud::rate();
        #ifdef DEBUGX        
        std::cout << "YEARDOY: " << weather->getYearDoy() << "\n\t- CloudF family: " << family << "\n\t- Total Spores in CloudF: " << getValue() << std::endl;
        #endif // DEBUG

        // Ensure we only run this once per day
        lastRate = Basic::getWeather()->getDoy();

        // Add the inoculum value into FlexibleIO
        FlexibleIO *fio = FlexibleIO::getInstance();
        fio->setRealMemory(family, "INOCULUM", getValue());
        // std::cout << "Just set " << family << ":INOCULUM to " << getValue() << std::endl;
    }
}

void CloudF::integration() {
    if (lastIntegration != Basic::getWeather()->getDoy()) {
        if (firstSporeCloud > 0) {
                #if DIAG_SPORES
                std::cout << "[DIAG] YEARDOY:" << Basic::getWeather()->getYearDoy() << " CloudF::integration seeding firstSporeCloud=" << firstSporeCloud << std::endl;
                #endif
                values.push_back(firstSporeCloud);
                firstSporeCloud = 0;
        }

        // Refactor
        else if (REMOVAL_METHOD == 1) {
            if (values.size() == (unsigned) disease->getVectorSizeCloudF()) {
                #if DIAG_SPORES
                std::cout << "[DIAG] YEARDOY:" << Basic::getWeather()->getYearDoy() << " CloudF::integration queueing age removal (size==vectorSizeCloudF)" << std::endl;
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

        //     if (Basic::getWeather()->getRain() >= disease->getMRRS()) {
        //         float percent = 1 - exp(-0.035f * Basic::getWeather()->getRain());
        //         float removed = getValue() - (getValue() * percent);
        //         addSporesToBeRemoved(removed);
        //     }
        // }
        
        std::ostringstream convert;
        convert << Basic::getWeather()->getYearDoy() << "," << getValue();
        for (auto& value : values) {
            convert << "," << value;
        }
        Basic::output.push_back(convert.str());
        
        // Ensure we only run this once per day
        lastIntegration = Basic::getWeather()->getDoy();
    }
}

void CloudF::output() {
    if (lastOutput != Basic::getWeather()->getDoy()) {
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
        lastOutput = Basic::getWeather()->getDoy();
    }
    
}
