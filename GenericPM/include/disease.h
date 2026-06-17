/**
 * @file disease.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#ifndef DISEASE_H
#define DISEASE_H

#include "basic.h"
#include "utilities.h"
#include "coupling.h"
#include "injection.h"
#include "expression.h"

#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>


enum class InoculumDestination {
    DORMANT,    // Inoculum cannot infect until next season favorability reached.
    INFECTIVE   // Inoculum can infect immediately.
};

/**
 * @class Disease
 * 
 * @brief Disease is a subclass of Basic and is used to store disease parameters.
 */
class Disease : public Basic {
protected:
    int id;
    std::string cropModel = "CRGRO";
    std::string family = "RUST";
    std::string diseaseID = "SLR";
    CouplingPointID organCP = CouplingPointID::SDWT;
    CouplingPointID damageCP = CouplingPointID::PSDD; 

    // NOTE: Units for all of these values would be great to include in the documentation.
    float initialInoculum = 50;
    int vectorSizeCloudF = 5;
    int vectorSizeCloudP = 7;
    int vectorSizeCloudO = 10;
    // float dailySporeProductionPerLesion = 3000;
    float maxSporeCloudsDensity = 12000;
    float proportionFromOrganToPlantCloud = 0.20;
    float proportionFromPlantToFieldCloud = 0.30;   
    
    std::string depositionFrequency = "0.367753*(x+0.001)^0.129605*exp(-0.085252*(x+0.001))";
    
    
    float acumulateFavorability = 35;
    static std::vector<Disease*> listDiseases;
    float biologicalFactor = 0.0;

    OrganMode organMode = OrganMode::COHORT;

    bool createdSpores = false;

    /**
     * @var cardinalTempPhysiologicalLife
     * 
     * Set of temperatures in Celsius used to calculate the physiological life of an
     * organ (an analog of growing degree days). This is used by Utilities::TrapezoidalFunction.
     */
    float cardinalTempPhysiologicalLife[4] = {0, 28, 30, 40};

    std::vector<Injection> rateInjections, integInjections, outputInjections;

    Expression INOC_EXT; // Expression for external inoculum amount
    Expression INOC_LES; // Expression for lesion-based inoculum production
    Expression INOC_REM; // Expression for daily proportional removal of inoculum
    Expression INOC_DEST; // Destination for inoculum produced by the disease (dormant or infective)
    bool INOC_AGE; // Indicates whether this disease is set to track inoculum age (tipping bucket) for removal purposes.

    Expression DAMAGE; // Expression for organ damage due to this disease

    Expression II_AGE; // Expression for how quickly favorability accumulates for initial spore release

    Expression LES_AGE; // Expression for how quickly lesions increase physiological age
    Expression NEW_LES; // Expression for how many new lesions are generated daily

    Expression VGF; // Expression for visible growth function
    Expression IGF; // Expression for invisible growth function

    Expression INIT_LES; // Initial lesion size in unit of ORGAN_CP; used as the starting point for lesion growth

    // A phase duration of -1 means that the phase is perpetual.
    int P1_DUR = 7; // Duration of phase 1 of lesions development (usually latent); in units of LES_AGE
    int P2_DUR = 21; // Duration of phase 2 of lesions development (usually infectious); in units of LES_AGE

    Expression DEBUG_EXPR; // Expression evaluated and printed at the end of each integration step

public:
    Disease() {
        listDiseases.push_back(this);
    }

    ~Disease() {
        std::cout << "Disease destructor called for " << this << " family: " << family << std::endl;
    }

    static std::vector<Disease*>& getDisease() {
        return listDiseases;
    }

