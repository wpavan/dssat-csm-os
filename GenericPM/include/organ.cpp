/**
 * @file organ.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
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

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>

int Organ::firstOutputCall = 0;

void Organ::rate() {
    Manager *manager = Manager::getInstance();
    // Get the crop interface that refers to this type of organs specifically
    // CropInterface *cropinterface = manager->getCropInterface(organCP);

    // Calculate the ratio due senescence based on previous day
    // float actualDisease = 0; //, ratioSenescence = this->senescenceValue / this->getTotalValue();
    // float actualInvisibleValue = 0, actualVisibleValue = 0;

    // Update the senescence area for the current day
    // NOTE: Is this routing of simulator -> cropinterface -> getSenescenceOrganArea needed? 
    //       We could maybe change the senescenceValue of the organ to be held in the organ object itself.
    //this->senescenceValue = cropinterface->getSenescenceOrganArea(organNumber);

    // Recalculate the ratio due senescence and take the difference from previous ratio
    //ratioSenescence = (this->senescenceValue / this->getTotalValue()) - ratioSenescence;

    // Update the total organ area (current day) - (shouldn't do anything)
    // this->totalArea = cropinterface->getOrganArea(organNumber);
    std::cout << "Organ " << organNumber << " Daily Healthy Value: " << dailyHealthyValue << std::endl;
    if (dailyHealthyValue > 0) {
        healthyValue += dailyHealthyValue;
        dailyHealthyValue = 0;
    }

    // If the organ was not previously susceptible and now has area, set it to susceptible
    if (!suceptible && this->getTotalValue() > 0) {
        suceptible = true;
    }

    // If the organ is not alive, skip the rest of the calculations
    if (!isAlive()) {
        return;
    }

    // std::cout << "=== Starting LC rate loop ===" << std::endl;
    for (auto& lc : lesionCohorts) {
        lc.setOrganHealthyValue(getHealthyValue());
        lc.setOrganDiseaseValue(getDiseaseValue());
        // std::cout << "organ healthy & disease: " << getHealthyValue() << " & " << getDiseaseValue() << std::endl;
        lc.rate();
        // After rate, lc has calculated the daily (in)visible value growth
        // spores are also calculated if the method calls for it
    }
}

void Organ::integration() {
    if (!isAlive()) {
        return;
    }
    
    CloudO *cloudo;
    LesionCohort *lc;

    float cloudDensity = 0;
    float cloudOValue = 0, cloudPValue = 0, cloudFValue = 0;
    dailyTotalLesions = totalLesions;
    dailyVisibleLesions = visibleLesions;
    visibleLesions = 0;
    newLesions = 0;

    if (suceptible) {
        for (auto& cloudo : cloudsO) {
            physiologicalLife += Utilities::trapezoidalFunction(Basic::getWeather()->getTMean(), 
                                                                cloudo.getDisease()->getCardinalTempPhysiologicalLife());

            cloudOValue = cloudAmount();
            cloudPValue = cloudo.getCloudP()->getValue();
            cloudFValue = cloudo.getCloudP()->getCloudF()->getValue();

            // The number of lesions that will be created on any given 
            // organ is proportional to that organ's exposed healthy 
            // surface area. Organs should get lesions proportional to
            // the fraction of healthy area represented by that organ.
            // std::cout << "\n--------------- Lesion Math ---------------------" << std::endl;
            // std::cout << "YEARDOY: " << std::to_string(getWeather()->getYearDoy()) << " Lesion Queue: " << lesionQueue << std::endl;

            float lesionQueueOrgan = cloudo.getDisease()->newLesions(cloudOValue, healthyValue);
            float lesionQueuePlant = cloudo.getDisease()->newLesions(cloudPValue, healthyValue) * (healthyValue / getOrganSetHealthyValue()); 
            float lesionQueueField = cloudo.getDisease()->newLesions(cloudFValue, healthyValue) * (healthyValue / getOrganSetHealthyValue()); 

            lesionQueue += lesionQueueOrgan + lesionQueuePlant + lesionQueueField;
            // std::cout << "--------     New Lesion Queue: " << lesionQueue << std::endl;

            int totalNewLesions = lesionQueue;
            // std::cout << "--------       Lesions Formed: " << totalNewLesions << std::endl;

            lesionQueue -= totalNewLesions;
            // std::cout << "--------     End Lesion Queue: " << lesionQueue << std::endl;
            // std::cout << "-----------------------------------------\n" << std::endl;

            // Using old integer-based approach
            // newLesionsFromOrgan = cloudo.getDisease()->newLesions(cloudOValue,healthAreaProportion);
            // newLesionsFromPlant = cloudo.getDisease()->newLesions(cloudPValue,healthAreaProportion) * (healthyValue / getOrganSetHealthyValue()); 
            // newLesionsFromField = cloudo.getDisease()->newLesions(cloudFValue,healthAreaProportion) * (healthyValue / getOrganSetHealthyValue()); 
                        
            // int totalNewLesions = newLesionsFromField + newLesionsFromPlant + newLesionsFromOrgan;

            // Uncomment the following line to debug this step
            // std::cout << "YEARDOY: " << std::to_string(getWeather()->getYearDoy()) << " New Lesions: " << totalNewLesions << std::endl;
            // std::cout << "-------- ------- Cloud Value: " << cloudOValue + cloudPValue + cloudFValue << std::endl;
            // std::cout << "-------- ------- Physio Life: " << physiologicalLife << std::endl;

            // NOTE: This hardcoded physiological life should be 
            //       replaced by a dynamic threshold in the .yaml file.
            //       We could also modulate the number of lesions 
            //       created based on some dynamic age factor.
            if (totalNewLesions > 0 && physiologicalLife >= 0) {
                // std::cout << "Organ: " << organNumber << "\tnew lesions: " << newLesions << std::endl;
                
                lesionCohorts.emplace_back(totalNewLesions, &cloudo);
                totalLesions += totalNewLesions;
 
                // std::cout << "YEARDOY: " << getWeather()->getYearDoy() << " Organ (" << doc << ") Lesions: " << totalLesions << std::endl;

                // Add Spores that will be removed because were used to infect the tissue
                cloudo.addSporesToBeRemoved(newLesionsFromOrgan);
                cloudo.getCloudP()->addSporesToBeRemoved(newLesionsFromPlant);
                cloudo.getCloudP()->getCloudF()->addSporesToBeRemoved(newLesionsFromField);
            } 
            newLesionsFromOrgan = newLesionsFromPlant = newLesionsFromField = 0;            
        }
    }

    // Run lesion cohort integrations
    for (auto& lc : lesionCohorts) {
        lc.integration();
        visibleLesions += lc.getVisibleLesions();
    }

    // Read in the lesion cohort values after running their integrations
    readDiseaseValues();
    
    // dailyDiseaseArea = fmax(0,diseaseArea - dailyDiseaseArea);
    // dailyVisibleDiseaseArea = fmax(0,visibleDiseaseArea - dailyVisibleDiseaseArea);
    // dailySenescenceArea = fmax(0,senescenceArea - dailySenescenceArea);
    // dailyLatentDiseaseArea = fmax(0,latentDiseaseArea - dailyLatentDiseaseArea);
    // dailyInfectionDiseaseArea = fmax(0,infectionDiseaseArea - dailyInfectionDiseaseArea);
    // dailyNecroticDiseaseArea = fmax(0,necroticDiseaseArea - dailyNecroticDiseaseArea);
    // dailyTotalLesions = fmax(0,totalLesions - dailyTotalLesions);
    // dailyVisibleLesions = fmax(0,visibleLesions - dailyVisibleLesions);
    //    printf(" Daily: DiseaseArea (%f),"
    //            "VisibleDiseaseArea (%f),"
    //            "SenescenceArea (%f),"
    //            "LatentDiseaseArea (%f),"
    //            "InfectionDiseaseArea (%f),"
    //            "NecroticDiseaseArea (%f)\n "
    //            "TotalLesions (%i),"
    //            "VisibleLesions (%i)\n", dailyDiseaseArea, dailyVisibleDiseaseArea,
    //            dailySenescenceArea,dailyLatentDiseaseArea,dailyInfectionDiseaseArea,
    //            dailyNecroticDiseaseArea,dailyTotalLesions,dailyVisibleLesions);

    /**
     * Add the following values to the output queue:
     * OrganID, YearDoy, TotalArea, SenescedArea,
     * DiseasedArea, VisibleDiseasedArea, InvisibleDiseasedArea,
     * LesionDensity, Age, DailyNewLesions, TotalLesions, 
     * CloudO, CloudP, CloudF, HealthAreaProportion, ProportionOfPlantTotalArea,
     * WetnessDuration, NewLesionsFromOrgan, NewLesionsFromPlant, NewLesionsFromField
     */
    // std::ostringstream convert;
    // convert << organNumber << "," 
    //         << Basic::getWeather()->getYearDoy() << "," << totalArea << "," << senescenceArea << "," 
    //         << Utilities::formatfloat(diseaseArea, 4) << ","
    //         << Utilities::formatfloat(visibleDiseaseArea, 4) << "," 
    //         << Utilities::formatfloat(invisibleDiseaseArea, 4) << ","
    //         << Utilities::formatfloat(totalLesions / totalArea, 4) << ","
    //         << Utilities::formatfloat(physiologicalLife, 4) << "," << dailyTotalLesions << "," 
    //         << totalLesions << "," 
    //         << Utilities::formatfloat(cloudOValue) << "," << Utilities::formatfloat(cloudPValue) << ","
    //         << Utilities::formatfloat(cloudFValue) << "," << healthAreaProportion << "," 
    //         << getProportionFromTotalArea() << "," << Basic::getWeather()->getWetDur() << ","
    //         << newLesionsFromOrgan << "," << newLesionsFromPlant << "," << newLesionsFromField;
    // Basic::output.push_back(convert.str());
}

float Organ::cloudAmount() {
    float cloudOValue=0;
    for (auto& cloudO : cloudsO) {
        cloudOValue += cloudO.getValue();
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
        cloudo.output();
    }

    for (auto& lc : lesionCohorts) {
        lc.output();
    }
}

void printCloudValues(int CloudFValue, int CloudPValue, int CloudOValue) {
    printf("Cloud Field: %d\nCloud Plant: %d\nCloud Organ: %d\n", CloudFValue, CloudPValue, CloudOValue);
}