/**
 * @file manager.cpp
 * 
 * @author Virginia Covert (virginiacovert454@gmail.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include <sstream>
#include "manager.h"
#include "../../FlexibleIO/Data/FlexibleIO.hpp"

std::vector<Simulator*> Manager::simulators;


Manager::Manager() {
    // Initialize the manager
    cropinterface = CropInterface::newInstance();
    cropinterface->start();
}

Manager* Manager::instance = nullptr;

Manager* Manager::getInstance() {
    if (instance == nullptr) {
        instance = new Manager();
    }
    return instance;
}

Manager* Manager::newInstance() {
    instance = nullptr;
    return getInstance();
}

Simulator* Manager::getSimulator(int index) {
    if (index < 0 || index >= simulators.size()) {
        return nullptr; // or throw an exception
    }
    return simulators[index];
}

void Manager::pushSimulator(Simulator* simulator) {
    simulators.push_back(simulator);
}

void Manager::readYAMLFile() {
    // This check ensures that diseases are only entered on the first year
    // of a multi-year simulation.
    if (simulators.size() == 0) {
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
            if (groupName != "-99") {
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

                // Use the *disease to find a new slot in the simulators 
                // vector and then initialize a new Simulator inside it.
                Manager::pushSimulator(new Simulator(disease));
            }
        }
    }
}

void Manager::setCurrentSimDate(int yearDoy) {
    for (auto& simulator : simulators) {
        simulator->setCurrentYearDoy(yearDoy);
    }
}

/*
Execution workflow:
0. Coupling functions are the only ones that are called by fortran.
   These functions have the capability to create and manage the 
   (static) manager instance.
1. Singleton manager instance is created by couplingInit (can we 
   determine if this is the first time?).
2. The manager loads all of the diseases into fio ONCE (during an init 
   ideally).
3. The manager creates a list of simulators and holds onto them in an
   order defined by the user (in the YAML file).
4. The manager can loop through the simulators and propagate the rate,
   integration, and output functions down to lower objects.
NOTES: No objects will be passed back to DSSAT to maintain modularity.
       One simulator contains one initial condition/disease.
*/