    void printDisease() {
        std::cout << "Disease ID: " << diseaseID << std::endl;
        std::cout << "Initial Inoculum: " << initialInoculum << std::endl;
        std::cout << "Vector Size Cloud F: " << vectorSizeCloudF << std::endl;
        std::cout << "Vector Size Cloud P: " << vectorSizeCloudP << std::endl;
        std::cout << "Vector Size Cloud O: " << vectorSizeCloudO << std::endl;
        std::cout << "Max Spore Clouds Density: " << maxSporeCloudsDensity << std::endl;
        std::cout << "Proportion From Organ To Plant Cloud: " << proportionFromOrganToPlantCloud << std::endl;
        std::cout << "Proportion From Plant To Field Cloud: " << proportionFromPlantToFieldCloud << std::endl;
        std::cout << "Latent Period: " << P1_DUR << std::endl;
        std::cout << "Infection Period: " << P2_DUR << std::endl;
        std::cout << "Deposition Frequency: " << depositionFrequency << std::endl;
        std::cout << "Initial Lesion Size: " << INIT_LES.getOriginal() << std::endl;
        std::cout << "Acumulate Favorability: " << acumulateFavorability << std::endl;
        std::cout << "Cardinal Temp Physiological Life: " << cardinalTempPhysiologicalLife[0] << " " << cardinalTempPhysiologicalLife[1] << " " << cardinalTempPhysiologicalLife[2] << " " << cardinalTempPhysiologicalLife[3] << std::endl;
    }

    OrganMode getOrganMode() {
        return organMode;
    }

    void setOrganMode(OrganMode organMode) {
        this->organMode = organMode;
    }

    float getBiologicalFactor() {
        return biologicalFactor;
    }

    float* getBiologicalFactorRef() {
        return &biologicalFactor;
    }

    void setBiologicalFactor(float biologicalFactor) {
        this->biologicalFactor = biologicalFactor;
    }

    std::string getDiseaseID() {
        return diseaseID;
    } 
    
    void setDiseaseID(std::string diseaseID) {
        this->diseaseID = diseaseID;
    }

    std::string getFamily() {
        return family;
    }

    void setFamily(std::string family) {
        this->family = family;
    }

    CouplingPointID getOrganCP() {
        return organCP;
    }

    void setOrganCP(CouplingPointID organCouplingPoint) {
        this->organCP = organCouplingPoint;
    }

    CouplingPointID getDamageCP() {
        return damageCP;
    }

    std::vector<Injection>& getRateInjections() {
        return rateInjections;
    }

    void addRateInjection(Injection inj) {
        rateInjections.push_back(inj);
    }

    std::vector<Injection>& getIntegrationInjections() {
        return integInjections;
    }

    void addIntegrationInjection(Injection inj) {
        integInjections.push_back(inj);
    }

    std::vector<Injection>& getOutputInjections() {
        return outputInjections;
    }

    void addOutputInjection(Injection inj) {
        outputInjections.push_back(inj);
    }

    void setDamageCP(CouplingPointID damageCouplingPoint) {
        this->damageCP = damageCouplingPoint; // Default coupling point for the organ cloud.
    }

    /**
     * Get proportion of spores from organ to plant cloud.
     * 
     * @return Proportion of spores from organ to plant cloud.
     */
    float getProportionFromOrganToPlantCloud() {
        return proportionFromOrganToPlantCloud;
    }

    /**
     * Set proportion of spores from organ to plant cloud.
     * 
     * @param proportionFromOrganToPlantCloud Proportion of spores from organ to plant cloud.
     */
    void setProportionFromOrganToPlantCloud(float proportionFromOrganToPlantCloud) {
        this->proportionFromOrganToPlantCloud = proportionFromOrganToPlantCloud;
    }

    /**
     * Get proportion of spores from plant to field cloud.
     * 
     * @return Proportion of spores from plant to field cloud.
     */
    float getProportionFromPlantToFieldCloud() {
        return proportionFromPlantToFieldCloud;
    }

    /**
     * Set proportion of spores from plant to field cloud.
     * 
     * @param proportionFromPlantToFieldCloud Proportion of spores from plant to field cloud.
     */
    void setProportionFromPlantToFieldCloud(float proportionFromPlantToFieldCloud) {
        this->proportionFromPlantToFieldCloud = proportionFromPlantToFieldCloud;
    }

    /**
     * Get maximum spore clouds density.
     * 
     * @return Maximum spore clouds density.
     */
    float getMaxSporeCloudsDensity() {
        return maxSporeCloudsDensity;
    }

