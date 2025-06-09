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

// This is the placeholder to interact with the groups dedicated to GDM 2.
// NOTE: This name must be changed to properly replace the previous version.
void Simulator::inputPST_FromYaml() {
    // This check ensures that diseases are only entered on the first year
    // of a multi-year simulation.
    if (Disease::getDisease().size() == 0) {
        FlexibleIO *flexibleio = FlexibleIO::getInstance();
        std::string str;
        float f; 
        float arraysize3[3], arraysize4[4];

        // Get group names from PST group.
        int maxDiseases = flexibleio->getInteger("PEST", "MAXDISEASES");
        std::vector<std::string> diseaseHashes;
        std::string storedHash;
        std::istringstream iss(flexibleio->getCharArray("PEST", "DISEASES", std::to_string(maxDiseases)));
        while (iss >> storedHash) {
          diseaseHashes.push_back(storedHash);
        }

        for (std::string groupName : diseaseHashes) {
            if (groupName != "-99"){
                Disease *disease = new Disease();

                // NOTE: The disease ID situation needs to be resolved.
                // NOTE: Description also does not exist, so this should return a -99?
                disease->setDescription(flexibleio->getChar(groupName, "PESTID"));
                
                disease->setDailySporeProductionPerLesion((float) flexibleio->getReal(groupName, "DSPL"));
                
                f = flexibleio->getRealIndex(groupName, "SPE", 1);
                arraysize4[0] = (float) f;
                f = flexibleio->getRealIndex(groupName, "SPE", 2);
                arraysize4[1] = (float) f;
                f = flexibleio->getRealIndex(groupName, "SPE", 3);
                arraysize4[2] = (float) f;
                f = flexibleio->getRealIndex(groupName, "SPE", 4);
                arraysize4[3] = (float) f;
                disease->setCohortAgeSet(arraysize4);

                f = flexibleio->getRealIndex(groupName, "SCF", 1);
                arraysize3[0] = (float) f;
                f = flexibleio->getRealIndex(groupName, "SCF", 2);
                arraysize3[1] = (float) f;
                f = flexibleio->getRealIndex(groupName, "SCF", 3);
                arraysize3[2] = (float) f;
                disease->setSporulationCrowdingFactorsSet(arraysize3);
                
                disease->setMaxSporeCloudsDensity((float) flexibleio->getReal(groupName, "MSCD"));
                
                disease->setProportionFromOrganToPlantCloud((float) flexibleio->getReal(groupName, "SPO2P"));
                
                disease->setProportionFromPlantToFieldCloud((float) flexibleio->getReal(groupName, "SPP2F"));
                
                disease->setVectorSizeCloudF(flexibleio->getIntegerIndex(groupName, "CCFPO", 1));
                
                disease->setVectorSizeCloudP(flexibleio->getIntegerIndex(groupName, "CCFPO", 2));
                
                disease->setVectorSizeCloudO(flexibleio->getIntegerIndex(groupName, "CCFPO", 3));
                
                disease->setMRRS(flexibleio->getInteger(groupName, "MRRS"));
                
                disease->setInitialInoculum((float) flexibleio->getReal(groupName, "II"));
                
                disease->setAcumulateFavorability((float) flexibleio->getReal(groupName, "AFII"));    
                
                f = flexibleio->getRealIndex(groupName, "TFS", 1);
                arraysize3[0] = (float) f;
                f = flexibleio->getRealIndex(groupName, "TFS", 2);
                arraysize3[1] = (float) f;
                f = flexibleio->getRealIndex(groupName, "TFS", 3);
                arraysize3[2] = (float) f;
                disease->setTemperatureFavorabilitySet(arraysize3);
                
                disease->setInfectionEfficiency((float) flexibleio->getReal(groupName, "IE"));
                
                disease->setInitialPustuleSize((float) flexibleio->getReal(groupName, "IPS"));
                
                disease->setLatentPeriod(flexibleio->getInteger(groupName, "LP"));
                
                disease->setInfectionPeriod(flexibleio->getInteger(groupName, "IP"));
                
                disease->setWetnessThreshold((float) flexibleio->getReal(groupName, "WT"));
                
                disease->setHostFactor((float) flexibleio->getReal(groupName, "HF"));
                
                disease->setInvisibleGrowthFunction(flexibleio->getChar(groupName, "IGF"));
                
                disease->setVisibleGrowthFunction(flexibleio->getChar(groupName, "VGF"));

                disease->setWetnessFunction(flexibleio->getChar(groupName, "WF"));

                disease->setDispersionFrequency(flexibleio->getChar(groupName, "DRE"));

                disease->setRhFactor(flexibleio->getChar(groupName, "RHFac"));

                // Added new parameter called sporeModule to the disease class
                // - V. L. Covert 4/1/2025
                disease->setSporeModule(flexibleio->getChar(groupName, "SPOREMODULE"));

                // Added SWF parameter which was previously unused.
                // - V. L. Covert 4/1/2025
                disease->setSWF(flexibleio->getChar(groupName, "SWF"));
                
                disease->printDisease();
            }
        }
    }
}

void Simulator::inputPST() {
    inputPST_FromYaml();
    /*
    // This happens every new initialization phase, but...
    if (Disease::getDisease().size() == 0) {
        // ...this happens only the first time. Once the 
        // disease is created it is not destroyed.
        Disease *disease = new Disease();
        FlexibleIO *flexibleio = FlexibleIO::getInstance();
        
        std::string str;
        float f; 
        float arraysize3[3], arraysize4[4];
        
        // Insert code here to bring in the disease info from yaml groups.
        std::istringstream diseaseHash(flexibleio->getCharArray("PEST", "DISEASES", "5"));
        while (diseaseHash >> str) {
            std::cout << flexibleio->getChar(str, "SPE") << std::endl;
        }

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
    */
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

    // <- Here goes the rate code injection.
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
