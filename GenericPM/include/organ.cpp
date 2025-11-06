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
    CropInterface *cropinterface = manager->getCropInterface(organCP);

    // Calculate the ratio due senescence based on previous day
    float actualDisease = 0, ratioSenescence = this->senescenceArea / this->totalArea;

    // Update the senescence area for the current day
    // NOTE: Is this routing of simulator -> cropinterface -> getSenescenceOrganArea needed? 
    //       We could maybe change the senescenceArea of the organ to be held in the organ object itself.
    this->senescenceArea = cropinterface->getSenescenceOrganArea(organNumber);

    // Recalculate the ratio due senescence and take the difference from previous ratio
    ratioSenescence = (this->senescenceArea / this->totalArea) - ratioSenescence;

    // Update the total organ area (current day)
    this->totalArea = cropinterface->getOrganArea(organNumber);
    
    // If the organ was not previously susceptible and now has area, set it to susceptible
    if (!suceptible && this->totalArea > 0) {
        suceptible = true;
    }

    // If the organ is not alive, skip the rest of the calculations
    if (!isAlive()) {
        return;
    }

    // Use the lesion cohorts to update the diseased area based on the senescence
    for (auto& lc : lesionCohorts) {
        // Must affect the disease area related with senescent area
        if (ratioSenescence > 0) {
            lc.setVisibleArea(lc.getVisibleArea() * (1-ratioSenescence));
            lc.setInvisibleArea(lc.getInvisibleArea() * (1-ratioSenescence));
            lc.setTotalArea(lc.getVisibleArea()+lc.getInvisibleArea());
        }
        actualDisease += lc.getTotalArea();
    }

    // Updating the disease amount on organ
    setDiseaseArea(actualDisease);

    // Calculating the health area proportion
    healthAreaProportion = Utilities::getHealthAreaProportion(getDiseaseArea(), 
                                                              getTotalArea(), 
                                                              getSenescenceArea());

    for (auto& lc : lesionCohorts) {
        lc.setOrganHealthAreaProportion(healthAreaProportion);
        lc.rate();
    }
}

