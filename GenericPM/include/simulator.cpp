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
#include "manager.h"
#include "../../FlexibleIO/Data/FlexibleIO.hpp"
#include "utilities.h"

#include <sstream>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>

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
}

void Simulator::integration() {
    float dArea = 0, tArea=0, sArea=0;
    int seedAge = 0;

    initialCondition.integration();

    CouplingData *couplingData = CouplingData::getInstance();
    CouplingPointID cp = initialCondition.getCloud()->getDisease()->getDamageCP();

    plant->integration();

    if (plant != nullptr && plant->getOrgans().size() > 0) {
        dArea = plant->getDiseaseArea();
        tArea = plant->getTotalArea();
        sArea = plant->getSenescenceArea();
        seedAge = plant->getOrgans().size();
        //pDArea = (dArea/(tArea-sArea)*100);
        //printf("Int YRDOY: %i TArea: %f DArea: %f SArea: %f\n", *YRDOY, tArea,dArea,sArea);
        //*PSDD = (dArea/tArea*5);
        // NOTE: This needs to be recalibrated to remove hardcoded 
        //       values. And to only happen once because as it 
        //       currently stands, only the last value of PSDD will
        //       get sent back to DSSAT.
        if (tArea > 0) {
            couplingData->overwriteCouplingValue(cp, (dArea/tArea)*15);
        } else {
            couplingData->overwriteCouplingValue(cp, 0);
        }        
        //printf("ORIGINAL YRDOY: %i CloudF: %f PSDD %f\n",*YRDOY, s->getPlants()[0].getCloudsP()[0].getCloudF()->getValue(), PSDD);
        //printf("YRDOY: %i Plant Total Area: %f Disease Area: %f Senescence Area: %f AREALF: %f PDLA: %f PLFAD: %f\n", *YRDOY, tArea,dArea,sArea,*AREALF,*PDLA,*PLFAD);
        //printf("YRDOY: %i SDWT: %f PSDD: %f\n", *YRDOY, *SDWT, *PSDD);
    }
}

void Simulator::output() {
    initialCondition.output();
    plant->output();
}

/**
 * Simulator rate function
 * 
 * For each of the initial conditions and each of the plants in the simulation, call their 
 * rate functions. These calls propagate downwards (e.g. to organs).
 */
void Simulator::rate() {
    CouplingData *couplingData = CouplingData::getInstance(); 

    InitialCondition *ic;
    FlexibleIO *fio = FlexibleIO::getInstance();

    float CloudField;
    float newOrgan = cropinterface->getOrgansQtd()+1;

    // NOTE: This code (using the soil information) is for FHB, not WB
    //       We should get the WB code from GenericPM-Spores and parameterize
    //       a new disease in the YAML file which corresponds to external inoculum.
    //
    // This is also confirmed the only place that the spores were being
    // generated for real once the spores module was removed.
    /*if(plants.size()>0) {
        // Run rate function from yaml...
        SL1 = fio->getReal("PEST", "SL1");
        SLL1 = fio->getReal("PEST", "SLL1");
        SDUL1 = fio->getReal("PEST", "SDUL1");
        SSAT1 = fio->getReal("PEST", "SSAT1");

        SW = std::min(100.0f, std::max(0.0f, (SL1-SLL1)/(SSAT1-SLL1)*100));
        CloudField = Utilities::runExpressionFunction(SW, plants[0].getCloudsP()[0].getDisease()->getSWF()); 
        // 0.0000005*exp(0.20*x) 

        plants[0].getCloudsP()[0].getCloudF()->addSporesCreated(CloudField);
    }*/
    
    CouplingPointID organCP = initialCondition.getCloud()->getDisease()->getOrganCP();
    float *organCPVal = couplingData->getCouplingValue(organCP);
    float organCPValPrev = couplingData->getCouplingValuePrev(organCP); 

    // This should generically perform the organ addition step regardless of the coupling point.
    if (*organCPVal - organCPValPrev > 0) {
        // NOTE: Ask Dr. Pavan about this following commented code:
        /*if(first == 0){
            FSEED = *YRDOY;
            fio->setIntegerMemory("PEST", "FSEED", FSEED);
            first = 1;
        }*/
        // std::cout << "New organ growth detected:\nOrgan number: " << newOrgan 
        //     << "\nValue: " << *organCPVal 
        //     << "\nPrevious Value: " << organCPValPrev 
        //     << "\nGrowth: " << (*organCPVal - organCPValPrev) 
        //     << std::endl;
        cropinterface->setOrganArea(newOrgan, (*organCPVal - organCPValPrev));
        organCPValPrev = *organCPVal;
    }

    /** For each Initial Condition call the rate function */
    initialCondition.rate();

    /** Call the rate function for the Plant */
    plant->rate();
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
    // std::cout << "- Updating current YEARDOY from " << getCurrentYearDoy() << " to " << yearDoy << std::endl;
    while(util.addOneDay(getCurrentYearDoy()) < yearDoy) {
        setCurrentYearDoy(util.addOneDay(getCurrentYearDoy()));
        Weather::getInstance()->update();
        rate();
        integration();
    }
    setCurrentYearDoy(yearDoy);
    // std::cout << "- Current YEARDOY is now " << getCurrentYearDoy() << std::endl;
}
