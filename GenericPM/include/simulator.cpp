/**
 * @file simulator.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "../../FlexibleIO/Data/FlexibleIO.hpp"

#include "injection.h"
#include "simulator.h"
#include "disease.h"
#include "cropinterface.h"
#include "initialcondition.h"
#include "weather.h"
#include "utilities.h"
#include "coupling.h"

#include <sstream>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>

bool diseaseHasOutput(Disease *disease) {
    if (disease->getOutputInjections().size() > 0) {
        return true;
    } else {
        for (const auto& inj : disease->getRateInjections()) {
            if (inj.getEndpoint() == InjEndpoint::OUTPUT) {
                return true;
            }
        }
        for (const auto& inj : disease->getIntegrationInjections()) {
            if (inj.getEndpoint() == InjEndpoint::OUTPUT) {
                return true;
            }
        }
    }
    return false;
}

int Simulator::currentYearDoy = -99;

void Simulator::clearOutputLog() {
    this->loggedOutputs.clear();
}

void Simulator::logOutput(std::string varName, float value) {
    this->loggedOutputs.emplace_back(varName, value);
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
    
    /* GDM2 Implementation of spore generation
     * 
     * Steps required to get spores added into the corresponding CloudF:
     * 1. Determine the amount of inoculum generated from injection obj.
     * 2. Find the corresponding CloudF for the disease.
     * 3. Add the inoculum to the CloudF.
     */

    // Load CouplingData instance for modification of damage values.
    CouplingData *cpData = CouplingData::getInstance();

    // Declare helpers for non-cp endpoints.
    float fioFloat = 0;
    std::string fioString;

    float outputVal;
    std::string outputVarName;

    for (auto& injection : disease->getRateInjections()) {
        if (injection.getEndpoint() == InjEndpoint::OUTPUT) {
            injection.apply(outputVal, outputVarName);
            logOutput(outputVarName, outputVal);
        } else if (injection.getEndpoint() == InjEndpoint::FIO) {
            injection.apply(fioString);
            fioFloat = fio->getReal("PEST", fioString);
            if (fioFloat == -99.0f) {
                std::cout << "Warning: FIO variable " << fioString << " has value -99.0f" << std::endl;
                fioFloat = 0.0f;
                std::cout << "  Using 0.0f instead..." << std::endl;
            }
            injection.apply(fioFloat);

            // This choice to separate the "PEST" group from the 
            // "PEST_INJ" group was done to isolate the protected 
            // variables in FIO.
            fio->setRealMemory("PEST", fioString.c_str(), fioFloat);
            // std::cout << "Simulator Rate FIO Injection applied: " 
            //           << "Variable: " << fioString 
            //           << " Value: " << fioFloat 
            //           << std::endl;
        } else if (injection.getEndpoint() == InjEndpoint::PCP) {
            CouplingPointID injCP = strToCPID(injection.getRawEndpoint());
            float *injCPVal = couplingData->getCouplingValue(injCP);
            // std::cout << "CP Injection - " << injection.getRawEndpoint() << " - Value before: " << *injCPVal << std::endl;
            injection.apply(*injCPVal);
            // std::cout << "CP Injection - " << injection.getRawEndpoint() << " - Value after : " << *injCPVal << std::endl;
            // Unknown if this is needed: couplingData->overwriteCouplingValue(injCP, *injCPVal);
        }
    }
    
    CouplingPointID organCP = this->disease->getOrganCP();
    CouplingPointID damageCP = this->disease->getDamageCP();
    if (organCP != CouplingPointID::VALUE) {
        // COUPLING POINT VALUE FOR ORGAN SET VALUE DIRECTLY
        // 
        // Since we cannot directly set the value without breaking 
        // other aspects of the GDM logic, we instead calculate the 
        // raw growth and use that to track the plant value.

        float *organCPVal = couplingData->getCouplingValue(organCP);
        float organCPValPrev = couplingData->getCouplingValuePrev(organCP);
        float damageCPValPrev = couplingData->getCouplingValuePrev(damageCP);

        // `dssatDelta` is the amount of change (growth and senescence) that 
        // has occurred since the last rate call.
        float dssatDelta = *organCPVal - organCPValPrev + damageCPValPrev;

        if (dssatDelta > 0) {
            if (disease->getOrganMode() == OrganMode::COHORT) {
                cropinterface->setOrganArea(newOrgan, (dssatDelta));
            } else if (disease->getOrganMode() == OrganMode::SINGULAR) {
                Plant::getInstance()->getOrganSet(organCP).queueHealthyGrowth(dssatDelta);
            }
            this->logOutput("NEW_GROWTH_" + cpIDToStr(organCP), dssatDelta);
        } else {
            #ifdef DEBUGX
            std::cout << "No new organ growth detected: " 
                    << "Disease: " << this->disease->getDiseaseID() 
                    << " Coupling Point: " << cpIDToStr(organCP) 
                    << " Growth: " << dssatDelta 
                    << std::endl;
            #endif // DEBUG
        }

        CouplingPointID outputCP;
        for (int i = 1; i < static_cast<int>(CouplingPointID::COUNT); i++) {
            outputCP = static_cast<CouplingPointID>(i);
            this->logOutput("CP_" + cpIDToStr(outputCP), *couplingData->getCouplingValue(outputCP));
        }

        // OLD CODE THAT ASSUMES NEW GROWTH = DIFFERENCE IN COUPLING VALUE
        //
        // float *organCPVal = couplingData->getCouplingValue(organCP);
        // float organCPValPrev = couplingData->getCouplingValuePrev(organCP);
        //
        // // This should generically perform the organ addition step regardless of the coupling point.
        // if (*organCPVal - organCPValPrev > 0) {
        //     // NOTE: Ask Dr. Pavan about this following commented code:
        //     /*if(first == 0){
        //         FSEED = *YRDOY;
        //         fio->setIntegerMemory("PEST", "FSEED", FSEED);
        //         first = 1;
        //     }*/
        //
        //     #ifdef DEBUGX
        //     std::cout << "New organ growth detected: " 
        //             << "Disease: " << this->disease->getDiseaseID() 
        //             << " Coupling Point: " << cpIDToStr(organCP) 
        //             << " Growth: " << (*organCPVal - organCPValPrev) 
        //             << std::endl;
        //     #endif // DEBUG
        //     if (disease->getOrganMode() == OrganMode::COHORT) {
        //         cropinterface->setOrganArea(newOrgan, (*organCPVal - organCPValPrev));
        //     } else if (disease->getOrganMode() == OrganMode::SINGULAR) {
        //         Plant::getInstance()->getOrganSet(organCP).queueHealthyGrowth((*organCPVal - organCPValPrev));
        //     }
        // }
        
    } else {
        #ifdef DEBUGX
            std::cout << "Constant value organ: " 
                    << "Disease: " << this->disease->getDiseaseID() 
                    << " Coupling Point: " << cpIDToStr(organCP) 
                    << " Value: " << cropinterface->getConstValue()
                    << std::endl;
            #endif // DEBUG
    }

    /** For each Initial Condition call the rate function */
    initialCondition.rate();

    // Orchestrate rate calls for all CloudOs, then CloudPs, then CloudFs.
    Plant* plant = getPlant();
    for (auto& OrganSet : plant->getOrgans()) {
        for (auto& organ : OrganSet.organs) {
            for (auto& cloudO : organ.getCloudsO()) {
                if (cloudO.getDisease() == this->disease) {
                    cloudO.rate();
                }
            }
        }
    }
    for (auto& cloudP : plant->getCloudsP()) {
        if (cloudP.getDisease() == this->disease) {
            cloudP.rate();
        }
    }
    initialCondition.getCloud()->rate();

    try {
        initialCondition.getCloud()->addSporesCreated(disease->getINOC_EXT()->evaluate());
    } catch (const std::runtime_error& e) {
        std::cerr << "Error evaluating INOC_EXT expression for DiseaseID: " << disease->getDiseaseID() << std::endl << "Exception: " << e.what() << std::endl;
        // Default to 0 inoculum if evaluation fails
        initialCondition.getCloud()->addSporesCreated(0.0f);
    }

    /** Call the rate function for the Plant */
    plant->rate();
}


