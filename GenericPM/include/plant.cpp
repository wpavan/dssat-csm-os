/**
 * @file plant.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "plant.h"
#include "manager.h"
#include "simulator.h"

#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

static double TE_getHealthyValue() {
    if (gEqContext) {
        if (gEqContext->plant) {
            return static_cast<double>(gEqContext->plant->getTotalValue() - gEqContext->plant->getDiseaseValue());
        } else {
            std::cerr << "Warning: TE_getHealthyValue called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    }
    return 0.0;
}

static double TE_getDiseaseValue() {
    if (gEqContext) {
        if (gEqContext->plant) {
            return static_cast<double>(gEqContext->plant->getDiseaseValue());
        } else {
            std::cerr << "Warning: TE_getDiseaseValue called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    }
    return 0.0;
}

static double TE_getTotalValue() {
    if (gEqContext) {
        if (gEqContext->plant) {
            return static_cast<double>(gEqContext->plant->getTotalValue());
        } else {
            std::cerr << "Warning: TE_getTotalValue called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    }
    return 0.0;
}

static double TE_getInvisibleDiseaseValue() {
    if (gEqContext) {
        if (gEqContext->plant) {
            return static_cast<double>(gEqContext->plant->getInvisibleValue());
        } else {
            std::cerr << "Warning: TE_getInvisibleDiseaseValue called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    }
    return 0.0;
}

static double TE_getVisibleDiseaseValue() {
    if (gEqContext) {
        if (gEqContext->plant) {
            return static_cast<double>(gEqContext->plant->getVisibleValue());
        } else {
            std::cerr << "Warning: TE_getVisibleDiseaseValue called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    }
    return 0.0;
}

static double TE_getThisDiseaseInvisibleValue() {
    if (gEqContext) {
        if (gEqContext->plant && gEqContext->disease) {
            return static_cast<double>(gEqContext->plant->getInvisibleValue(gEqContext->disease));
        } else {
            std::cerr << "Warning: TE_getThisDiseaseInvisibleValue called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    }
    return 0.0;
}

static double TE_getThisDiseaseTotalValue() {
    if (gEqContext) {
        if (gEqContext->plant && gEqContext->disease) {
            return static_cast<double>(gEqContext->plant->getTotalValue(gEqContext->disease));
        } else {
            std::cerr << "Warning: TE_getThisDiseaseTotalValue called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    }
    return 0.0;
}

namespace {
    struct FunctionRegistrar {
        FunctionRegistrar() {
            getCustomFunctions().register_context_function({"PLANT_VALUE", TE_getTotalValue});
            getCustomFunctions().register_context_function({"PLANT_TOTAL_VALUE", TE_getTotalValue});
            getCustomFunctions().register_context_function({"PLANT_DISEASE_VALUE", TE_getDiseaseValue});
            getCustomFunctions().register_context_function({"PLANT_HEALTHY_VALUE", TE_getHealthyValue});
            getCustomFunctions().register_context_function({"PLANT_INV_DIS_VALUE", TE_getInvisibleDiseaseValue});
            getCustomFunctions().register_context_function({"PLANT_VIS_DIS_VALUE", TE_getVisibleDiseaseValue});
            
            getCustomFunctions().register_context_function({"PLANT_THIS_DIS_VALUE", TE_getThisDiseaseTotalValue});
            getCustomFunctions().register_context_function({"PLANT_THIS_DIS_INV_VALUE", TE_getThisDiseaseInvisibleValue});

            // NOTE: Add in this dis vis value
        }
    };

    // Static instance to trigger the registration at program startup
    static FunctionRegistrar registrar;
}

int Plant::qtd = 0;
int Plant::firstOutputCall = 0;

Plant* Plant::instance = nullptr;

Plant::Plant() {
    std::vector<CouplingPointID> cps = Manager::getInstance()->getCouplingPointIDs();

    for (auto& cp : cps) {
        organSets.emplace_back(cp);
    }

    // NOTE: Move forward assuming that every disease creates a CloudF and "family" affiliation will be determined at function call time.
    for (auto& sim : Manager::getInstance()->getSimulators()) {
        if (sim->getCloudF() == nullptr) {
            throw std::runtime_error("Simulator has nullptr CloudF. Ensure that CloudF is created in Simulator constructor.");
        } else {
            std::cout << "=====================================\nSimulator has valid CloudF for family: " << sim->getDisease()->getFamily() << "\n====================================="<< std::endl;
        }
        cloudsP.emplace_back(std::make_shared<CloudP>(sim->getDisease(), sim->getCloudF()));
    }
}

void Plant::rate() {
    // NEW CODE FOR GDM2:
    // Set the current plant for context
    gEqContext->plant = this;

    int newOrgan = 0;

    // Create new organs if needed
    for (auto& set : organSets) {
        CropInterface *ci = Manager::getCropInterface(set.CP);
        Expression ORGAN_AGE = ci->getORGAN_AGE();

        // get all diseases for this organCP
        // get all families for those diseases
        // get all clouds for those families and add to this organ's cloudsO
        Manager *manager = Manager::getInstance();
        std::vector<std::unique_ptr<Simulator>>& sims = manager->getSimulators();
        std::vector<std::shared_ptr<CloudP>> relevantCloudsP;

        for (auto& sim : sims) {
            Disease* disease = sim->getDisease();
            if (disease->getOrganCP() == set.CP) {
                for (auto& cloudP : cloudsP) {
                    if (cloudP->getDisease() == disease && (std::find(relevantCloudsP.begin(), relevantCloudsP.end(), cloudP) == relevantCloudsP.end())) {
                        relevantCloudsP.push_back(cloudP);
                    }
                }
            }
        }

        // If the user has indicated that new organs can be created, we
        // follow algorithm 1. Otherwise we use algorithm 2.

        // Algorithm 1:
        // ORGAN_MODE: COHORT
        // Check if there is a new organ and if so, how many need to be created
        newOrgan = ci->hasNewOrgan();

        // If the number of new organs is greater than 0, create an organ with the
        // corresponding data and index in the crop interface.
        if (newOrgan > 0) {
            set.organs.emplace_back(set.CP, relevantCloudsP, newOrgan, ci->getOrganArea(newOrgan), ORGAN_AGE);
        }

        // Algorithm 2:
        // ORGAN_MODE: SINGULAR
        // If there is growth queued (only in SINGULAR mode), add it to the organ.
        if (set.growthQueue > 0) {
            // Create the first organ if there are none yet.
            if (set.organs.size() == 0) {
                set.organs.emplace_back(set.CP, relevantCloudsP, 1, set.growthQueue, ORGAN_AGE);
            } 
            // Otherwise, grow the existing organ(s).
            else {
                set.organs.back().grow(set.growthQueue);
            }
            // Only count growth once.
            set.clearGrowthQueue();
        }
    
        if (set.senescenceQueue > 0) {
            set.doSenescence();
        }
    }

    // Calculate total area by each set of organs. 
    for (auto& set : organSets) {
        set.totalValue = 0;
        set.healthyValue = 0;
        for (auto& organ : set.organs) {
            // Debug statement for checking organ values
            // std::cout << "Organ " << organ.getOrganNumber() << " Total Value: " << organ.getTotalValue() << " Healthy Value: " << organ.getHealthyValue() << std::endl;
            set.totalValue += organ.getTotalValue();
            set.healthyValue += organ.getHealthyValue();
        }
    }

    // Assign organ area proportions based on new total area.
    // NOTE: Instead of direct assignment, should we make the getter 
    //       function return the evaluated area? To find out, we should
    //       do some profiling of the code in the two configurations.
    for (auto& set : organSets) {
        for (auto& organ : set.organs) {
            if(organ.getHealthyValue() > 0) {
                if(set.totalValue > 0) {
                    organ.setProportionFromTotalValue(organ.getTotalValue()/set.totalValue);
                } else {
                    organ.setProportionFromTotalValue(0);
                }
                organ.rate();
            }
        }
    }
    // Release the plant context
    gEqContext->plant = nullptr;
}

void Plant::integration() {
    // Value partitions
    visibleValue = invisibleValue = 0;
    healthyValue = 0;

    // Lesion trackers
    totalLesions = visibleLesions = 0;

    // Loop through all of the organs
    for (auto& set : organSets) {
        for (auto& organ : set.organs) {
            // If the organ has ANY living tissue
            if (organ.getHealthyValue() > 0) {
                // Organ lesions are calculated based on the total 
                // lesions from that compartment (organ/plant/field). 
                // Then, these lesions (for plant and field) are 
                // divided up based on the proportion of healthy value 
                // represented by that organ. This is based on the 
                // previous healthy value (make sure not 0)
                organ.setOrganSetHealthyValue(set.healthyValue);

                // Run the integration step
                organ.integration();

                // Then update the areas of the plant to match the state of the organs.
                // Because all of the component areas are set to 0 beforehand, this 
                // definitely reflects the current state of all the organs.
                visibleValue += organ.getVisibleValue();
                invisibleValue += organ.getInvisibleValue();
                healthyValue += organ.getHealthyValue();
                visibleLesions += organ.getVisibleLesions();
                totalLesions += organ.getTotalLesions();

                // std::cout << "Total lesions today so far: " << totalLesions << std::endl;
            }
        }
    }
}

void Plant::output() {
    // Iterate through the organs and output
    for (auto& set : organSets) {
        //set.output();
        for (auto& organ : set.organs) {
            organ.output();
        }
    }
    // Iterate through the plant clouds and output
    for (auto& cloudP : cloudsP) {
        cloudP->output();
    }
}
