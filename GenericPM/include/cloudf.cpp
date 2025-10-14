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

#include <iostream>
#include <sstream>
#include <cmath>

int CloudF::firstOutputCall = 0;

void CloudF::rate() {
    if (lastRate != Basic::getWeather()->getDoy()) {
        Cloud::rate();
        #ifdef DEBUG
        std::cout << "YEARDOY: " << weather->getYearDoy() << "\n\t- CloudF family: " << family << "\n\t- Total Spores in CloudF: " << getValue() << std::endl;
        #endif // DEBUG

        // Ensure we only run this once per day
        lastRate = Basic::getWeather()->getDoy();
    }
}

void CloudF::integration() {
    if (lastIntegration != Basic::getWeather()->getDoy()) {
        if (firstSporeCloud > 0) {
            values.push_back(firstSporeCloud);
            firstSporeCloud = 0;
        }

        Cloud::integration();
        if (values.size() > (unsigned) disease->getVectorSizeCloudF()) {
            values.erase(values.begin());
        }
        if (getValue() > disease->getMaxSporeCloudsDensity()) {
            Cloud::removeSporesCloud(getValue() - disease->getMaxSporeCloudsDensity());
        }
        if (Basic::getWeather()->getRain() >= disease->getMRRS()) {
            // Here we should parameterize the rain effect on the spores cloud
            Cloud::removeSporesCloudByRain(1-exp(-0.035*Basic::getWeather()->getRain()));
        }

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