void Organ::integration() {
    if (!isAlive()) {
        return;
    }
    
    CloudO *cloudo;
    LesionCohort *lc;

    float cloudDensity = 0;
    float cloudOValue = 0, cloudPValue = 0, cloudFvalue = 0;
    dailyDiseaseArea = diseaseArea;
    dailyVisibleDiseaseArea = visibleDiseaseArea;
    dailySenescenceArea = senescenceArea;
    dailyLatentDiseaseArea = latentDiseaseArea;
    dailyInfectionDiseaseArea = infectionDiseaseArea;
    dailyNecroticDiseaseArea = necroticDiseaseArea;
    dailyTotalLesions = totalLesions;
    dailyVisibleLesions = visibleLesions;
    diseaseArea = 0;
    visibleLesions = 0;
    visibleDiseaseArea = invisibleDiseaseArea = 0;
    latentDiseaseArea = infectionDiseaseArea = necroticDiseaseArea = 0; 
    newLesions = 0;

    if (suceptible) {
        for (auto& cloudo : cloudsO) {
            physiologicalLife += Utilities::trapezoidalFunction(Basic::getWeather()->getTMean(), 
                                                                cloudo.getDisease()->getCardinalTempPhysiologicalLife());

            cloudOValue = cloudAmount();
            cloudPValue = cloudo.getCloudP()->getValue();
            cloudFvalue = cloudo.getCloudP()->getCloudF()->getValue();

            // The number of lesions that will be created on any given organ is proportional to that organ's exposed surface area.
            // NOTE: Should this organ get lesions proportional to total organ area or organ set area?
            newLesionsFromOrgan = cloudo.getDisease()->newLesions(cloudOValue,healthAreaProportion);
            newLesionsFromPlant = cloudo.getDisease()->newLesions(cloudPValue,healthAreaProportion) * 
                                  getProportionFromTotalArea(); 
            newLesionsFromField = cloudo.getDisease()->newLesions(cloudFvalue,healthAreaProportion) *
                                  getProportionFromTotalArea(); // NOTE: Same as above
            
            // Uncomment the following line to debug this step
            
            // NOTE: This hardcoded physiological life should be 
            //       replaced by a dynamic threshold in the .yaml file.
            //       We could also modulate the number of lesions 
            //       created based on some dynamic age factor.
            if ((newLesionsFromOrgan+newLesionsFromPlant+newLesionsFromField) > 0 && physiologicalLife >= 5) {
                newLesions = newLesionsFromOrgan+newLesionsFromPlant+newLesionsFromField;
                //std::cout << "Organ: " << organNumber << "\tnew lesions: " << newLesions << std::endl;
                
                lesionCohorts.emplace_back(newLesions, &cloudo);
                totalLesions += newLesions;

                // Add Spores that will be removed because were used to infect the tissue
                cloudo.addSporesToBeRemoved(newLesionsFromOrgan);
                cloudo.getCloudP()->addSporesToBeRemoved(newLesionsFromPlant);
                cloudo.getCloudP()->getCloudF()->addSporesToBeRemoved(newLesionsFromField);
            } 
            newLesionsFromOrgan = newLesionsFromPlant = newLesionsFromField = 0;            
        }
    }

    for (auto& lc : lesionCohorts) {
        lc.integration();
        diseaseArea += lc.getTotalArea();
        latentDiseaseArea += lc.getLatentArea();
        infectionDiseaseArea += lc.getInfectionArea();
        necroticDiseaseArea += lc.getNecroticArea();
        visibleLesions += lc.getVisibleLesions();
        visibleDiseaseArea += lc.getVisibleArea();
        invisibleDiseaseArea += lc.getInvisibleArea();
    }

    cloudIntegration();
    if (!cloudsO.empty()) {
        cloudOValue = cloudAmount();
        cloudPValue = cloudsO[0].getCloudP()->getValue();
        cloudFvalue = cloudsO[0].getCloudP()->getCloudF()->getValue();
    }
    
    
    dailyDiseaseArea = fmax(0,diseaseArea - dailyDiseaseArea);
    dailyVisibleDiseaseArea = fmax(0,visibleDiseaseArea - dailyVisibleDiseaseArea);
    dailySenescenceArea = fmax(0,senescenceArea - dailySenescenceArea);
    dailyLatentDiseaseArea = fmax(0,latentDiseaseArea - dailyLatentDiseaseArea);
    dailyInfectionDiseaseArea = fmax(0,infectionDiseaseArea - dailyInfectionDiseaseArea);
    dailyNecroticDiseaseArea = fmax(0,necroticDiseaseArea - dailyNecroticDiseaseArea);
    dailyTotalLesions = fmax(0,totalLesions - dailyTotalLesions);
    dailyVisibleLesions = fmax(0,visibleLesions - dailyVisibleLesions);
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
    std::ostringstream convert;
    convert << organNumber << "," 
            << Basic::getWeather()->getYearDoy() << "," << totalArea << "," << senescenceArea << "," 
            << Utilities::formatfloat(diseaseArea, 4) << ","
            << Utilities::formatfloat(visibleDiseaseArea, 4) << "," 
            << Utilities::formatfloat(invisibleDiseaseArea, 4) << ","
            << Utilities::formatfloat(totalLesions / totalArea, 4) << ","
            << Utilities::formatfloat(physiologicalLife, 4) << "," << dailyTotalLesions << "," 
            << totalLesions << "," 
            << Utilities::formatfloat(cloudOValue) << "," << Utilities::formatfloat(cloudPValue) << ","
            << Utilities::formatfloat(cloudFvalue) << "," << healthAreaProportion << "," 
            << getProportionFromTotalArea() << "," << Basic::getWeather()->getWetDur() << ","
            << newLesionsFromOrgan << "," << newLesionsFromPlant << "," << newLesionsFromField;
    Basic::output.push_back(convert.str());
}

void Organ::cloudIntegration() {
    for (auto& cloudO : cloudsO) {
        cloudO.integration();
    }
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