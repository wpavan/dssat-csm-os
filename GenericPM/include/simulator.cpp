/**
 * @file simulator.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "simulator.h"
#include "disease.h"
#include "cropinterface.h"
#include "initialcondition.h"
#include "weather.h"
#include "../../FlexibleIO/Data/FlexibleIO.hpp"

#include <sstream>
#include <vector>
#include <iostream>
#include <string>

Simulator::Simulator() {
    initialization();
}

Simulator* Simulator::instance = nullptr;

Simulator* Simulator::getInstance() {
    if (instance == nullptr) {
        instance = new Simulator();
    }
    return instance;
}

Simulator* Simulator::newInstance() {
    instance=nullptr;
    return getInstance();
}

void Simulator::initialization() {
    cropinterface = CropInterface::newInstance();
    cropinterface->start();
    inputPST();

    std::vector<Disease*> &diseases = Disease::getDisease();
    for (unsigned int i = 0; i < diseases.size(); i++) {
        initialConditions.emplace_back(diseases[i]);
    }
}

void Simulator::inputPST() {
    if (Disease::getDisease().size() == 0) {
        Disease *disease = new Disease();
        FlexibleIO *flexibleio = FlexibleIO::getInstance();
        
        std::string str;
        float f; 
        float arraysize3[3], arraysize4[4];
        
        str = flexibleio->getChar("PST", "PESTID#");
        disease->setId(std::stoi(str.substr(2, str.size()),nullptr,0));
        
        disease->setDescription(flexibleio->getChar("PST", "PSTNAME"));
        
        disease->setDailySporeProductionPerLesion((float)flexibleio->getReal("PST", "DSPL"));
        
        f = flexibleio->getRealIndex("PST", "SPE", 1);
        arraysize4[0] = (float) f;
        f = flexibleio->getRealIndex("PST", "SPE", 2);
        arraysize4[1] = (float) f;
        f = flexibleio->getRealIndex("PST", "SPE", 3);
        arraysize4[2] = (float) f;
        f = flexibleio->getRealIndex("PST", "SPE", 4);
        arraysize4[3] = (float) f;
        disease->setCohortAgeSet(arraysize4);
        
        f = flexibleio->getRealIndex("PST", "SCF", 1);
        arraysize3[0] = (float) f;
        f = flexibleio->getRealIndex("PST", "SCF", 2);
        arraysize3[1] = (float) f;
        f = flexibleio->getRealIndex("PST", "SCF", 3);
        arraysize3[2] = (float) f;
        disease->setSporulationCrowdingFactorsSet(arraysize3);
        
        disease->setMaxSporeCloudsDensity((float) flexibleio->getReal("PST", "MSCD"));
        
        disease->setProportionFromOrganToPlantCloud((float) flexibleio->getReal("PST", "SPO2P"));
        
        disease->setProportionFromPlantToFieldCloud((float) flexibleio->getReal("PST", "SPP2F"));
        
        disease->setVectorSizeCloudF(flexibleio->getIntegerIndex("PST", "CCFPO", 1));
        
        disease->setVectorSizeCloudP(flexibleio->getIntegerIndex("PST", "CCFPO", 2));
        
        disease->setVectorSizeCloudO(flexibleio->getIntegerIndex("PST", "CCFPO", 3));
        
        disease->setMRRS(flexibleio->getInteger("PST", "MRRS"));
        
        disease->setInitialInoculum((float) flexibleio->getReal("PST", "II"));
        
        disease->setAcumulateFavorability((float) flexibleio->getReal("PST", "AFII"));    
        
        f = flexibleio->getRealIndex("PST", "TFS", 1);
        arraysize3[0] = (float) f;
        f = flexibleio->getRealIndex("PST", "TFS", 2);
        arraysize3[1] = (float) f;
        f = flexibleio->getRealIndex("PST", "TFS", 3);
        arraysize3[2] = (float) f;
        disease->setTemperatureFavorabilitySet(arraysize3);
        
        disease->setInfectionEfficiency((float) flexibleio->getReal("PST", "IE"));
        
        disease->setInitialPustuleSize((float) flexibleio->getReal("PST", "IPS"));
        
        disease->setLatentPeriod(flexibleio->getInteger("PST", "LP"));
        
        disease->setInfectionPeriod(flexibleio->getInteger("PST", "IP"));
        
        disease->setWetnessThreshold((float) flexibleio->getReal("PST", "WT"));
        
        disease->setHostFactor((float) flexibleio->getReal("PST", "HF"));
        
        disease->setInvisibleGrowthFunction(flexibleio->getChar("PST", "IGF"));
        
        disease->setVisibleGrowthFunction(flexibleio->getChar("PST", "VGF"));

        disease->setWetnessFunction(flexibleio->getChar("PST", "WF"));

        disease->setDispersionFrequency(flexibleio->getChar("PST", "DRE"));

        disease->setRhFactor(flexibleio->getChar("PST", "RHFac"));
    }
}

void Simulator::integration() {
    InitialCondition *ic;
    for (unsigned int i = 0; i < initialConditions.size(); i++) {
        ic = &initialConditions[i];
        ic->integration();
    }
    Plant *p;
    for (unsigned int i = 0; i < plants.size(); i++) {
        p = &plants[i];
        p->integration();
    }
}

void Simulator::output() {
    InitialCondition *ic;
    for (unsigned int i = 0; i < initialConditions.size(); i++) {
        ic = &initialConditions[i];
        ic->output();
    }
    Plant *p;
    for (unsigned int i = 0; i < plants.size(); i++) {
        p = &plants[i];
        p->output();
    }
}

/**
 * Simulator rate function
 * 
 * For each of the initial conditions and each of the plants in the simulation, call their 
 * rate functions. These calls propagate downwards (e.g. to organs).
 */
void Simulator::rate() {
    InitialCondition *ic;
    Plant *p;

    /** If Planting Date is the current day, instantiate the Plant */
    if (CropInterface::getInstance()->getPlantingDate() == getCurrentYearDoy()) {
        //printf("NEW Plant: PlantingDate: %i - CurrentYearDoy: %i \n",CropInterface::getInstance()->getPlantingDate(), getCurrentYearDoy());
        plants.emplace_back();
    }    

    /** For each Initial Condition call the rate function */
    for (unsigned int i = 0; i < initialConditions.size(); i++) {
        ic = &initialConditions[i];
        ic->rate();
    }

    /** For each Plant, call the rate function */
    for (unsigned int i = 0; i < plants.size(); i++) {
        p = &plants[i];
        p->rate();
    }
}

/**
 * Synchronize the current DSSAT and simulator dates
 * 
 * This function takes the current YEARDOY from DSSAT and updates the simulator date accordingly. 
 * This can happen because DSSAT may run for days without calling the GDM, leading to a mismatch. 
 * This function runs the integration and rate functions of the simulator as well, meaning 
 * information is synchronized.
 * 
 * @param yearDoy The current DSSAT date
 */
void Simulator::updateCurrentYearDoy(int yearDoy) {
    while(util.addOneDay(getCurrentYearDoy()) < yearDoy) {
        setCurrentYearDoy(util.addOneDay(getCurrentYearDoy()));
        Weather::getInstance()->update();
        rate();
        integration();
    }
    setCurrentYearDoy(yearDoy);
}
