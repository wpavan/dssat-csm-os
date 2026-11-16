/**
 * @file organ.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * @author Virginia Covert (virginiacovert454@gmail.com)
 * 
 * @copyright Copyright (c) 2017–2026, University of Florida
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "organ.h"
#include "simulator.h"
#include "cloudo.h"
#include "cloudp.h"
#include "cloudf.h"
#include "lesioncohort.h"
#include "utilities.h"
#include "basic.h"
#include "manager.h"
#include "equation_context.h"
#include "numericstringcache.h"

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>

static double TE_getHealthyValue(void) {
    if (gEqContext) {
        if (gEqContext->organ) {
            return static_cast<double>(gEqContext->organ->getHealthyValue());
        } else {
            std::vector<std::string> messages;
            messages.push_back("Warning: TE_getHealthyValue called outside of the appropriate context.");

            throwWarning(messages.size(), messages);
            return 0.0;
        }
    }
    return 0.0;
}

static double TE_getDiseaseValue(void) {
    if (gEqContext) {
        if (gEqContext->organ) {
            return static_cast<double>(gEqContext->organ->getDiseaseValue());
        } else {
            std::vector<std::string> messages;
            messages.push_back("Warning: TE_getDiseaseValue called outside of the appropriate context.");

            throwWarning(messages.size(), messages);
            return 0.0;
        }
    }
    return 0.0;
}

static double TE_getInvisibleDiseaseValue(void) {
    if (gEqContext) {
        if (gEqContext->organ) {
            return static_cast<double>(gEqContext->organ->getInvisibleValue());
        } else {
            std::vector<std::string> messages;
            messages.push_back("Warning: TE_getInvisibleDiseaseValue called outside of the appropriate context.");

            throwWarning(messages.size(), messages);
            return 0.0;
        }
    }
    return 0.0;
}

static double TE_getVisibleDiseaseValue(void) {
    if (gEqContext) {
        if (gEqContext->organ) {
            return static_cast<double>(gEqContext->organ->getVisibleValue());
        } else {
            std::vector<std::string> messages;
            messages.push_back("Warning: TE_getVisibleDiseaseValue called outside of the appropriate context.");

            throwWarning(messages.size(), messages);
            return 0.0;
        }
    }
    return 0.0;
}

static double TE_getTotalValue(void) {
    if (gEqContext) {
        if (gEqContext->organ) {
            return static_cast<double>(gEqContext->organ->getTotalValue());
        } else {
            std::vector<std::string> messages;
            messages.push_back("Warning: TE_getTotalValue called outside of the appropriate context.");

            throwWarning(messages.size(), messages);
            return 0.0;
        }
    }
    return 0.0;
}

static double TE_getAge(void) {
    if (gEqContext) {
        if (gEqContext->organ) {
            return static_cast<double>(gEqContext->organ->getPhysiologicalLife());
        } else {
            std::vector<std::string> messages;
            messages.push_back("Warning: TE_getAge called outside of the appropriate context.");

            throwWarning(messages.size(), messages);
            return 0.0;
        }
    }
    return 0.0;
}
// TE function to get the inoculum around an organ (considers field, plant, and organ clouds).
// Takes a disease context either from a disease directly or a cloud and returns inoculum for all diseases that share a family. 
static double TE_getLocalInoculum(void) {
    float totalInoc = 0.0f;

    if (gEqContext && gEqContext->organ) {
        std::string family;
        if (gEqContext->disease) {
            family = gEqContext->disease->getFamily();
        } else if (gEqContext->cloud) {
            family = gEqContext->cloud->getDisease()->getFamily();
        } else {
            std::cerr << "Warning: TE_getLocalInoculum called without disease or cloud context. Unable to determine family for inoculum calculation." << std::endl;
            return 0.0;
        }

        for (const auto& simulator : Manager::getInstance()->getSimulators()) {
            if (simulator->getDisease()->getFamily() != family) {
                continue; // Skip diseases that are not in the same family
            }
            std::shared_ptr<CloudO> cloudO = gEqContext->organ->getCloudO(simulator->getDisease());
            if (cloudO) {
                totalInoc += cloudO->getValue();
                if (cloudO->getCloudP()) {
                    totalInoc += cloudO->getCloudP()->getValue();
                    if (cloudO->getCloudP()->getCloudF()) {
                        totalInoc += cloudO->getCloudP()->getCloudF()->getValue();
                    }
                }
            }
        }
        return static_cast<double>(totalInoc);
    } else {
        std::vector<std::string> messages;
            messages.push_back("Warning: TE_getLocalInoculum called outside of the appropriate context.");

            throwWarning(messages.size(), messages);
    }
    return 0.0;
}

// NOTE: we need to replace family text with a number
static double TE_getLocalInoculumByFamily(double family) {
    // Get the double to string decoder for the family name
    FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();
    std::string familyStr = converter->decode(family);

    // Determine if the name is a valid family
    if (!Manager::getInstance()->familiesHas(familyStr)) {
        // Throw an error if the family name is not valid
        std::cerr << "Error: Local organ inoculum by family received invalid family name: " << familyStr << std::endl;
        throw std::invalid_argument("Invalid family name for local inoculum retrieval");
    } else {
        std::cout << "Retrieving local inoculum for family: " << familyStr << std::endl;
        // Evaluate the total inoculum for the family across all clouds affecting the organ
        float totalInoc = 0.0f;
        if (gEqContext && gEqContext->organ) {
            for (auto& cloudo : gEqContext->organ->getCloudsO()) {
                if (familyStr == cloudo->getCloudP()->getCloudF()->getFamily()) {
                    totalInoc += cloudo->getValue();
                    totalInoc += cloudo->getCloudP()->getValue();
                    totalInoc += cloudo->getCloudP()->getCloudF()->getValue();
                    return static_cast<double>(totalInoc);
                }
            }
        } else if (gEqContext && !gEqContext->organ) {
            std::vector<std::string> messages;
            messages.push_back("Warning: TE_getLocalInoculumByFamily called outside of the appropriate context.");

            throwWarning(messages.size(), messages);
        }
        return 0.0;
    }
}

namespace {
    struct FunctionRegistrar {
        FunctionRegistrar() {
            getCustomFunctions().register_context_function({"ORGAN_VALUE", TE_getTotalValue});
            getCustomFunctions().register_context_function({"ORGAN_DISEASE_VALUE", TE_getDiseaseValue});
            getCustomFunctions().register_context_function({"ORGAN_VIS_DIS_VALUE", TE_getVisibleDiseaseValue});
            getCustomFunctions().register_context_function({"ORGAN_INV_DIS_VALUE", TE_getInvisibleDiseaseValue});
            getCustomFunctions().register_context_function({"ORGAN_HEALTHY_VALUE", TE_getHealthyValue});
            getCustomFunctions().register_context_function({"ORGAN_AGE", TE_getAge});
            getCustomFunctions().register_context_function({"ORGAN_LOCAL_INOC", TE_getLocalInoculum});
            getCustomFunctions().register_context_function({"ORGAN_LOCAL_FAMILY_INOC", TE_getLocalInoculumByFamily});
        }
    };

    // Static instance to trigger the registration at program startup
    static FunctionRegistrar registrar;
}

int Organ::firstOutputCall = 0;

void Organ::rate() {
    // Set the current organ for context
    gEqContext->organ = this;

    // Get the manager instance to access crop interfaces and coupling data
    Manager *manager = Manager::getInstance();

    // std::cout << "\nStarting rate for Organ " << organNumber << " (CP: " << cpIDToStr(organCP) << ") On day" << manager->getCurrentSimDate() << std::endl;

    // Get the flexible IO instance for new lesion storage updating.
    FlexibleIO *fio = FlexibleIO::getInstance();

    // Determine the increase in physiological age today
    // NOTE: Ensure that the organ age parameter is set
    //       once per organ value CP in the yaml file.
    try {
        dailyPhysiologicalLife = ORGAN_AGE.evaluate();
    } catch (const std::runtime_error& e) {
        std::cerr << "Error evaluating ORGAN_AGE expression for Organ CP: " << cpIDToStr(organCP) << std::endl << "Exception: " << e.what() << std::endl;
        dailyPhysiologicalLife = 0.0f; // Default to 0 favorability if evaluation fails
    }
    
    // Update the senescence area for the current day
    // NOTE: Is this routing of simulator -> cropinterface -> getSenescenceOrganArea needed? 
    //       We could maybe change the senescenceValue of the organ to be held in the organ object itself.
    // this->senescenceValue = cropinterface->getSenescenceOrganArea(organNumber);

    // Determine the number of new lesions on the organ today
    if(suceptible) {
        int i = 0;
        for (auto& cloudo : cloudsO) {
            gEqContext->cloud = cloudo; // Set the current cloud for context
            gEqContext->disease = cloudo->getDisease(); // Set current disease context

            // Record that new lesions should be created
            int newLesionsVal = 0;
            try {
                newLesionsVal = cloudo->getDisease()->getNEW_LES()->evaluate();
            } catch (const std::runtime_error& e) {
                std::cerr << "Error evaluating NEW_LES expression for DiseaseID: " << cloudo->getDisease()->getDiseaseID() << std::endl << "Exception: " << e.what() << std::endl;
            }
            i++;

            if (newLesionsVal > 0) {
                // Add the new lesions to the organ's tracking structure
                newLesions.addLesions(cloudo, newLesionsVal);

                // Push information about new daily lesions to FIO
                fio->setIntegerMemory(cloudo->getDisease()->getDiseaseID(), "DAILY_NEW_LESIONS", fio->getInteger(cloudo->getDisease()->getDiseaseID(), "DAILY_NEW_LESIONS") + newLesions.getTotalLesions());
            }
            
            gEqContext->cloud = nullptr; // Clear the current cloud from context
        }
    }
    // NOTE: This should be divided by which disease is creating the lesions
    
    // Recalculate the ratio due senescence and take the difference from previous ratio
    // ratioSenescence = (this->senescenceValue / this->getTotalValue()) - ratioSenescence;

    // Update the total organ area (current day) - (shouldn't do anything)
    // this->totalArea = cropinterface->getOrganArea(organNumber);
    // std::cout << "Organ " << organNumber << " Daily Healthy Value: " << dailyHealthyValue << std::endl;
    if (dailyHealthyValue > 0) {
        healthyValue += dailyHealthyValue;
        dailyHealthyValue = 0;
    }

    // If the organ is not alive, skip the rest of the calculations
    if (!isAlive()) {
        return;
    }

    // === Starting LC rate loop ===
    for (auto& lc : lesionCohorts) {
        // Lesion cohort rate call
        lc.rate();
    }

    // Safely nullify the organ pointer in the context before exiting/looping
    gEqContext->organ = nullptr;
}

void Organ::integration() {
    if (!isAlive()) {
        return;
    }
    gEqContext->organ = this;

    CloudO *cloudo;
    LesionCohort *lc;

    float cloudDensity = 0;
    float cloudOValue = 0, cloudPValue = 0, cloudFValue = 0;
    dailyTotalLesions = totalLesions;
    dailyVisibleLesions = visibleLesions;
    visibleLesions = 0;

    // Increase the organ's physiological age
    physiologicalLife += dailyPhysiologicalLife;

    // Create the new lesions
    if (suceptible) {
        for (auto& pair : newLesions.lesions) {
            // Create a new lesion cohort for each disease that has created lesions on this organ
            lesionCohorts.emplace_back(pair.second, pair.first);

            // Update the total lesions count for this organ
            totalLesions += pair.second;
        }

        newLesions.lesions.clear(); // Clear the new lesions after creating cohorts
    }

    // NOTE: we still have to implement removal from infection.

    // Run lesion cohort integrations
    for (auto& lc : lesionCohorts) {
        lc.integration();
        visibleLesions += lc.getVisibleLesions();
        
        // Update the lesion cohort's "knowledge" of organ status
        lc.setOrganDiseaseValue(this->getDiseaseValue());
        lc.setOrganHealthyValue(this->getHealthyValue());
    }

    // Read in the lesion cohort values after running their integrations
    // NOTE: reconsider these values here vs global or fio context.
    readDiseaseValues();

    gEqContext->organ = nullptr;
}

float Organ::cloudAmount() {
    float cloudOValue=0;
    for (auto& cloudO : cloudsO) {
        cloudOValue += cloudO->getValue();
    }
    return cloudOValue;
}

void Organ::output() {
    std::ostringstream convert;

    Basic::getOutput("Cpp_Organ.txt",this->firstOutputCall);
    this->firstOutputCall++;
    // Speedup the model removing outputs
    #ifdef OUTPUT
    for(unsigned int i=0; i<Basic::output.size(); i++) {
        std::cout << Basic::output[i] << std::endl;
    }
    #endif // OUTPUT

    for (auto& cloudo : cloudsO) {
        cloudo->output();
    }

    for (auto& lc : lesionCohorts) {
        lc.output();
    }
}

void printCloudValues(int CloudFValue, int CloudPValue, int CloudOValue) {
    printf("Cloud Field: %d\nCloud Plant: %d\nCloud Organ: %d\n", CloudFValue, CloudPValue, CloudOValue);
}