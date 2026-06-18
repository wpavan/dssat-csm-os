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
#include "debug_control.h"
#include "cropinterface.h"
#include "initialcondition.h"
#include "utilities.h"
#include "coupling.h"

#include <sstream>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>
#include <algorithm>

// Initialize static members
std::vector<std::string> Simulator::columnOrder;
std::map<std::string, int> Simulator::columnWidths;
bool Simulator::headerWritten = false;

bool diseaseHasOutput(std::shared_ptr<Disease> disease) {
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

void Simulator::clearOutputLog() {
    this->loggedOutputs.clear();
}

void Simulator::logOutput(std::string varName, float value) {
    this->loggedOutputs.emplace_back(varName, value);
    
    // Track this column if it's new
    if (std::find(columnOrder.begin(), columnOrder.end(), varName) == columnOrder.end()) {
        columnOrder.push_back(varName);
        // Set column width to at least 6, but use the name length if longer
        int width = std::max(6, static_cast<int>(varName.length()));
        columnWidths[varName] = width;
    }
}

/**
 * Format a numeric value for fixed-width column output
 * Handles different value ranges and formats appropriately
 */
std::string Simulator::formatValue(float value, int width) {
    std::ostringstream oss;
    oss.precision(4);
    
    // Handle missing/error values
    if (value == -99.0f || value == -99) {
        oss << std::setw(width) << std::right << "-99";
        return oss.str();
    }
    
    // Handle zero
    if (value == 0.0f) {
        oss << std::setw(width) << std::right << std::fixed << std::setprecision(2) << 0.0;
        return oss.str();
    }
    
    // For small values, use scientific notation
    if (std::abs(value) < 0.001f) {
        oss << std::setw(width) << std::right << std::scientific << std::setprecision(3) << value;
    } else if (std::abs(value) < 10.0f) {
        oss << std::setw(width) << std::right << std::fixed << std::setprecision(3) << value;
    } else if (std::abs(value) < 1000.0f) {
        oss << std::setw(width) << std::right << std::fixed << std::setprecision(2) << value;
    } else {
        oss << std::setw(width) << std::right << std::fixed << std::setprecision(0) << value;
    }
    
    return oss.str();
}

/**
 * Rebuild the entire output file with current columns using buffered rows
 */
void Simulator::rebuildOutputFile() {
    std::cout << "[REBUILD] Rebuilding output file with " << columnOrder.size() << " columns" << std::endl;
    std::cout.flush();
    
    // Write header and all buffered rows
    outputFile.open(outputFileName, std::ios::out | std::ios::trunc);
    
    // Write DSSAT header section
    outputFile << "$GENERIC PEST MODEL OUTPUT FILE\n\n";
    outputFile << "*DSSAT Cropping System Model\n\n";
    outputFile << "*RUN            : GDM Output\n";
    outputFile << " MODEL          : GDM\n";
    outputFile << " DISEASE        : " << disease->getDiseaseID() << "\n";
    outputFile << " DATA PATH      :\n";
    outputFile << " TREATMENT      : GenericPM\n\n";
    
    // Write column header line with @ symbol and proper spacing
    outputFile << "@YEAR DOY";
    for (const auto& colName : columnOrder) {
        // Right-align column name within its width
        int colWidth = columnWidths[colName];
        outputFile << " " << std::setw(colWidth) << std::right << colName;
    }
    outputFile << "\n";
    
    outputFile.close();
    
    // Now write all buffered rows
    for (const auto& rowPair : bufferedRows) {
        int yearDoy = rowPair.first;
        const auto& outputs = rowPair.second;
        
        std::ostringstream row;
        row << std::setw(5) << std::right << (yearDoy / 1000) << " ";
        row << std::setw(3) << std::right << (yearDoy % 1000) << " ";
        
        for (const auto& colName : columnOrder) {
            int colWidth = columnWidths[colName];
            auto it = outputs.find(colName);
            if (it != outputs.end()) {
                row << formatValue(it->second, colWidth) << " ";
            } else {
                row << std::setw(colWidth) << std::right << std::fixed << std::setprecision(2) << 0.0 << " ";
            }
        }
        
        row << "\n";
        
        outputFile.open(outputFileName, std::ios::app);
        outputFile << row.str();
        outputFile.close();
    }
}

/**
 * Format and write an output row in DSSAT fixed-width format
 */
void Simulator::formatAndWriteOutputRow(int yearDoy, const std::map<std::string, float>& outputs, bool newColumnsDiscovered) { 
    if (outputs.empty()) {
        return;
    }
    
    // Update columnWidths for any columns
    for (const auto& pair : outputs) {
        if (columnWidths.find(pair.first) == columnWidths.end()) {
            int width = std::max(6, static_cast<int>(pair.first.length()));
            columnWidths[pair.first] = width;
        }
    }
    
    // If this is the first write, write header
    if (!headerWritten) {
        std::cout << "[FIRST WRITE] Writing initial header" << std::endl;
        std::cout.flush();
        writeOutputHeader();
        headerWritten = true;
    }
    // If new columns were discovered, rebuild the file with all buffered rows
    else if (newColumnsDiscovered) {
        std::cout << "[NEW COLUMNS DETECTED] Rebuilding file with new columns" << std::endl;
        std::cout.flush();
        rebuildOutputFile();
    }
    
    // Store this row in the buffer
    bufferedRows.push_back({yearDoy, outputs});
    
    // Write the current row to file
    std::ostringstream row;
    row << std::setw(5) << std::right << (yearDoy / 1000) << " ";
    row << std::setw(3) << std::right << (yearDoy % 1000) << " ";
    
    for (const auto& colName : columnOrder) {
        int colWidth = columnWidths[colName];
        auto it = outputs.find(colName);
        if (it != outputs.end()) {
            row << formatValue(it->second, colWidth) << " ";
        } else {
            row << std::setw(colWidth) << std::right << std::fixed << std::setprecision(2) << 0.0 << " ";
        }
    }
    
    row << "\n";
    
    outputFile.open(outputFileName, std::ios::app);
    outputFile << row.str();
    outputFile.close();
}

/**
 * Write DSSAT-style output header
 */
void Simulator::writeOutputHeader() {
    outputFile.open(outputFileName, std::ios::out | std::ios::trunc);
    
    // Write DSSAT header section
    outputFile << "$GENERIC PEST MODEL OUTPUT FILE\n\n";
    outputFile << "*DSSAT Cropping System Model\n\n";
    outputFile << "*RUN            : GDM Output\n";
    outputFile << " MODEL          : GDM\n";
    outputFile << " DISEASE        : " << disease->getDiseaseID() << "\n";
    outputFile << " DATA PATH      :\n";
    outputFile << " TREATMENT      : GenericPM\n\n";
    
    // Write column header line with @ symbol and proper spacing
    outputFile << "@YEAR DOY";
    for (const auto& colName : columnOrder) {
        // Right-align column name within its width
        int colWidth = columnWidths[colName];
        outputFile << " " << std::setw(colWidth) << std::right << colName;
    }
    outputFile << "\n";
    
    outputFile.close();
}

/**
 * Simulator rate function
 * 
 * For each of the initial conditions and each of the plants in the simulation, call their 
 * rate functions. These calls propagate downwards (e.g. to organs).
 */
void Simulator::rate() {
#if GENERICPM_DEBUG_ENABLED
    std::cerr << "[SIM] Simulator::rate() called" << std::endl << std::flush;
#endif
    CouplingData *couplingData = CouplingData::getInstance(); 

    InitialCondition *ic;
    FlexibleIO *fio = FlexibleIO::getInstance();

    gEqContext->disease = this->disease;

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

        plants[0].getCloudsP()[0].getCloudF()->addInoculumCreated(CloudField);
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
            // this->logOutput("NEW_GROWTH_" + cpIDToStr(organCP), dssatDelta);
        } else {
            // Some amount of senescence has occurred. This is independent of disease, so damage decreases healthy value. 
            // When in COHORT mode, the senescence should be applied proportional to healthy cohort value.
            // When in SINGULAR mode, damage can be applied directly. 
            Plant::getInstance()->getOrganSet(organCP).senescenceQueue -= dssatDelta;
        }

        // NOTE: The disease model should explicitly log any CP_* values it needs
        // via OUTPUT integration endpoints. We don't output all coupling points
        // to avoid garbage values from uninitialized coupling pointers.

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
        //     }
        //     if (disease->getOrganMode() == OrganMode::COHORT) {
        //         cropinterface->setOrganArea(newOrgan, (*organCPVal - organCPValPrev));
        //     } else if (disease->getOrganMode() == OrganMode::SINGULAR) {
        //         Plant::getInstance()->getOrganSet(organCP).queueHealthyGrowth((*organCPVal - organCPValPrev));
        //     }   
    }

    /** For each Initial Condition call the rate function */
    initialCondition->rate();

    // Orchestrate rate calls for all CloudOs, then CloudPs, then CloudFs.
    Plant* plant = getPlant();
    for (auto& OrganSet : plant->getOrgans()) {
        for (auto& organ : OrganSet.organs) {
            for (auto& cloudO : organ.getCloudsO()) {
                if (cloudO->getDisease() == this->disease) {
                    cloudO->rate();
                }
            }
        }
    }
    for (auto& cloudP : plant->getCloudsP()) {
        if (cloudP->getDisease() == this->disease) {
            cloudP->rate();
        }
    }
    this->cloudF->rate();

    float destination = -99.0f;
    try {
        destination = static_cast<float>(disease->resolveInoculumDestination());
#if GENERICPM_DEBUG_ENABLED
        std::cerr << "[SIM] INOC_DEST resolved to: " << destination << " (original expression: " << disease->getINOC_DEST().getOriginal() << ")" << std::endl << std::flush;
#endif
    } catch (const std::runtime_error& e) {
#if GENERICPM_DEBUG_ENABLED
        std::cerr << "Error resolving INOC_DEST expression for DiseaseID: " << disease->getDiseaseID() << std::endl << "Exception: " << e.what() << std::endl;
#endif
        throw e;
    }

    try {
#if GENERICPM_DEBUG_ENABLED
        std::cerr << "[SIM] About to evaluate INOC_EXT for DiseaseID: " << disease->getDiseaseID() << std::endl << std::flush;
#endif
        float inocExt = disease->getINOC_EXT()->evaluate();
#if GENERICPM_DEBUG_ENABLED
        std::cerr << "[SIM] INOC_EXT evaluated to: " << inocExt << std::endl << std::flush;
        std::cerr << "[SIM] Cloud object: " << initialCondition.getCloud() << " (family: " << initialCondition.getCloud()->getFamily() << ")" << std::endl << std::flush;
#endif
        this->cloudF->addInoculumCreated(inocExt, destination);
#if GENERICPM_DEBUG_ENABLED
        std::cerr << "[SIM] After addInoculumCreated, cloud value: " << this->cloudF->getValue() << std::endl << std::flush;
#endif
    } catch (const std::runtime_error& e) {
        std::cerr << "Error evaluating INOC_EXT expression for DiseaseID: " << disease->getDiseaseID() << std::endl << "Exception: " << e.what() << std::endl;
        // Default to 0 inoculum if evaluation fails
        this->cloudF->addInoculumCreated(0.0f);
    }

    gEqContext->disease = nullptr;
}