    /**
     * Set maximum spore clouds density.
     * 
     * @param maxSporeCloudsDensity Maximum spore clouds density.
     */
    void setMaxSporeCloudsDensity(float maxSporeCloudsDensity) {
        this->maxSporeCloudsDensity = maxSporeCloudsDensity;
    }

    /**
     * Get the disease ID.
     * 
     * @return Disease ID.
     */
    int getId() {
        return id;
    }

    /**
     * Set the disease ID.
     * 
     * @param id Disease ID.
     */
    void setId(int id) {
        this->id = id;
    }

    /**
     * Get the latent period.
     * 
     * @return Latent period.
     */
    int getP1_DUR() {
        return P1_DUR;
    }

    /**
     * Set the latent period.
     * 
     * @param latentPeriod Latent period.
     */
    void setP1_DUR(int P1_DUR) {
        this->P1_DUR = P1_DUR;
    }

    /**
     * Get the infection period.
     * 
     * @return Infection period.
     */
    int getP2_DUR() {
        return P2_DUR;
    }

    /**
     * Set the infection period.
     * 
     * @param infectionPeriod Infection period.
     */
    void setP2_DUR(int P2_DUR) {
        this->P2_DUR = P2_DUR;
    }

    /**
     * Get the vector size of cloud F.
     * 
     * @return Vector size of cloud F.
     */
    int getVectorSizeCloudF() {
        return vectorSizeCloudF;
    }

    /** 
     * Set the vector size of cloud F.
     * 
     * @param vectorSizeCloudF Vector size of cloud F.
     */
    void setVectorSizeCloudF(int vectorSizeCloudF) {
        this->vectorSizeCloudF = vectorSizeCloudF;
    }

    /** 
     * Get the vector size of cloud P.
     * 
     * @return Vector size of cloud P.
     */
    int getVectorSizeCloudP() {
        return vectorSizeCloudP;
    }
    
    /**
     * Set the vector size of cloud P.
     * 
     * @param vectorSizeCloudP Vector size of cloud P.
     */
    void setVectorSizeCloudP(int vectorSizeCloudP) {
        this->vectorSizeCloudP = vectorSizeCloudP;
    }

    /**
     * Get the vector size of cloud O.
     * 
     * @return Vector size of cloud O.
     */
    int getVectorSizeCloudO() {
        return vectorSizeCloudO;
    }

    /**
     * Set the vector size of cloud O.
     * 
     * @param vectorSizeCloudO Vector size of cloud O.
     */
    void setVectorSizeCloudO(int vectorSizeCloudO) {
        this->vectorSizeCloudO = vectorSizeCloudO;
    }

    /**
     * Get the initial Lesion size.
     * 
     * @return Initial Lesion size.
     */
    Expression getInitialLesionSize() {
        return INIT_LES;
    }

    /**
     * Set the initial Lesion size.
     * 
     * @param initialLesionSize Initial Lesion size.
     */
    void setInitialLesionSize(Expression initialLesionSize) {
        this->INIT_LES = initialLesionSize;
    }

    /** 
     * Get the initial inoculum.
     * 
     * @return Initial inoculum.
     */
    float getInitialInoculum() {
        return initialInoculum;
    }

    /**
     * Set the initial inoculum.
     * 
     * @param initialInoculum Initial inoculum.
     */
    void setInitialInoculum(float initialInoculum) {
        this->initialInoculum = initialInoculum;
    }

    /**
     * Get the acumulate favorability.
     * 
     * @return Acumulate favorability.
     */
    float getAcumulateFavorability() {
        return acumulateFavorability;
    }
    
    /**
     * Set the acumulate favorability.
     * 
     * @param acumulateFavorability Acumulate favorability.
     */
    void setAcumulateFavorability(float acumulateFavorability) {
        this->acumulateFavorability = acumulateFavorability;
    }
    
