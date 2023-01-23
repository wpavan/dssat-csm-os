#include "organ.h"
#include "simulator.h"
#include "cloudo.h"
#include "cloudp.h"
#include "cloudf.h"
#include "lesioncohort.h"
#include "utilities.h"
#include "basic.h"

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>

int Organ::firstOutputCall = 0;

void Organ::integration() {
    if (!isAlive()) return;
    
    CloudO *cloudo;
    LesionCohort *lc;

    float cloudDensity = 0;
    double cloudOValue = 0, cloudPValue = 0, cloudFvalue = 0;
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
    latentDiseaseArea = infectionDiseaseArea = necroticDiseaseArea = 0;    newLesions = 0;

    if (suceptible) {
        for (unsigned int i = 0; i < cloudsO.size(); i++) {
            cloudo = &cloudsO[i];
            physiologicalLife += Utilities::trapezoidalFunction(Basic::getWeather()->getTMean(), cloudo->getDisease()->getCardinalTempPhysiologicalLife());
            //cloudOValue = 0;
            cloudOValue = cloudAmount();
            //cloudPValue = 0;
            cloudPValue = cloudo->getCloudP()->getValue();
            cloudFvalue = cloudo->getCloudP()->getCloudF()->getValue();

            newLesionsFromOrgan = cloudo->getDisease()->newLesions(cloudOValue,healthAreaProportion);
            newLesionsFromPlant = cloudo->getDisease()->newLesions(cloudPValue,healthAreaProportion)*getProportionFromTotalArea();
            //std::cout << "cloudF Antes: " << cloudFvalue << std::endl;
            newLesionsFromField = cloudo->getDisease()->newLesions(cloudFvalue,healthAreaProportion)*getProportionFromTotalArea();
            //std::cout << "cloudF: " << cloudFvalue << std::endl;
            //std::cout << "cloudP: " << cloudPValue << std::endl;
            //std::cout << "cloudO: " << cloudOValue << std::endl;
            //std::cout << "healthAreaProportion " << healthAreaProportion << " getProportionFromTotalArea " << getProportionFromTotalArea() <<
            //" newLesionsFromField " << newLesionsFromField << std::endl;
            
            if ((newLesionsFromOrgan+newLesionsFromPlant+newLesionsFromField) > 0 && physiologicalLife >= 5) {
                newLesions = newLesionsFromOrgan+newLesionsFromPlant+newLesionsFromField;
                lesionCohorts.emplace_back(newLesions, cloudo);
                totalLesions += newLesions;

                // Add Spores that will be removed because were used to infect the tissue
                cloudo->addSporesToBeRemoved(newLesionsFromOrgan);
                cloudo->getCloudP()->addSporesToBeRemoved(newLesionsFromPlant);
                cloudo->getCloudP()->getCloudF()->addSporesToBeRemoved(newLesionsFromField);
            } else {
                newLesionsFromOrgan = newLesionsFromPlant = newLesionsFromField = 0;
            }
        }
    }

    for (unsigned int i = 0; i < lesionCohorts.size(); i++) {
        lc = &lesionCohorts[i];
        lc->integration();
        diseaseArea += lc->getTotalArea();
        latentDiseaseArea += lc->getLatentArea();
        infectionDiseaseArea += lc->getInfectionArea();
        necroticDiseaseArea += lc->getNecroticArea();
        visibleLesions += lc->getVisibleLesions();
        visibleDiseaseArea += lc->getVisibleArea();
        invisibleDiseaseArea += lc->getInvisibleArea();
    }

    cloudIntegration();
    //cloudOValue = 0;
    cloudOValue = cloudAmount();
    //cloudPValue = 0;
    cloudPValue = cloudo->getCloudP()->getValue();
    cloudFvalue = cloudo->getCloudP()->getCloudF()->getValue();
    
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
    std::ostringstream convert;
    convert << organNumber << "," 
            << Basic::getWeather()->getYearDoy() << "," << totalArea << "," << senescenceArea << "," 
            << Utilities::formatDouble(diseaseArea, 4) << ","
            << Utilities::formatDouble(visibleDiseaseArea, 4) << "," 
            << Utilities::formatDouble(invisibleDiseaseArea, 4) << ","
            << Utilities::formatDouble(totalLesions / totalArea, 4) << ","
            << Utilities::formatDouble(physiologicalLife, 4) << "," << dailyTotalLesions << "," 
            << totalLesions << "," 
            << Utilities::formatDouble(cloudOValue) << "," << Utilities::formatDouble(cloudPValue) << ","
            << Utilities::formatDouble(cloudFvalue) << "," << healthAreaProportion << "," 
            << getProportionFromTotalArea() << "," << Basic::getWeather()->getWetDur() << ","
            << newLesionsFromOrgan << "," << newLesionsFromPlant << "," << newLesionsFromField;
    Basic::output.push_back(convert.str());
}
void Organ::cloudIntegration() {
    for (unsigned int i = 0; i < cloudsO.size(); i++) {
        (&cloudsO[i])->integration();
    }
}
float Organ::cloudAmount() {
    float cloudOValue=0;
    for (unsigned int i = 0; i < cloudsO.size(); i++) {
        cloudOValue += (&cloudsO[i])->getValue();
    }
    return cloudOValue;
}
void Organ::output() {
    std::ostringstream convert;
    //convert << "Cpp_Organ_" << getOrganNumber() << ".txt";
    Basic::getOutput("Cpp_Organ.txt",this->firstOutputCall);
    this->firstOutputCall++;
    // Speedup the model removing outputs
    //std::cout << "\nOrgan " << getID() << ":";
    //for(unsigned int i=0; i<Basic::output.size(); i++)
    //{
    //    std::cout << Basic::output[i] << std::endl;
    //}

    CloudO *cloudo;
    for (unsigned int i = 0; i < cloudsO.size(); i++) {
        cloudo = &cloudsO[i];
        cloudo->output();
    }
    
//    LesionCohort *lc;
//    for (unsigned int i = 0; i < lesionCohorts.size(); i++) {
//        lc = &lesionCohorts[i];
//        lc->output();
//    }

}

