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

int Plant::qtd = 0;
int Plant::firstOutputCall = 0;

Plant* Plant::instance = nullptr;

Plant::Plant() {
    std::vector<std::unique_ptr<Simulator>>& simulators = Manager::getInstance()->getSimulators(); 
    std::vector<CouplingPointID> cps = Manager::getInstance()->getCouplingPointIDs();

    for (auto& cp : cps) {
        organSets.emplace_back(cp);
    }
    
    // NOTE: This currently makes as many clouds as there are 
    //       simulators. This is a good start, but we need to make it 
    //       such that it only creates one per unique disease type (WB
    //       preseason and in-season should be combined).
    for (auto& simulatorPtr : simulators) {
        cloudsP.emplace_back(simulatorPtr.get()->getDisease(), simulatorPtr.get()->getInitialCondition()->getCloud());
    }
}

void Plant::rate() {
    // NEW CODE FOR GDM2:
    int newOrgan = 0;

    // Create new organs if needed
    for (auto& set : organSets) {
        // If the user has indicated that new organs can be created, we
        // follow algorithm 1. Otherwise we use algorithm 2.

        // Algorithm 1:
        //   Check if there is a new organ and if so, how many need to be created
        CropInterface *ci = Manager::getCropInterface(set.CP);
        
        newOrgan = ci->hasNewOrgan();

        // If the number of new organs is greater than 0, create an organ with the
        // corresponding data and index in the crop interface.
        if (newOrgan > 0) {
            set.organs.emplace_back(set.CP, cloudsP, newOrgan, Manager::getCropInterface(set.CP)->getOrganArea(newOrgan));
        }

        // Algorithm 2:
        // Debug statement for reporting growth queue
        // std::cout << "Growth queue for this OrganSet: " << set.growthQueue << std::endl;
        if (set.growthQueue > 0) {
            if (set.organs.size() == 0) {
                set.organs.emplace_back(set.CP, cloudsP, 1, set.growthQueue);
            } else {
                set.organs.back().grow(set.growthQueue);
            }
            set.clearGrowthQueue();
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

                // Printing for debug
                // std::cout << "YEARDOY: " << getWeather()->getYearDoy() << " invis | vis | healthy: " <<
                // organ.getInvisibleValue() << " | " <<
                // organ.getVisibleValue() << " | " <<
                // organ.getHealthyValue() << std::endl;

                // std::cout << "Total lesions today so far: " << totalLesions << std::endl;
            }
        }
    }
    //std::cout << "---------------" << std::endl;

    // THIS IS WHERE NEW ORGAN CREATION +++WAS+++
    
    // INSERT ABOVE TO REVERT

    // These could be moved into the convert block below instead of adding to memory
    //cloudPValue = cloud->getValue();
    //cloudFvalue = cloud->getCloudF()->getValue();

    #ifdef OUTPUT_PLANT
    std::ostringstream convert;
    //YearDoy, TotalValue, HealthyValue, InvisibleValue, VisibleValue, TotalLesions
    convert << Basic::getWeather()->getYearDoy() << "," << getTotalValue() << "," << healthyValue << ","
            << invisibleValue << "," << visibleValue << "," << totalLesions;
    Basic::output.push_back(convert.str());
    #endif // OUTPUT_PLANT
}

void Plant::output() {
    #ifdef OUTPUT_PLANT
    std::ostringstream convert;
    Basic::getOutput("Cpp_Plant.txt", this->firstOutputCall);
    this->firstOutputCall++;
    #endif // OUTPUT_PLANT

    // Speedup the model removing outputs
    #ifdef OUTPUT_PLANT
    std::cout << "\nPlant " << getID() << ":\n";
    for(unsigned int i=0; i<Basic::output.size(); i++) {
       std::cout << Basic::output[i] << std::endl;
    }
    #endif // OUTPUT_PLANT

    // Iterate through the organs and output
    for (auto& set : organSets) {
        //set.output();
        for (auto& organ : set.organs) {
            organ.output();
        }
    }
    // Iterate through the plant clouds and output
    for (auto& cloudP : cloudsP) {
        cloudP.output();
    }
}