    /**
     * Get the cardinal temperature physiological life temperature vector.
     * 
     * @copydoc cardinalTempPhysiologicalLife
     * 
     * @return Cardinal temperature physiological life.
     */
    float* getCardinalTempPhysiologicalLife() {
        return &cardinalTempPhysiologicalLife[0];
    }

    /**
     * Set the cardinal temperature physiological life temperature vector.
     * 
     * @copydoc cardinalTempPhysiologicalLife
     * 
     * @param cardinalTempPhysiologicalLife Cardinal temperature physiological life vector (size = 4).
     */
    void setCardinalTempPhysiologicalLife(float cardinalTempPhysiologicalLife[]) {
        std::copy(cardinalTempPhysiologicalLife, cardinalTempPhysiologicalLife + 4, 
                  this->cardinalTempPhysiologicalLife);
    }

    /**
     * Get the visible growth function.
     * 
     * @return Visible growth function.
     */
    Expression* getVGF() {
        return &VGF;
    }
    
    /**
     * Set the visible growth function.
     * 
     * @param visibleGrowthFunction Visible growth function.
     */
    void setVGF(Expression visibleGrowthFunction) {
        this->VGF = visibleGrowthFunction;
    }

    /**
     * Get the invisible growth function.
     * 
     * @return Invisible growth function.
     */
    Expression* getIGF() {
        return &IGF;
    }
    
    /**
     * Set the invisible growth function.
     * 
     * @param invisibleGrowthFunction Invisible growth function.
     */
    void setIGF(Expression invisibleGrowthFunction) {
        this->IGF = invisibleGrowthFunction;
    }

    /**
     * Get the INOC_EXT expression.
     * 
     * @return INOC_EXT expression.
     */
    Expression* getINOC_EXT() {
        return &INOC_EXT;
    }

    /**
     * Set the INOC_EXT expression.
     * 
     * @param INOC_EXT INOC_EXT expression.
     */
    void setINOC_EXT(Expression INOC_EXT) {
        this->INOC_EXT = INOC_EXT;
    }

    /**
     * Get the INOC_LES expression.
     * 
     * @return INOC_LES expression.
     */
    Expression* getINOC_LES() {
        return &INOC_LES;
    }

    /**
     * Set the INOC_LES expression.
     * 
     * @param INOC_LES INOC_LES expression.
     */
    void setINOC_LES(Expression INOC_LES) {
        this->INOC_LES = INOC_LES;
    }

    bool getINOC_AGE() {
        return INOC_AGE;
    }

    void setINOC_AGE(bool INOC_AGE) {
        this->INOC_AGE = INOC_AGE;
    }

    /**
     * Get the DAMAGE expression.
     * 
     * @return DAMAGE expression.
     */
    Expression* getDAMAGE() {
        return &DAMAGE;
    }

    /**
     * Set the DAMAGE expression.
     * 
     * @param DAMAGE DAMAGE expression.
     */
    void setDAMAGE(Expression DAMAGE) {
        this->DAMAGE = DAMAGE;
    }

    /**
     * Get the II_AGE expression.
     * 
     * @return II_AGE expression.
     */
    Expression* getII_AGE() {
        return &II_AGE;
    }

    /**
     * Set the II_AGE expression.
     * 
     * @param II_AGE II_AGE expression.
     */
    void setII_AGE(Expression II_AGE) {
        this->II_AGE = II_AGE;
    }

    /**
     * Get the NEW_LES expression.
     * 
     * @return NEW_LES expression.
     */
    Expression* getNEW_LES() {
        return &NEW_LES;
    }

    /**
     * Set the NEW_LES expression.
     * 
     * @param NEW_LES NEW_LES expression.
     */
    void setNEW_LES(Expression NEW_LES) {
        this->NEW_LES = NEW_LES;
    }

    /**
     * Get the LES_AGE expression.
     * 
     * @return LES_AGE expression.
     */
    Expression* getLES_AGE() {
        return &LES_AGE;
    }

    /**
     * Set the LES_AGE expression.
     * 
     * @param LES_AGE LES_AGE expression.
     */
    void setLES_AGE(Expression LES_AGE) {
        this->LES_AGE = LES_AGE;
    }

