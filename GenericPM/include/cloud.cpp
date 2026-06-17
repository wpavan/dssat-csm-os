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
#include "debug_control.h"

#include <cmath>
#include <iostream>
#include <sstream>

static double TE_isFieldCloud(void) {
    if (gEqContext) {
        if (gEqContext->cloud) {
            return (gEqContext->cloud->getLevel() == CloudLevel::FIELD) ? 1.0 : 0.0;
        } else {
            std::cerr << "Warning: TE_isFieldCloud called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    }
    return 0.0;
}

static double TE_isPlantCloud(void) {
    if (gEqContext) {
        if (gEqContext->cloud) {
            return (gEqContext->cloud->getLevel() == CloudLevel::PLANT) ? 1.0 : 0.0;
        } else {
            std::cerr << "Warning: TE_isPlantCloud called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    }
    return 0.0;
}

static double TE_isOrganCloud(void) {
    if (gEqContext) {
        if (gEqContext->cloud) {
            return (gEqContext->cloud->getLevel() == CloudLevel::ORGAN) ? 1.0 : 0.0;
        } else {
            std::cerr << "Warning: TE_isOrganCloud called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    }
    return 0.0;
}

static double TE_getThisCloudValue(void) {
    // NOTE: Add a yeardoy piece to the debug statement so we can figure out if it's calling too many times?
    // I wonder if calling it a bunch is either moving a pointer from where it should be, or moving a value into garbage memory.
    if (gEqContext) {
        if (gEqContext->cloud) {
            return static_cast<double>(gEqContext->cloud->getValue());
        } else {
            std::cerr << "Warning: TE_getThisCloudValue called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    }
    return 0.0;
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

std::unordered_map<std::string, float> DormantInoculum::amountByID; 
DormantInoculum* DormantInoculum::instance = nullptr;

// All clouds share the same integration logic:
//   1. Remove spores that were used to infect tissue.
//   2. Add the queued spores to the values vector.
//
// Note that new spores get pushed to the end of the vector and the 
// ageing based removal deletes entries from the beginning of the 
// vector.
void Cloud::integration() {
    // ===== Add new inoculum =====
    // Add new infective inoculum into clouds
#if GENERICPM_DEBUG_ENABLED
    std::cerr << "[CLOUD] integration() called, activeInoculumCreated=" << activeInoculumCreated << ", values.size()=" << values.size() << std::endl << std::flush;
#endif
    if (values.empty()) {
        values.push_back(activeInoculumCreated);
    } else {
        if (disease->getINOC_AGE()) {
            incrementSporesAge();
        }
        values.back() += activeInoculumCreated;
    }

    // Add new dormant inoculum into cloud
    // dormantInoculum += dormantInoculumCreated;

    // ===== Remove inoculum =====
    // Remove active inoculum according to queued removal (INOC_REM expression)
    // This should include most/all effects including:
    // - Infection-driven removal (via # of new lesions)
    // - Rain-driven removal (via fio->rain)
    // - Density cap removal (via TinyExpr++ functions)
    // - Age-based removal (NEED TO IMPLEMENT NEW METHOD)
    // - UV-based removal (also via fio)
    if (activeInoculumRemoved > 0.0f) {
        removeSporesVal(activeInoculumRemoved);
    }

    // If an age-based removal was queued, perform it now
    // if (removeByAge) {
    //     values.erase(values.begin());
    //     removeByAge = false;
    // }

    // Reset counters
    activeInoculumCreated = 0.0f;
    activeInoculumRemoved = 0.0f;
}

float Cloud::getValue() {
    float sum = 0.0f;
    
    if (values.empty()) return 0.0f;

    // Sanitize values: replace non-finite values with 0 and accumulate
    for (size_t i = 0; i < values.size(); ++i) {
        try {
            float& value = values[i];
            if (!std::isfinite(value)) {
                value = 0.0f;
            }
            sum += value;
            // std::cout << "Index: " << i << "\tValue: " << value << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Exception while checking cloud value for finiteness: " << e.what() << std::endl;
        }  
    }
    return sum;
}

void Cloud::removeSporesVal(float toBeRemoved) {
    if (toBeRemoved <= 0.0f) return;
    float total = getValue();

#if GENERICPM_DEBUG_ENABLED
    std::cout << "[DIAG] YEARDOY:" << FlexibleIO::getInstance()->getReal("CONTROL", "YEARDOY") << " removeSporesVal requested=" << toBeRemoved << " total=" << total << std::endl;
#endif

    // If there is nothing in the cloud, nothing to remove.
    if (!std::isfinite(total) || total <= 0.0f) {
#if GENERICPM_DEBUG_ENABLED
        std::cout << "[DIAG] YEARDOY:" << FlexibleIO::getInstance()->getReal("CONTROL", "YEARDOY") << " removeSporesVal: nothing to remove (total non-finite or <=0)" << std::endl;
#endif
        values.clear();
        return;
    }

    // If trying to remove >= total, just zero all entries
    if (toBeRemoved >= total) {
#if GENERICPM_DEBUG_ENABLED
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
#if GENERICPM_DEBUG_ENABLED
        std::cout << "[DIAG] YEARDOY:" << FlexibleIO::getInstance()->getReal("CONTROL", "YEARDOY") << " removeSporesVal bucket before=" << before << " after=" << value << std::endl;
#endif
    }
}

void Cloud::removeSporesPct(float percent) {
    for (auto& value : values) {
#if GENERICPM_DEBUG_ENABLED
        std::cout << "[DIAG] YEARDOY:" << FlexibleIO::getInstance()->getReal("CONTROL", "YEARDOY") << " removeSporesPct bucket before=" << value;
#endif
        value *= percent;
#if GENERICPM_DEBUG_ENABLED
        std::cout << " after=" << value << std::endl;
#endif
    }
}

void Cloud::incrementSporesAge() {
    values.push_back(0.0f);
}