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

static double TE_isFieldCloud(void) {
    return gEqContext && gEqContext->cloud && (gEqContext->cloud->getLevel() == CloudLevel::FIELD) ? 1.0 : 0.0;
}

static double TE_isPlantCloud(void) {
    return gEqContext && gEqContext->cloud && (gEqContext->cloud->getLevel() == CloudLevel::PLANT) ? 1.0 : 0.0;
}

static double TE_isOrganCloud(void) {
    return gEqContext && gEqContext->cloud && (gEqContext->cloud->getLevel() == CloudLevel::ORGAN) ? 1.0 : 0.0;
}

static double TE_getThisCloudValue(void) {
    return gEqContext && gEqContext->cloud ? static_cast<double>(gEqContext->cloud->getValue()) : 0.0;
}

namespace {
    struct FunctionRegistrar {
        FunctionRegistrar() {
            getCustomFunctions().register_context_function({"IS_FIELD_CLOUD", TE_isFieldCloud});
            getCustomFunctions().register_context_function({"IS_PLANT_CLOUD", TE_isPlantCloud});
            getCustomFunctions().register_context_function({"IS_ORGAN_CLOUD", TE_isOrganCloud});
            getCustomFunctions().register_context_function({"CLOUD_INOCULUM", TE_getThisCloudValue});
        }
    };

    // Static instance to trigger the registration at program startup
    static FunctionRegistrar registrar;
}

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
    // if (removeByAge) {
    //     values.erase(values.begin());
    //     removeByAge = false;
    // }

    if (REMOVAL_METHOD == 1) {
        // Density cap
        if (getValue() > disease->getMaxSporeCloudsDensity()) {
            float over = getValue() - disease->getMaxSporeCloudsDensity();
            removeSporesVal(over);
        }

        // Infection-driven queued removals
        if (sporesToBeRemoved > 0.0f) {
            removeSporesVal(sporesToBeRemoved);
        }

        // Rain effect
        // NOTE: This should be taken care of in the .yaml input file. It is 
        //       important to note that this percentage-based removal indicates
        //       the need for the current inoculum value to be available in FIO 
        //       or available through some context.
        //
        // if (Basic::getWeather()->getRain() >= disease->getMRRS()) {
        //     float percent = 1 - exp(-0.035f * Basic::getWeather()->getRain());
        //     removeSporesPct(1 - percent);
        // }
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
    //std::cout << "this pointer: " << this << std::endl;
    //std::cout << "values container address: " << &values << std::endl;
    //std::cout << "values.size(): " << values.size() << std::endl;
    
    float sum = 0.0f;

    #ifdef DEBUGX
    if (this->disease == nullptr) {
        std::cout << "getValue() called on a cloud with a nullptr for disease." << std::endl;
        printf("Address of disease: %p\n", (void *)this->disease);
    }
    #endif // DEBUGX
    
    if (values.empty()) return 0.0f;

    // Sanitize values: replace non-finite values with 0 and accumulate
    for (size_t i = 0; i < values.size(); ++i) {
        try {
            float& value = values[i];
            if (!std::isfinite(value)) {
                #ifdef DEBUGX
                std::cout << "Sanitizing non-finite value in cloud values: " << value << " -> 0" << std::endl;
                #endif
                value = 0.0f;
            }
            sum += value;
        } catch (const std::exception& e) {
            std::cout << "Exception while checking cloud value for finiteness: " << e.what() << std::endl;
        }  
    }
    return sum;
}

void Cloud::removeSporesVal(float toBeRemoved) {
    if (toBeRemoved <= 0.0f) return;
    float total = getValue();

#if DIAG_SPORES
    std::cout << "[DIAG] YEARDOY:" << FlexibleIO::getInstance()->getReal("CONTROL", "YEARDOY") << " removeSporesVal requested=" << toBeRemoved << " total=" << total << std::endl;
#endif

    // If there is nothing in the cloud, nothing to remove.
    if (!std::isfinite(total) || total <= 0.0f) {
#if DIAG_SPORES
        std::cout << "[DIAG] YEARDOY:" << FlexibleIO::getInstance()->getReal("CONTROL", "YEARDOY") << " removeSporesVal: nothing to remove (total non-finite or <=0)" << std::endl;
#endif
        values.clear();
        return;
    }

    // If trying to remove >= total, just zero all entries
    if (toBeRemoved >= total) {
#if DIAG_SPORES
        std::cout << "[DIAG] YEARDOY:" << FlexibleIO::getInstance()->getReal("CONTROL", "YEARDOY") << " removeSporesVal: removing all entries (toBeRemoved >= total)" << std::endl;
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
        std::cout << "[DIAG] YEARDOY:" << FlexibleIO::getInstance()->getReal("CONTROL", "YEARDOY") << " removeSporesVal bucket before=" << before << " after=" << value << std::endl;
#endif
    }
}

void Cloud::removeSporesPct(float percent) {
    for (auto& value : values) {
#if DIAG_SPORES
        std::cout << "[DIAG] YEARDOY:" << FlexibleIO::getInstance()->getReal("CONTROL", "YEARDOY") << " removeSporesPct bucket before=" << value;
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