void Simulator::integration() {
    // Declare helper values for integration 
    float diseaseValue = 0, totalValue=0, sArea=0;
    int seedAge = 0;

    // Load CouplingData instance for modification of damage values.
    CouplingData *couplingData = CouplingData::getInstance();
    CouplingPointID damageCP = this->disease->getDamageCP();

    // Declare helper values for non-cp endpoints.
    float* biologicalFactor = disease->getBiologicalFactorRef();
    float outputVal;
    std::string outputVarName;

    // Perform all integration injections.
    for (auto& injection : disease->getIntegrationInjections()) {
        if (injection.getEndpoint() == InjEndpoint::INFECTION_BIOLOGICAL_FACTOR) {
            injection.apply(*biologicalFactor);
        } else if (injection.getEndpoint() == InjEndpoint::OUTPUT) {
            injection.apply(outputVal, outputVarName);
            logOutput(outputVarName, outputVal);
        }
    }

    initialCondition.integration(disease);

    // Orchestrate rate calls for all CloudOs, then CloudPs, then CloudF.
    //  CloudOs
    Plant* plant = getPlant();
    for (auto& OrganSet : plant->getOrgans()) {
        for (auto& organ : OrganSet.organs) {
            for (auto& cloudO : organ.getCloudsO()) {
                if (cloudO.getDisease() == this->disease) {
                    cloudO.integration();
                }
            }
        }
    }

    //  CloudPs
    for (auto& cloudP : plant->getCloudsP()) {
        if (cloudP.getDisease() == this->disease) {
            cloudP.integration();
        }
    }

    // CloudF
    initialCondition.getCloud()->integration();

    plant->integration();

    if (getPlant() != nullptr && getPlant()->getOrgans().size() > 0) {
        // NOTE: Testing out the use of daily disease value
        // float diseaseDailyValue = getPlant()->getDailyDiseaseValue();

        // Previous damageCP hardcoded expression
        // dArea = getPlant()->getDiseaseValue();
        // tArea = getPlant()->getTotalValue();
        // seedAge = getPlant()->getOrgans().size();
        // pDArea = (dArea/(tArea-sArea)*100);

        if (totalValue > 0) {
            // Get the current CP value
            float existingDamageValue = *couplingData->getCouplingValue(damageCP);

            // Determine the daily disease damage value
            // Default to 0 favorability if evaluation fails
            float damageValue = 0.0f;
            try {
                damageValue = disease->getDAMAGE()->evaluate(); 
            } catch (const std::runtime_error& e) {
                std::cerr << "Error evaluating DAMAGE expression for DiseaseID: " << disease->getDiseaseID() << std::endl << "Exception: " << e.what() << std::endl;
                
            }

            // Set the damage CP value to the sum 
            couplingData->overwriteCouplingValue(damageCP, existingDamageValue + damageValue);
            
            // Previous implementations:
            // couplingData->overwriteCouplingValue(damageCP, diseaseDailyValue);
            // couplingData->overwriteCouplingValue(damageCP, diseaseValue);
            
        } else {
            couplingData->overwriteCouplingValue(damageCP, 0);
        }        
    }
}

