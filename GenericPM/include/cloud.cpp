/**
 * @file cloud.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "cloud.h"
#include "disease.h"
#include "simulator.h"
#include "project_config.h"
//#include "../../FlexibleIO/Data/FlexibleIO.hpp"

#include <cmath>
#include <iostream>
#include <sstream>

// All clouds share the same integration logic:
//   1. Remove spores that were used to infect tissue.
//   2. Add the queued spores to the values vector.
//
// Note that new spores get pushed to the end of the vector and the 
// ageing based removal deletes entries from the beginning of the 
// vector.
void Cloud::integration() {
    // diagSnapshot("integration_start");

    // Add new spores into cloud
    if (values.empty()) {
        values.push_back(sporesCreated);
    } else {
        if (REMOVAL_METHOD == 1) {
            incrementSporesAge();
        }
        values.back() += sporesCreated;
    }

    // If an age-based removal was queued, perform it now
    if (removeByAge) {
#if DIAG_SPORES
        std::cout << "[DIAG] YEARDOY:" << Basic::getWeather()->getYearDoy() << " age removal (integration): removing oldest bucket=" << values.front() << std::endl;
#endif
        values.erase(values.begin());
        removeByAge = false;
    }

    // If method 0: proportional daily removal is modelled via queued sporesToBeRemoved
    if (REMOVAL_METHOD == 0) {
        sporesToBeRemoved = getValue() * (K_G + K_D);
        if (sporesToBeRemoved > 0.0f) {
#if DIAG_SPORES
            std::cout << "[DIAG] YEARDOY:" << Basic::getWeather()->getYearDoy() << " method 0 queued proportional removal amount=" << sporesToBeRemoved << std::endl;
#endif
            removeSporesVal(sporesToBeRemoved);
        }
    } else if (REMOVAL_METHOD == 1) {
        // Density cap
        if (getValue() > disease->getMaxSporeCloudsDensity()) {
            float over = getValue() - disease->getMaxSporeCloudsDensity();
#if DIAG_SPORES
            std::cout << "[DIAG] YEARDOY:" << Basic::getWeather()->getYearDoy() << " density cap removal amount=" << over << std::endl;
#endif
            removeSporesVal(over);
        }

        // Infection-driven queued removals
        if (sporesToBeRemoved > 0.0f) {
#if DIAG_SPORES
            std::cout << "[DIAG] YEARDOY:" << Basic::getWeather()->getYearDoy() << " queued infection removal amount=" << sporesToBeRemoved << std::endl;
#endif
            removeSporesVal(sporesToBeRemoved);
        }

        // Rain effect
        if (Basic::getWeather()->getRain() >= disease->getMRRS()) {
            float percent = 1 - exp(-0.035f * Basic::getWeather()->getRain());
#if DIAG_SPORES
            std::cout << "[DIAG] YEARDOY:" << Basic::getWeather()->getYearDoy() << " rain effect percent=" << percent << " rain=" << Basic::getWeather()->getRain() << std::endl;
#endif
            removeSporesPct(1 - percent);
        }
    }

    // diagSnapshot("integration_end");

    // Reset counters
    sporesCreated = 0.0f;
    sporesToBeRemoved = 0.0f;

    // // Original
    // int qtd = 0;

    // // Clear out the removal queue
    // if (sporesToBeRemoved > 0) {
    //     // Remove from cloud spores used to infect tissue
    //     removeSporesVal(sporesToBeRemoved);
    // }

    // if (removeByAge) {
    //     values.erase(values.begin());
    //     removeByAge = false;
    // }

    // if (REMOVAL_METHOD == 1){
    //     // Age the spores in the cloud (only for method 1)
    //     incrementSporesAge();
    // }

    // // Add new spores into cloud
    // if (values.size() == 0) {
    //     values.push_back(sporesCreated);
    // } else {
    //     values.back() += sporesCreated;
    // }
    
    // // Reset created and to-be-removed counters
    // sporesCreated = 0.0f;
    // sporesToBeRemoved = 0.0f;
}

float Cloud::getValue() {
    // Validate this pointer
    // std::cout << "Cloud::getValue() called on " << this << std::endl;
    // std::cout << "Expected Cloud object? " << (typeid(*this).name()) << std::endl;

    float sum = 0.0f;

    #ifdef DEBUGX
    if (this->disease == nullptr) {
        std::cout << "getValue() called on a cloud with a nullptr for disease." << std::endl;
        printf("Address of disease: %p\n", (void *)this->disease);
    }
    #endif // DEBUGX
    
    // Sanitize values: replace non-finite values with 0 and accumulate
    for (auto& value : values) {
        if (!std::isfinite(value)) {
            #ifdef DEBUGX
            std::cout << "Sanitizing non-finite value in cloud values: " << value << " -> 0" << std::endl;
            #endif
            value = 0.0f;
        }
        #ifdef DEBUGX
        std::cout << "\tValue: " << value << std::endl;
        #endif
        sum += value;
    }
    return sum;
}

void Cloud::removeSporesVal(float toBeRemoved) {
    if (toBeRemoved <= 0.0f) return;
    float total = getValue();

#if DIAG_SPORES
    std::cout << "[DIAG] YEARDOY:" << Basic::getWeather()->getYearDoy() << " removeSporesVal requested=" << toBeRemoved << " total=" << total << std::endl;
#endif

    // If there is nothing in the cloud, nothing to remove.
    if (!std::isfinite(total) || total <= 0.0f) {
#if DIAG_SPORES
        std::cout << "[DIAG] YEARDOY:" << Basic::getWeather()->getYearDoy() << " removeSporesVal: nothing to remove (total non-finite or <=0)" << std::endl;
#endif
        values.clear();
        return;
    }

    // If trying to remove >= total, just zero all entries
    if (toBeRemoved >= total) {
#if DIAG_SPORES
        std::cout << "[DIAG] YEARDOY:" << Basic::getWeather()->getYearDoy() << " removeSporesVal: removing all entries (toBeRemoved >= total)" << std::endl;
#endif
        values.clear();
        return;
    }

    // Compute factor once and subtract proportionally
    float factor = toBeRemoved / total;
    if (!std::isfinite(factor) || factor <= 0.0f) return;
    for (auto& value : values) {
        float before = value;
        value -= (value * factor);
        // Guard again against any rounding errors producing non-finite
        if (!std::isfinite(value)) value = 0.0f;
        if (value < 0.0f) value = 0.0f;
#if DIAG_SPORES
        std::cout << "[DIAG] YEARDOY:" << Basic::getWeather()->getYearDoy() << " removeSporesVal bucket before=" << before << " after=" << value << std::endl;
#endif
    }
}

void Cloud::removeSporesPct(float percent) {
    for (auto& value : values) {
#if DIAG_SPORES
        std::cout << "[DIAG] YEARDOY:" << Basic::getWeather()->getYearDoy() << " removeSporesPct bucket before=" << value;
#endif
        value *= percent;
#if DIAG_SPORES
        std::cout << " after=" << value << std::endl;
#endif
    }
}

void Cloud::incrementSporesAge() {
    values.push_back(0.0f);
}