void Organ::rate() {
    // Calculate the ratio due senescence based on previews day
    float actualDisease=0, ratioSenescence = this->senescenceArea / this->totalArea;
    // Update the senescence area for the current day
    this->senescenceArea = Simulator::getInstance()->getCropInterface()->getSenescenceOrganArea(organNumber);
    // Recalculate the ratio due senescence and take the difference from previews ratio
    ratioSenescence = (this->senescenceArea / this->totalArea) - ratioSenescence;
    // Update the total organ area (current day)
    this->totalArea = Simulator::getInstance()->getCropInterface()->getOrganArea(organNumber);
    
    if (!suceptible && this->totalArea > 0) {
        suceptible = true;
    }
    if (!isAlive())
        return;
    CloudO *cloudo;
    LesionCohort *lc;
    //printf("OrganNumber: %i OrganTotalArea: %f healthAreaProportion: %f \n",this->organNumber, this->totalArea,healthAreaProportion);
    for (unsigned int i = 0; i < lesionCohorts.size(); i++) {
        lc = &lesionCohorts[i];
        // Must affect the disease area related with senescent area
        if (ratioSenescence > 0) {
            //printf("ID: %i Age: %i VisibleArea: %f InvisibleArea: %f RatioDueDefoliation: %f\n", lc->getID(), lc->getAge(), lc->getVisibleArea(),lc->getInvisibleArea(),CropInterface::getInstance()->getRatioDueDefoliation(1));
            lc->setVisibleArea(lc->getVisibleArea() * (1-ratioSenescence));
            lc->setInvisibleArea(lc->getInvisibleArea() * (1-ratioSenescence));
            lc->setTotalArea(lc->getVisibleArea()+lc->getInvisibleArea());
            //printf("ID: %i Age: %i VisibleArea: %f InvisibleArea: %f \n", lc->getID(), lc->getAge(), lc->getVisibleArea(),lc->getInvisibleArea());
        }
        actualDisease = actualDisease + lc->getTotalArea();
    }
    // Updating the disease amount on organ
    setDiseaseArea(actualDisease);
    // Calculating the health area proportion
    healthAreaProportion = Utilities::getHealthAreaProportion(getDiseaseArea(), getTotalArea(), getSenescenceArea());

    for (unsigned int i = 0; i < lesionCohorts.size(); i++) {
        lc = &lesionCohorts[i];
        lc->setOrganHealthAreaProportion(healthAreaProportion);
        lc->rate();
    }
}
