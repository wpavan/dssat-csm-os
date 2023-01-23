#include "plant.h"
#include "simulator.h"

#include<sstream>
#include<vector>
#include<iostream>
#include<fstream>
#include<string>

int Plant::qtd = 0;
int Plant::firstOutputCall = 0;

Plant::Plant() {
    std::vector<InitialCondition> &vectIC = Simulator::getInstance()->getInitialConditions();
    InitialCondition *ic;

    for (unsigned int i = 0; i < vectIC.size(); i++) {
        ic = &vectIC[i];
        cloudsP.emplace_back(ic->getCloud()->getDisease(), ic->getCloud());
    }
    //Basic::output.push_back("Day, PlantArea, DiseaseArea, Density, Severity, LatentDArea, InfectionDArea, NecroticDArea,SenescenceArea");
}

void Plant::integration() {
    totalArea = diseaseArea = latentDiseaseArea = infectionDiseaseArea = necroticDiseaseArea = visibleDiseaseArea = invisibleDiseaseArea = totalLesions = visibleLesions = senescenceArea = 0;
    int newOrgan = 0;
    double cloudOValue = 0, cloudPValue = 0, cloudFvalue = 0;
    Organ *o;
    for (unsigned int i = 0; i < organs.size(); i++) {
        o = &organs[i];
        if(o->getSenescenceArea() < o->getTotalArea()) {
            o->integration();
            diseaseArea += o->getDiseaseArea();
            latentDiseaseArea += o->getLatentDiseaseArea();
            infectionDiseaseArea += o->getInfectionDiseaseArea();
            necroticDiseaseArea += o->getNecroticDiseaseArea();
            visibleDiseaseArea += o->getVisibleDiseaseArea();
            invisibleDiseaseArea += o->getInvisibleDiseaseArea();
            visibleLesions += o->getVisibleLesions();
            totalLesions += o->getTotalLesions();
        } else {
            //o->cloudIntegration();
            //printf("## Organ: %d died - integration!!!\n",o->getOrganNumber());
        }
        totalArea += o->getTotalArea();
        senescenceArea += o->getSenescenceArea();
        cloudOValue += o->cloudAmount();
    }

    CloudP *cloud;
    for (unsigned int i = 0; i < cloudsP.size(); i++) {
        cloud = &cloudsP[i];
        cloud->integration();
    }

    newOrgan = Simulator::getInstance()->getCropInterface()->hasNewOrgan();
    if (newOrgan > 0) {
        //printf("Creating new organ: %i\n",newOrgan);
        organs.emplace_back(cloudsP, newOrgan, Simulator::getInstance()->getCropInterface()->getOrganArea(newOrgan));
    }

    cloudPValue = cloud->getValue();
    cloudFvalue = cloud->getCloudF()->getValue();

    std::ostringstream convert;
    //Plant, YearDoy, TotalArea, Senesced, Diseased, VisibleArea, InvisibleArea, TotalLesions, CloudO, CloudP, CloudF
    convert << ID << "," << Basic::getWeather()->getYearDoy() << "," << totalArea << "," << senescenceArea << "," << diseaseArea << "," 
            << visibleDiseaseArea << "," << invisibleDiseaseArea << "," << totalLesions << "," 
            << latentDiseaseArea << "," << infectionDiseaseArea << "," << necroticDiseaseArea << ","
            << Utilities::formatDouble(cloudOValue) << "," << Utilities::formatDouble(cloudPValue) << "," 
            << Utilities::formatDouble(cloudFvalue);
    Basic::output.push_back(convert.str());

}

void Plant::output() {
    std::ostringstream convert;
    //convert << "Cpp_Plant_" << getID() << ".txt";
    Basic::getOutput("Cpp_Plant.txt", this->firstOutputCall);
    this->firstOutputCall++;

    // Speedup the model removing outputs
    //std::cout << "\nPlant " << getID() << ":\n";
    //for(unsigned int i=0; i<Basic::output.size(); i++)
    //{
    //    std::cout << Basic::output[i] << std::endl;
    //}

    Organ *o;
    for (unsigned int i = 0; i < organs.size(); i++) {
        o = &organs[i];
        o->output();
    }

    CloudP *cp;
    for (unsigned int i = 0; i < cloudsP.size(); i++) {
        cp = &cloudsP[i];
        cp->output();
    }
}

void Plant::rate() {
    Organ *o;
    for (unsigned int i = 0; i < organs.size(); i++) {
        o = &organs[i];
        if(o->getSenescenceArea() < o->getTotalArea()) {
            o->setProportionFromTotalArea(o->getTotalArea()/totalArea);
            o->rate();
        } else {
            //printf("## Organ: %d died - rate!!!\n",o->getOrganNumber());
        }
    }
}