    /**
     * Get the INOC_REM expression.
     * 
     * @return INOC_REM expression.
     */
    Expression* getINOC_REM() {
        return &INOC_REM;
    }

    /**
     * Set the INOC_REM expression.
     * 
     * @param INOC_REM INOC_REM expression.
     */
    void setINOC_REM(Expression INOC_REM) {
        this->INOC_REM = INOC_REM;
    }

    /**
     * Get the inoculum destination.
     * 
     * @return Inoculum destination.
     */
    Expression getINOC_DEST() {
        return INOC_DEST;
    }

    /**
     * Resolve inoculum destination from expression to enum.
     * Handles both static constants (INFECTIVE, DORMANT) and dynamic expressions.
     * 
     * @return InoculumDestination enum value (0=DORMANT, 1=INFECTIVE)
     */
    int resolveInoculumDestination() {
        std::string original = INOC_DEST.getOriginal();
        
        // Handle static constants first - exact string match
        if (original == "INFECTIVE") {
            return static_cast<int>(InoculumDestination::INFECTIVE);
        }
        if (original == "DORMANT") {
            return static_cast<int>(InoculumDestination::DORMANT);
        }
        
        // Handle dynamic expressions - pre-process to replace string constants
        // with numeric values, then evaluate
        std::string preprocessed = original;
        
        // Replace all occurrences of "INFECTIVE" with 1
        // Use word boundary replacement to avoid partial matches
        size_t pos = 0;
        while ((pos = preprocessed.find("INFECTIVE", pos)) != std::string::npos) {
            // Check if this is a whole word (not part of another identifier)
            bool isWholeWord = true;
            if (pos > 0 && std::isalnum(preprocessed[pos-1])) {
                isWholeWord = false;
            }
            if (pos + 9 < preprocessed.length() && std::isalnum(preprocessed[pos+9])) {
                isWholeWord = false;
            }
            
            if (isWholeWord) {
                preprocessed.replace(pos, 9, "1");
                pos += 1;
            } else {
                pos += 9;
            }
        }
        
        // Replace all occurrences of "DORMANT" with 0
        pos = 0;
        while ((pos = preprocessed.find("DORMANT", pos)) != std::string::npos) {
            // Check if this is a whole word (not part of another identifier)
            bool isWholeWord = true;
            if (pos > 0 && std::isalnum(preprocessed[pos-1])) {
                isWholeWord = false;
            }
            if (pos + 7 < preprocessed.length() && std::isalnum(preprocessed[pos+7])) {
                isWholeWord = false;
            }
            
            if (isWholeWord) {
                preprocessed.replace(pos, 7, "0");
                pos += 1;
            } else {
                pos += 7;
            }
        }
        
        // Evaluate the preprocessed expression
        Expression temp(preprocessed);
        float result = temp.evaluate();
        
        // Convert result to enum (0=DORMANT, 1=INFECTIVE)
        // Any non-zero value is treated as INFECTIVE
        return (result != 0.0f) ? static_cast<int>(InoculumDestination::INFECTIVE) : static_cast<int>(InoculumDestination::DORMANT);
    }

    /**
     * Set the inoculum destination.
     * 
     * @param INOC_DEST Inoculum destination.
     */
    void setINOC_DEST(Expression INOC_DEST) {
        this->INOC_DEST = BoundExpression(INOC_DEST, std::vector<float>{0.0f, 1.0f});
    }

    /**
     * Get the debug expression.
     * 
     * @return Debug expression.
     */
    Expression getDEBUG() {
        return DEBUG_EXPR;
    }

    /**
     * Set the debug expression.
     * 
     * @param DEBUG Debug expression.
     */
    void setDEBUG(Expression DEBUG_EXPR) {
        if (DEBUG_EXPR.getOriginal().empty()) {
            this->DEBUG_EXPR = Expression("-99.0"); // Default to a no-op expression if empty
        } else {
            this->DEBUG_EXPR = DEBUG_EXPR;
        }
    }
};

#endif // DISEASE_H