void Simulator::integration() {
    // Declare helper values for integration 
    float diseaseValue = 0, totalValue=0, sArea=0;
    int seedAge = 0;

    gEqContext->disease = this->disease;

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

    initialCondition->integration(disease);

    Plant* plant = getPlant();
    plant->integration();

    // Orchestrate rate calls for all CloudOs, then CloudPs, then CloudF.
    //  CloudOs
    for (auto& OrganSet : plant->getOrgans()) {
        for (auto& organ : OrganSet.organs) {
            for (auto& cloudO : organ.getCloudsO()) {
                if (cloudO->getDisease() == this->disease) {
                    cloudO->integration();
                }
            }
        }
    }

    //  CloudPs
    for (auto& cloudP : plant->getCloudsP()) {
        if (cloudP->getDisease() == this->disease) {
            cloudP->integration();
        }
    }

    // CloudF
    initialCondition->getCloud()->integration();

    // NOTE: Original plant integration placement
    // plant->integration();

    if (getPlant() != nullptr && getPlant()->getOrgans().size() > 0) {
        // NOTE: Why is plant even a context-dependent set of expressions? For damage, the functions 
        //       should return the value corresponding to that CP. Try going through the OrganSets as
        //       context! 
        gEqContext->plant = plant;
        // NOTE: Testing out the use of daily disease value
        // float diseaseDailyValue = getPlant()->getDailyDiseaseValue();

        // Previous damageCP hardcoded expression
        // dArea = getPlant()->getDiseaseValue();
        // tArea = getPlant()->getTotalValue();
        // seedAge = getPlant()->getOrgans().size();
        // pDArea = (dArea/(tArea-sArea)*100);

        if (plant->getTotalValue() > 0) {
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

    if (disease->getDEBUG() != Expression("-99.0")) {
        FlexibleIO *fio = FlexibleIO::getInstance();
        std::cout << "DEBUG for " << fio->getInteger("CONTROL", "YRDOY") << 
        ":\n\tOriginal Expression: " << disease->getDEBUG().getOriginal() << 
        "\n\tTranslated Expr:     " << disease->getDEBUG().getTranslated() << 
        "\n\tEvaluated Expr:      " << disease->getDEBUG().evaluate() << std::endl;
    }
    gEqContext->plant = nullptr;
    gEqContext->disease = nullptr;
}

void Simulator::output() {
    gEqContext->disease = this->disease;
    gEqContext->plant = getPlant();

    std::cout.flush();
    
    // Track column count before processing this output
    int columnCountBefore = columnOrder.size();
    
    initialCondition->output();
    getPlant()->output();

    // Always write output row for every day of simulation
    float outputVal;
    std::string outputVarName;

    // First, compute all specifically "OUTPUT" step injections from disease model
    for (auto& injection : disease->getIntegrationInjections()) {
        if (injection.getEndpoint() == InjEndpoint::OUTPUT) {
            injection.apply(outputVal, outputVarName);
            logOutput(outputVarName, outputVal);
            std::cout << "[OUTPUT] Disease injection: " << outputVarName << "=" << outputVal << std::endl;
            std::cout.flush();
        }
    }

    // Initialize all known columns to 0.0 first to avoid garbage values
    for (const auto& colName : columnOrder) {
        if (currentDayOutputs.find(colName) == currentDayOutputs.end()) {
            currentDayOutputs[colName] = 0.0f;
        }
    }

    // Accumulate disease-specific outputs for the current day
    for (const auto& output : loggedOutputs) {
        currentDayOutputs[output.varName] = output.value;
        
        // Track this column if we haven't seen it before
        if (std::find(columnOrder.begin(), columnOrder.end(), output.varName) == columnOrder.end()) {
            columnOrder.push_back(output.varName);
        }
    }

    // Add standard daily metrics (these are output every day for every simulator)
    float organCP = getOrganCPValue();
    float damageValue = getEvaluatedDamageValue();
    float invisibleArea = getPlantInvisibleDiseaseArea();
    float visibleArea = getPlantVisibleDiseaseArea();
    float totalLesions = getPlantTotalLesionNumber();
    Inoculum simulatorInoculum = getTotalInoculum();
    float familyInoculum = simulatorInoculum.familyAmount;
    float diseaseInoculum = simulatorInoculum.diseaseAmount;

    // Debug output to command line - VERY VISIBLE
    // std::cout << "[SIMULATOR METRICS] DiseaseID=" << disease->getDiseaseID() 
    //           << " organCP=" << organCP 
    //           << " damageValue=" << damageValue 
    //           << " invisibleArea=" << invisibleArea 
    //           << " visibleArea=" << visibleArea 
    //           << " totalLesions=" << totalLesions 
    //           << " familyInoculum=" << familyInoculum 
    //           << " diseaseInoculum=" << diseaseInoculum << std::endl;
    // std::cout.flush();

    // Log standard metrics with disease-specific names to avoid collisions
    std::string diseaseID = disease->getDiseaseID();
    logOutput("ORGAN_CP_" + diseaseID, organCP);
    logOutput("DAMAGE_" + diseaseID, damageValue);
    logOutput("INVIS_AREA_" + diseaseID, invisibleArea);
    logOutput("VIS_AREA_" + diseaseID, visibleArea);
    logOutput("TOT_LESIONS_" + diseaseID, totalLesions);
    logOutput("FAM_INOC_" + diseaseID, familyInoculum);
    logOutput("TOT_INOC_" + diseaseID, diseaseInoculum);

    // Add all logged outputs to the output map - directly assign, don't skip if exists
    for (const auto& output : loggedOutputs) {
        currentDayOutputs[output.varName] = output.value;
        // Track this column if we haven't seen it before
        if (std::find(columnOrder.begin(), columnOrder.end(), output.varName) == columnOrder.end()) {
            columnOrder.push_back(output.varName);
        }
    }

    // Get current date and format output row
    int currentYearDoy = Manager::getInstance()->getCurrentSimDate();
    
    // Check if new columns were discovered during this output
    int columnCountAfter = columnOrder.size();
    bool newColumnsDiscovered = (columnCountAfter > columnCountBefore);
    
    formatAndWriteOutputRow(currentYearDoy, currentDayOutputs, newColumnsDiscovered);

    // Clear accumulated outputs for the next day
    currentDayOutputs.clear();
    clearOutputLog();

    gEqContext->disease = nullptr;
    gEqContext->plant = nullptr;
}

/**
 * Get the current organ coupling point value
 */
float Simulator::getOrganCPValue() {
    CouplingPointID organCP = disease->getOrganCP();
    if (organCP != CouplingPointID::VALUE) {
        CouplingData *couplingData = CouplingData::getInstance();
        float *value = couplingData->getCouplingValue(organCP);
        if (value != nullptr) {
            return *value;
        }
    }
    return -99.0f;
}

/**
 * Get the evaluated damage value for this day
 */
float Simulator::getEvaluatedDamageValue() {
    try {
        return disease->getDAMAGE()->evaluate();
    } catch (const std::exception& e) {
#if GENERICPM_DEBUG_ENABLED
        std::cerr << "Error evaluating damage value: " << e.what() << std::endl;
#endif
        return -99.0f;
    }
    return -99.0f;
}

/**
 * Get plant invisible disease area (latent infections)
 */
float Simulator::getPlantInvisibleDiseaseArea() {
    Plant *plant = getPlant();
    if (plant != nullptr) {
        return plant->getInvisibleValue(this->disease);
    }
    return 0.0f;
}

/**
 * Get plant visible disease area (symptomatic infections)
 */
float Simulator::getPlantVisibleDiseaseArea() {
    Plant *plant = getPlant();
    if (plant != nullptr) {
        return plant->getVisibleValue(this->disease);
    }
    return 0.0f;
}

/**
 * Get total number of lesions on plant for this disease
 */
float Simulator::getPlantTotalLesionNumber() {
    Plant *plant = getPlant();
    if (plant != nullptr) {
        return plant->getTotalLesions();
    }
    return 0.0f;
}

/**
 * Get total inoculum for the disease managed by this simulator
 */
Inoculum Simulator::getTotalInoculum() {
    // 1) Search all CloudsF
    // 2) Search all CloudsP
    // 3) Search all CloudsO
    float thisDisease = 0.0f;
    float thisFamily = 0.0f;

    // Search this disease CloudF
    thisDisease += this->cloudF->getValue();
    thisFamily += thisDisease;

    // Search other disease CloudFs for this family
    for (const auto& sim : Manager::getInstance()->getSimulators()) {
        if (sim->getDisease()->getFamily() == this->disease->getFamily() && sim->getDisease() != this->disease) {
            thisFamily += sim->cloudF->getValue();
        }
    }

    // Search CloudP for this disease
    thisDisease += this->getPlant()->getCloudP(this->disease)->getValue();
    thisFamily += thisDisease - thisFamily;

    // Search other disease CloudPs for this family
    for (auto& cloudP : this->getPlant()->getCloudsP()) {
        if (cloudP->getDisease()->getFamily() == this->disease->getFamily() && cloudP->getDisease() != this->disease) {
            thisFamily += cloudP->getValue();
        }
    }

    // Search CloudO for this disease + this family
    for (auto& organ : this->getPlant()->getOrganSet(this->disease->getOrganCP()).organs) {
        for (auto& cloudO : organ.getCloudsO()) {
            if (cloudO->getDisease() == this->disease) {
                thisDisease += cloudO->getValue();
                thisFamily += cloudO->getValue();
            } else if (cloudO->getDisease()->getFamily() == this->disease->getFamily()) {
                thisFamily += cloudO->getValue();
            }
        }
    }

    return Inoculum(thisDisease, thisFamily);
}