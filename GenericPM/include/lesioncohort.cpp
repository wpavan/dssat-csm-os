/**
 * @file lesioncohort.cpp
 * @brief LesionCohort class source file
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "lesioncohort.h"
#include "cropinterface.h"
#include "equation_context.h"
#include "project_config.h"

#include <string>
#include <sstream>
#include <iostream>

int LesionCohort::qtd = 0;

bool LesionCohort::isPhase1() const {
    Disease *disease = cloudo->getDisease();
    if (getPhysiologicalDaysAcumm() <= disease->getP1_DUR()) {
        return true;
    }
    return false;
}

bool LesionCohort::isPhase2() const {
    Disease *disease = cloudo->getDisease();
    if (getPhysiologicalDaysAcumm() > disease->getP1_DUR() && getPhysiologicalDaysAcumm() <= (disease->getP1_DUR() + disease->getP2_DUR())) {
        return true;
    }
    return false;
}

bool LesionCohort::isPhase3() const {
    Disease *disease = cloudo->getDisease();
    if (getPhysiologicalDaysAcumm() > (disease->getP1_DUR() + disease->getP2_DUR())) {
        return true;
    }
    return false;
}

static double TE_isPhase1(void) {
    if (gEqContext) {
        if (gEqContext->lesionCohort) {
            return gEqContext->lesionCohort->isPhase1() ? 1.0 : 0.0;
        } else {
            std::cerr << "Warning: TE_isPhase1 called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    } else {
        std::cerr << "gEqContext lost" << std::endl;
        return 0.0;
    }
}

static double TE_isPhase2(void) {
    if (gEqContext) {
        if (gEqContext->lesionCohort) {
            return gEqContext->lesionCohort->isPhase2() ? 1.0 : 0.0;
        } else {
            std::cerr << "Warning: TE_isPhase2 called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    } else {
        std::cerr << "gEqContext lost" << std::endl;
        return 0.0;
    }
}

static double TE_isPhase3(void) {
    if (gEqContext) {
        if (gEqContext->lesionCohort) {
            return gEqContext->lesionCohort->isPhase3() ? 1.0 : 0.0;
        } else {
            std::cerr << "Warning: TE_isPhase3 called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    } else {
        std::cerr << "gEqContext lost" << std::endl;
        return 0.0;
    }
}

static double TE_getAge(void) {
    if (gEqContext) {
        if (gEqContext->lesionCohort) {
            return static_cast<double>(gEqContext->lesionCohort->getAge());
        } else {
            std::cerr << "Warning: TE_getAge called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    }
}

static double TE_getValue(void) {
    if (gEqContext) {
        if (gEqContext->lesionCohort) {
            return static_cast<double>(gEqContext->lesionCohort->getTotalValue());
        } else {
            std::cerr << "Warning: TE_getValue called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    }
}

static double TE_getVisibleValue(void) {
    if (gEqContext) {
        if (gEqContext->lesionCohort) {
            double visibleValue = static_cast<double>(gEqContext->lesionCohort->getVisibleValue());
            // std::cerr << "[DEBUG] TE_getVisibleValue: visibleValue=" << visibleValue << std::endl;
            return visibleValue;
        } else {
            std::cerr << "Warning: TE_getVisibleValue called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    } else {
        std::cerr << "Warning: TE_getVisibleValue called with null gEqContext." << std::endl;
        return 0.0;
    }
}

static double TE_getInvisibleValue(void) {
    if (gEqContext) {
        if (gEqContext->lesionCohort) {
            double invisibleValue = static_cast<double>(gEqContext->lesionCohort->getInvisibleValue());
            // std::cerr << "[DEBUG] TE_getInvisibleValue: invisibleValue=" << invisibleValue << std::endl;
            return invisibleValue;
        } else {
            std::cerr << "Warning: TE_getInvisibleValue called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    } else {
        std::cerr << "Warning: TE_getInvisibleValue called with null gEqContext." << std::endl;
        return 0.0;
    }
}

static double TE_getLCMaximumValue(void) {
    if (gEqContext) {
        if (gEqContext->lesionCohort) {
            // Get LC disease value
            double lesionDisease = gEqContext->lesionCohort->getDiseaseValue();

            // Get Organ healthy and disease values
            double organDiseaseValue = gEqContext->lesionCohort->getOrganDiseaseValue();
            double organHealthyValue = gEqContext->lesionCohort->getOrganHealthyValue();
            
            if (organDiseaseValue == 0.0) {
                std::cerr << "  WARNING: organDiseaseValue is ZERO - division by zero will occur!" << std::endl;
            }
            
            double diseaseRatio = (organDiseaseValue != 0.0) ? (lesionDisease / organDiseaseValue) : 0.0;
            
            double result = diseaseRatio * organHealthyValue;
            
            // std::cout << "Calculating LC Maximum Value:\n\tLC Total Value: "<< totalValue 
            //           << "\n\tLC Organ Disease Pct: " << (organDiseaseValue != 0.0 ? diseaseRatio * 100 : 0.0) << "%"
            //           << "\n\tAllocation: " << result
            //           << std::endl; 
            
            return static_cast<double>(result);
        } else {
            std::cerr << "Warning: TE_getLCMaximumValue called outside of the appropriate context." << std::endl;
            return 0.0;
        }
    } else {
        std::cerr << "Warning: TE_getLCMaximumValue called with null gEqContext." << std::endl;
        return 0.0;
    }
}

namespace {
    struct FunctionRegistrar {
        FunctionRegistrar() {
            getCustomFunctions().register_context_function({"IS_LATENT_PERIOD", TE_isPhase1});
            getCustomFunctions().register_context_function({"IS_INFECTION_PERIOD", TE_isPhase2});
            getCustomFunctions().register_context_function({"IS_NECROTIC_PERIOD", TE_isPhase3});
            getCustomFunctions().register_context_function({"IS_PHASE1", TE_isPhase1});
            getCustomFunctions().register_context_function({"IS_PHASE2", TE_isPhase2});
            getCustomFunctions().register_context_function({"IS_PHASE3", TE_isPhase3});
            getCustomFunctions().register_context_function({"LC_AGE", TE_getAge});
            getCustomFunctions().register_context_function({"LC_VALUE", TE_getValue});
            getCustomFunctions().register_context_function({"LC_VISIBLE_VALUE", TE_getVisibleValue});
            getCustomFunctions().register_context_function({"LC_INVISIBLE_VALUE", TE_getInvisibleValue});
            getCustomFunctions().register_context_function({"LC_MAX_VALUE", TE_getLCMaximumValue});
        }
    };

    // Static instance to trigger the registration at program startup
    static FunctionRegistrar registrar;
}

void LesionCohort::rate() {
    // Point global context to the current lesion cohort
    gEqContext->lesionCohort = this;

    Disease *disease = cloudo->getDisease();

    // Determination of daily changes
    // Use the LES_AGE function expression to determine daily age increase for lesions
    try {
        dailyAge = disease->getLES_AGE()->evaluate();
        // std::cout << "Evaluated LES_AGE: " << dailyAge << "\tTotal will be: " << physiologicalAge + dailyAge << std::endl;

    } catch (const std::runtime_error& e) {
        std::cerr << "Error evaluating LES_AGE expression for DiseaseID: " << disease->getDiseaseID() << std::endl << "Exception: " << e.what() << std::endl;
        dailyAge = 0.0f; // Default to 0 favorability if evaluation fails
    }
    
    // Use VGF & IGF to determine daily growth of lesion cohort
    try {
        dailyInvisibleValue = disease->getIGF()->evaluate() * lesionsInThisCohort;
        // std::cout << "Evaluated IGF: " << dailyInvisibleValue << "\tTotal will be: " << invisibleValue + dailyInvisibleValue << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error evaluating IGF expression for DiseaseID: " << disease->getDiseaseID() << std::endl << "===Exception===\n" << e.what() << "\n===End Exception==="<< std::endl;
        dailyInvisibleValue = 0.0f; // Default to 0 favorability if evaluation fails
    }
    try {
        dailyVisibleValue = disease->getVGF()->evaluate() * lesionsInThisCohort;
        // std::cout << "Evaluated VGF: " << dailyVisibleValue << "\tTotal will be: " << visibleValue + dailyVisibleValue << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error evaluating VGF expression for DiseaseID: " << disease->getDiseaseID() << std::endl << "Exception: " << e.what() << std::endl;
        dailyVisibleValue = 0.0f; // Default to 0 favorability if evaluation fails
    }
    
    // Use INOC_LES to determine new spores created by this lesion cohort
    try {
        newSpores = disease->getINOC_LES()->evaluate() * lesionsInThisCohort;
        // std::cout << "Evaluated INOC_LES: " << newSpores << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error evaluating INOC_LES expression for DiseaseID: " << disease->getDiseaseID() << std::endl << "Exception: " << e.what() << std::endl;
        newSpores = 0.0f; // Default to 0 favorability if evaluation fails
    }

    // Dereference of global context to avoid accidental misuse
    gEqContext->lesionCohort = nullptr;
}

void LesionCohort::integration() {
    Disease *disease = cloudo->getDisease();

    if (getOrganHealthyValue() > 0) {      
        if(dailyInvisibleValue > 0) {
            invisibleValue += dailyInvisibleValue;
        }

        if(dailyVisibleValue > 0) {
            visibleValue += dailyVisibleValue;
        }

        if(newSpores > 0) {
            cloudo->addInoculumCreated(newSpores, disease->getINOC_DEST().evaluate());
        }        

        physiologicalAge += dailyAge;

        std::ostringstream convert;
        convert << FlexibleIO::getInstance()->getReal("CONTROL", "YEARDOY") << "," << getTotalValue() << "," << lesionsInThisCohort << "," << getPhysiologicalDaysAcumm() << ","
                << getOrganDiseasedValueProportion() << "," << getLatentValue() << "," << getInfectionValue() << "," << getNecroticValue() << ","
                << newSpores << "," << dailyVisibleValue << "," << dailyInvisibleValue;
        Basic::output.push_back(convert.str());

        newSpores=0;
    }
}

int LesionCohort::getVisibleLesions() {
    if (isPhase2() || isPhase3()){
        return this->lesionsInThisCohort;
    } else {
        return 0;
    }
}

void LesionCohort::output() {
    // std::ostringstream convert;
    // convert << "Cpp_LesionCohort_" << getID() << ".txt";
    // Basic::getOutput(convert.str());
}
