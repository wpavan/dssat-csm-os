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

bool LesionCohort::isLatentPeriod() const {
    Disease *disease = cloudo->getDisease();
    if (getPhysiologicalDaysAcumm() <= disease->getLatentPeriod()) {
        return true;
    }
    return false;
}

bool LesionCohort::isInfectionPeriod() const {
    Disease *disease = cloudo->getDisease();
    if (getPhysiologicalDaysAcumm() > disease->getLatentPeriod() && getPhysiologicalDaysAcumm() <= (disease->getLatentPeriod() + disease->getInfectionPeriod())) {
        return true;
    }
    return false;
}

bool LesionCohort::isNecroticPeriod() const {
    Disease *disease = cloudo->getDisease();
    if (getPhysiologicalDaysAcumm() > (disease->getLatentPeriod() + disease->getInfectionPeriod())) {
        return true;
    }
    return false;
}

static double TE_isLatentPeriod(void) {
    return gEqContext && gEqContext->lesionCohort && gEqContext->lesionCohort->isLatentPeriod() ? 1.0 : 0.0;
}

static double TE_isInfectionPeriod(void) {
    return gEqContext && gEqContext->lesionCohort && gEqContext->lesionCohort->isInfectionPeriod() ? 1.0 : 0.0;
}

static double TE_isNecroticPeriod(void) {
    return gEqContext && gEqContext->lesionCohort && gEqContext->lesionCohort->isNecroticPeriod() ? 1.0 : 0.0;
}

static double TE_getAge(void) {
    return gEqContext && gEqContext->lesionCohort ? static_cast<double>(gEqContext->lesionCohort->getAge()) : 0.0;
}

static double TE_getValue(void) {
    return gEqContext && gEqContext->lesionCohort ? static_cast<double>(gEqContext->lesionCohort->getTotalValue()) : 0.0;
}

static double TE_getVisibleValue(void) {
    return gEqContext && gEqContext->lesionCohort ? static_cast<double>(gEqContext->lesionCohort->getVisibleValue()) : 0.0;
}

static double TE_getInvisibleValue(void) {
    return gEqContext && gEqContext->lesionCohort ? static_cast<double>(gEqContext->lesionCohort->getInvisibleValue()) : 0.0;
}

namespace {
    struct FunctionRegistrar {
        FunctionRegistrar() {
            getCustomFunctions().register_context_function({"IS_LATENT_PERIOD", TE_isLatentPeriod});
            getCustomFunctions().register_context_function({"IS_INFECTION_PERIOD", TE_isInfectionPeriod});
            getCustomFunctions().register_context_function({"IS_NECROTIC_PERIOD", TE_isNecroticPeriod});
            getCustomFunctions().register_context_function({"LC_AGE", TE_getAge});
            getCustomFunctions().register_context_function({"LC_VALUE", TE_getValue});
            getCustomFunctions().register_context_function({"LC_VISIBLE_VALUE", TE_getVisibleValue});
            getCustomFunctions().register_context_function({"LC_INVISIBLE_VALUE", TE_getInvisibleValue});
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
    } catch (const std::runtime_error& e) {
        std::cerr << "Error evaluating LES_AGE expression for DiseaseID: " << disease->getDiseaseID() << std::endl << "Exception: " << e.what() << std::endl;
        dailyAge = 0.0f; // Default to 0 favorability if evaluation fails
    }
    
    // Use VGF & IGF to determine daily growth of lesion cohort
    try {
        dailyVisibleValue = disease->getVGF()->evaluate() * lesionsInThisCohort;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error evaluating VGF expression for DiseaseID: " << disease->getDiseaseID() << std::endl << "Exception: " << e.what() << std::endl;
        dailyVisibleValue = 0.0f; // Default to 0 favorability if evaluation fails
    }
    try {
        dailyInvisibleValue = disease->getIGF()->evaluate() * lesionsInThisCohort;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error evaluating IGF expression for DiseaseID: " << disease->getDiseaseID() << std::endl << "Exception: " << e.what() << std::endl;
        dailyInvisibleValue = 0.0f; // Default to 0 favorability if evaluation fails
    }
    
    // Use INOC_LES to determine new spores created by this lesion cohort
    try {
        newSpores = disease->getINOC_LES()->evaluate() * lesionsInThisCohort;
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
            cloudo->addInoculumCreated(newSpores);
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
    if (isInfectionPeriod() || isNecroticPeriod()){
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