void Simulator::output() {
    initialCondition.output();
    getPlant()->output();

    // Do something to report the output.name and output.value on that day.
    // For a useable long format, one file could be created per simulator that has:
    // YEARDOY    OUTPUT_NAME    VALUE
    // ...
    // ...
    // 
    // This way, it's very simple to construct.

    if (loggedOutputs.size() > 0 || hasOutput) {
        float outputVal;
        std::string outputVarName;

        // First, compute all specifically "OUTPUT" step injections
        for (auto& injection : disease->getIntegrationInjections()) {
            if (injection.getEndpoint() == InjEndpoint::OUTPUT) {
                injection.apply(outputVal, outputVarName);
                logOutput(outputVarName, outputVal);
            }
        }

        // Then, report all injections that have the "OUTPUT" endpoint (from all steps)
        outputFile.open(outputFileName, std::ios::app);
        for (const auto& output : loggedOutputs) {
            outputFile << currentYearDoy << "\t" << output.varName << "\t" << output.value << "\n";
            // std::cout << currentYearDoy << "\t" << output.varName << "\t" << output.value << "\n";
        }
        outputFile.close();

        // Finally, clear the logged outputs for the next step.
        clearOutputLog();
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
    // std::cout << "- Updating current YEARDOY from " << getCurrentYearDoy() << " to " << yearDoy << std::endl;
    while(util.addOneDay(getCurrentYearDoy()) < yearDoy) {
        setCurrentYearDoy(util.addOneDay(getCurrentYearDoy()));
        Weather::getInstance()->update(getCurrentYearDoy());
        // Weather::getInstance()->update();
        rate();
        integration();
    }
    setCurrentYearDoy(yearDoy);
}
