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
    std::vector<Simulator*> simulators = Manager::getInstance()->getSimulators(); 
    std::vector<CouplingPointID> cps = Manager::getInstance()->getCouplingPointIDs();

    for (auto& cp : cps) {
        organSets.emplace_back(cp);
    }
    // NOTE: This currently makes as many clouds as there are 
    //       simulators. This is a good start, but we need to make it 
    //       such that it only creates one per unique disease type (WB
    //       preseason and in-season should be combined).
    for (auto& simulatorPtr : simulators) {
        cloudsP.emplace_back(simulatorPtr->getDisease(), simulatorPtr->getInitialCondition()->getCloud());
    }
}

void Plant::rate() {
    // NEW CODE FOR GDM2:
    // Calculate total area by each set of organs. 
    for (auto& set : organSets) {
        set.totalValue = 0;
        for (auto& organ : set.organs) {
            set.totalValue += organ.getTotalArea();
        }
    }

    // Assign organ area proportions based on new total area.
    // NOTE: Instead of direct assignment, should we make the getter 
    //       function return the evaluated area? To find out, we should
    //       do some profiling of the code in the two configurations.
    for (auto& set : organSets) {
        for (auto& organ : set.organs) {
            if(organ.getSenescenceArea() < organ.getTotalArea()) {
                if(set.totalValue > 0) {
                    organ.setProportionFromTotalArea(organ.getTotalArea()/set.totalValue);
                } else {
                    organ.setProportionFromTotalArea(0);
                }
                organ.rate();
            }
        }
    }
}

void Plant::integration() {
    totalArea = diseaseArea = latentDiseaseArea = infectionDiseaseArea = necroticDiseaseArea = visibleDiseaseArea = invisibleDiseaseArea = senescenceArea = 0;
    totalLesions = visibleLesions = 0;
    int newOrgan = 0;

    // Loop through all of the organs
    for (auto& set : organSets) {
        for (auto& organ : set.organs) {
            // If the organ has ANY living tissue
            if (organ.getSenescenceArea() < organ.getTotalArea()) {
                // Run the integration step
                organ.integration();

                // Then update the areas of the plant to match the state of the organs.
                // Because all of the component areas are set to 0 beforehand, this 
                // definitely reflects the current state of all the organs.
                diseaseArea += organ.getDiseaseArea();
                latentDiseaseArea += organ.getLatentDiseaseArea();
                infectionDiseaseArea += organ.getInfectionDiseaseArea();
                necroticDiseaseArea += organ.getNecroticDiseaseArea();
                visibleDiseaseArea += organ.getVisibleDiseaseArea();
                invisibleDiseaseArea += organ.getInvisibleDiseaseArea();
                visibleLesions += organ.getVisibleLesions();
                totalLesions += organ.getTotalLesions();
            }
            // Regardless of living status, the senescence and total areas should be recorded.
            totalArea += organ.getTotalArea();
            senescenceArea += organ.getSenescenceArea();
        }
    }

    // Run the integration step for each of the plant clouds (1 per disease)
    for (auto& cloudP : cloudsP) {
        cloudP.integration();
    }

    for (auto& set : organSets) {
        // Check if there is a new organ and if so, how many need to be created
        CropInterface *ci = Manager::getCropInterface(set.CP);

        newOrgan = ci->hasNewOrgan();
        printf("New organ count for coupling point %s: %d\n", cpIDToStr(set.CP).c_str(), newOrgan);

        // If the number of new organs is greater than 0, create an organ with the
        // corresponding data and index in the crop interface.
        if (newOrgan > 0) {
            set.organs.emplace_back(set.CP, cloudsP, newOrgan, Manager::getCropInterface(set.CP)->getOrganArea(newOrgan));
        }
    }

    // These could be moved into the convert block below instead of adding to memory
    //cloudPValue = cloud->getValue();
    //cloudFvalue = cloud->getCloudF()->getValue();

    /*
    std::ostringstream convert;
    //Plant, YearDoy, TotalArea, Senesced, Diseased, VisibleArea, InvisibleArea, TotalLesions, CloudO, CloudP, CloudF
    convert << ID << "," << Basic::getWeather()->getYearDoy() << "," << totalArea << "," << senescenceArea << "," << diseaseArea << "," 
            << visibleDiseaseArea << "," << invisibleDiseaseArea << "," << totalLesions << "," 
            << latentDiseaseArea << "," << infectionDiseaseArea << "," << necroticDiseaseArea << ","
            << Utilities::formatfloat(cloudOValue) << "," << Utilities::formatfloat(cloudPValue) << "," 
            << Utilities::formatfloat(cloudFvalue);
    Basic::output.push_back(convert.str());
    */
}

void Plant::output() {
    std::ostringstream convert;
    Basic::getOutput("Cpp_Plant.txt", this->firstOutputCall);
    this->firstOutputCall++;

    // Speedup the model removing outputs
    //std::cout << "\nPlant " << getID() << ":\n";
    //for(unsigned int i=0; i<Basic::output.size(); i++)
    //{
    //    std::cout << Basic::output[i] << std::endl;
    //}

    // Iterate through the organs and output
    for (auto& set : organSets) {
        for (auto& organ : set.organs) {
            organ.output();
        }
    }
    // Iterate through the plant clouds and output
    for (auto& cloudP : cloudsP) {
        cloudP.output();
    }
}
