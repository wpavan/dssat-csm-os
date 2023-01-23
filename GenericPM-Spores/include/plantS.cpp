#include "plantS.h"
#include "simulatorS.h"

#include<sstream>
#include<vector>
#include<iostream>
#include<fstream>
#include<string>

int PlantS::qtdS = 0;
int PlantS::firstOutputCallS = 0;

PlantS::PlantS() {
    std::vector<InitialConditionS> &vectIC = SimulatorS::getInstanceS()->getInitialConditions();
    InitialConditionS *ic;

    for (unsigned int i = 0; i < vectIC.size(); i++) {
        ic = &vectIC[i];
        cloudsP.emplace_back(ic->getCloud()->getDisease(), ic->getCloud());
    }
    //BasicS::output.push_back("Day, PlantArea, DiseaseArea, Density, Severity, LatentDArea, InfectionDArea, NecroticDArea,SenescenceArea");
}

void PlantS::integrationS() {
    totalArea = diseaseArea = latentDiseaseArea = infectionDiseaseArea = necroticDiseaseArea = visibleDiseaseArea = invisibleDiseaseArea = totalLesions = visibleLesions = senescenceArea = 0;
    int newOrgan = 0;
    double cloudOValue = 0, cloudPValue = 0, cloudFvalue = 0;
    OrganS *o;
    for (unsigned int i = 0; i < organs.size(); i++) {
        o = &organs[i];
        if(o->getSenescenceArea() < o->getTotalArea()) {
            o->integrationS();
            diseaseArea += o->getDiseaseArea();
            latentDiseaseArea += o->getLatentDiseaseArea();
            infectionDiseaseArea += o->getInfectionDiseaseArea();
            necroticDiseaseArea += o->getNecroticDiseaseArea();
            visibleDiseaseArea += o->getVisibleDiseaseArea();
            invisibleDiseaseArea += o->getInvisibleDiseaseArea();
            visibleLesions += o->getVisibleLesionsS();
            totalLesions += o->getTotalLesions();
        } else {
            //o->cloudIntegrationS();
            //printf("## OrganS: %d died - integration!!!\n",o->getOrganNumber());
        }
        totalArea += o->getTotalArea();
        senescenceArea += o->getSenescenceArea();
        cloudOValue += o->cloudAmountS();
    }

    CloudPS *cloud;
    for (unsigned int i = 0; i < cloudsP.size(); i++) {
        cloud = &cloudsP[i];
        cloud->integrationS();
    }

    newOrgan = SimulatorS::getInstanceS()->getCropInterface()->hasNewOrgan();
    if (newOrgan > 0) {
        //printf("Creating new organ: %i\n",newOrgan);
        organs.emplace_back(cloudsP, newOrgan, SimulatorS::getInstanceS()->getCropInterface()->getOrganArea(newOrgan));
    }

    cloudPValue = cloud->getValueS();
    cloudFvalue = cloud->getCloudF()->getValueS();

    std::ostringstream convert;
    //Plant, YearDoy, TotalArea, Senesced, Diseased, VisibleArea, InvisibleArea, TotalLesions, CloudOS, CloudPS, CloudFS
    convert << ID << "," << BasicS::getWeather()->getYearDoy() << "," << totalArea << "," << senescenceArea << "," << diseaseArea << "," 
            << visibleDiseaseArea << "," << invisibleDiseaseArea << "," << totalLesions << "," 
            << latentDiseaseArea << "," << infectionDiseaseArea << "," << necroticDiseaseArea << ","
            << UtilitiesS::formatDoubleS(cloudOValue) << "," << UtilitiesS::formatDoubleS(cloudPValue) << "," 
            << UtilitiesS::formatDoubleS(cloudFvalue);
    BasicS::output.push_back(convert.str());
    //std::cout << ID << "," << BasicS::getWeather()->getYearDoy() << "," << totalArea << "," << senescenceArea << "," << diseaseArea << "," 
    //        << visibleDiseaseArea << "," << invisibleDiseaseArea << "," << totalLesions << "," 
    //        << latentDiseaseArea << "," << infectionDiseaseArea << "," << necroticDiseaseArea << ","
    //        << UtilitiesS::formatDoubleS(cloudOValue) << "," << UtilitiesS::formatDoubleS(cloudPValue) << "," 
    //        << UtilitiesS::formatDoubleS(cloudFvalue)<<std::endl;
    BasicS::output.push_back(convert.str());

}

void PlantS::outputS() {
    std::ostringstream convert;
    //convert << "Cpp_Plant_" << getID() << ".txt";
    BasicS::getOutput("Cpp_Plant.txt", this->firstOutputCallS);
    this->firstOutputCallS++;

    // Speedup the model removing outputs
    //std::cout << "\nPlant " << getID() << ":\n";
    //for(unsigned int i=0; i<BasicS::output.size(); i++)
    //{
    //    std::cout << BasicS::output[i] << std::endl;
    //}

    OrganS *o;
    for (unsigned int i = 0; i < organs.size(); i++) {
        o = &organs[i];
        o->outputS();
    }

    CloudPS *cp;
    for (unsigned int i = 0; i < cloudsP.size(); i++) {
        cp = &cloudsP[i];
        cp->outputS();
    }
}

void PlantS::rateS() {
    OrganS *o;
    for (unsigned int i = 0; i < organs.size(); i++) {
        o = &organs[i];
        if(o->getSenescenceArea() < o->getTotalArea()) {
            o->setProportionFromTotalArea(o->getTotalArea()/totalArea);
            o->rateS();
        } else {
            //printf("## OrganS: %d died - rate!!!\n",o->getOrganNumber());
        }
    }
}
