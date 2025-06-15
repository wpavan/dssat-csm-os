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

#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

/**
 * @class Disease
 * 
 * @brief Disease is a subclass of Basic and is used to store disease parameters.
 */
class Disease : public Basic {
protected:
    int id;
    std::string cropModel = "CRGRO";
    std::string description = "Soybean Leaf Rust";
    std::string sporeModule = "GenericPM-Spores";

    // NOTE: Units for all of these values would be great to include in the documentation.
    float infectionEfficiency = 0.17;
    float initialInoculum = 50;
    int vectorSizeCloudF = 5;
    int vectorSizeCloudP = 7;
    int vectorSizeCloudO = 10;
    float dailySporeProductionPerLesion = 3000;
    float maxSporeCloudsDensity = 12000;
    float proportionFromOrganToPlantCloud = 0.20;
    float proportionFromPlantToFieldCloud = 0.30;   
    int latentPeriod = 7;
    int infectionPeriod = 21;
    int MRRS = 2000;
    std::string visibleGrowthFunction = "0.4*exp(-10*exp(-0.4*x))";
    std::string wetnessFunction = "1/(1+exp(4.948-0.348*x))";
    std::string depositionFrequency = "0.367753*(x+0.001)^0.129605*exp(-0.085252*(x+0.001))";
    float initialPustuleSize = 0.00001;
    std::string invisibleGrowthFunction = "0.8*exp(-10*exp(-0.4*x))";
    std::string SWF = "0.0000005*exp(0.20*x)";
    float wetnessThreshold = 6;
    float acumulateFavorability = 35;
    float hostFactor = 1;
    static std::vector<Disease*> listDiseases;
    std::string rhFactor = "1*x";
    std::string rateFunctionString = "";

    /**
     * @var cohortAgeSet
     * 
     * Set of factors related to the cohort age. These coefficients are
     * used as an input to the trapezoidal function for the calculation of 
     * new spores created by lesions.
     */ 
    float cohortAgeSet[4] = {4,20,22,30};

    /**
     * @var sporulationCrowdingFactorsSet
     * 
     * Set of coefficients used in an equation to calculate sporulation 
     * crowding factor. This factor is used in the calculation of new 
     * spores created by lesions.
     */
    float sporulationCrowdingFactorsSet[3] = {0.98669, 10.71894, 0.93374};

    /**
     * @var temperatureFavorabilitySet
     * 
     * Set of temperatures in Celsius used to calculate the favorability 
     * of the environment for the disease in terms of temperature. This is 
     * always used in Utilities::TemperatureFavorability.
     */
    float temperatureFavorabilitySet[3] = {29,10,22.5};

    /**
     * @var cardinalTempPhysiologicalLife
     * 
     * Set of temperatures in Celsius used to calculate the physiological life of an
     * organ (an analog of growing degree days). This is used by Utilities::TrapezoidalFunction.
     */
    float cardinalTempPhysiologicalLife[4] = {0, 28, 30, 40};

public:
    Disease() {
        listDiseases.push_back(this);
    }

    static std::vector<Disease*>& getDisease() {
        return listDiseases;
    }

    void printDisease() {
        std::cout << "Disease ID: " << id << std::endl;
        std::cout << "Description: " << description << std::endl;
        std::cout << "Spore Module: " << sporeModule << std::endl;
        std::cout << "Infection Efficiency: " << infectionEfficiency << std::endl;
        std::cout << "Initial Inoculum: " << initialInoculum << std::endl;
        std::cout << "Vector Size Cloud F: " << vectorSizeCloudF << std::endl;
        std::cout << "Vector Size Cloud P: " << vectorSizeCloudP << std::endl;
        std::cout << "Vector Size Cloud O: " << vectorSizeCloudO << std::endl;
        std::cout << "Daily Spore Production Per Lesion: " << dailySporeProductionPerLesion << std::endl;
        std::cout << "Cohort Age Set: " << cohortAgeSet[0] << " " << cohortAgeSet[1] << " " << cohortAgeSet[2] << " " << cohortAgeSet[3] << std::endl;
        std::cout << "Sporulation Crowding Factors Set: " << sporulationCrowdingFactorsSet[0] << " " << sporulationCrowdingFactorsSet[1] << " " << sporulationCrowdingFactorsSet[2] << std::endl;
        std::cout << "Max Spore Clouds Density: " << maxSporeCloudsDensity << std::endl;
        std::cout << "Proportion From Organ To Plant Cloud: " << proportionFromOrganToPlantCloud << std::endl;
        std::cout << "Proportion From Plant To Field Cloud: " << proportionFromPlantToFieldCloud << std::endl;
        std::cout << "Temperature Favorability Set: " << temperatureFavorabilitySet[0] << " " << temperatureFavorabilitySet[1] << " " << temperatureFavorabilitySet[2] << std::endl;
        std::cout << "Latent Period: " << latentPeriod << std::endl;
        std::cout << "Infection Period: " << infectionPeriod << std::endl;
        std::cout << "MRRS: " << MRRS << std::endl;
        std::cout << "Visible Growth Function: " << visibleGrowthFunction << std::endl;
        std::cout << "Wetness Function: " << wetnessFunction << std::endl;
        std::cout << "Deposition Frequency: " << depositionFrequency << std::endl;
        std::cout << "Initial Pustule Size: " << initialPustuleSize << std::endl;
        std::cout << "Invisible Growth Function: " << invisibleGrowthFunction << std::endl;
        std::cout << "Wetness Threshold: " << wetnessThreshold << std::endl;
        std::cout << "Acumulate Favorability: " << acumulateFavorability << std::endl;
        std::cout << "Host Factor: " << hostFactor << std::endl;
        std::cout << "Cardinal Temp Physiological Life: " << cardinalTempPhysiologicalLife[0] << " " << cardinalTempPhysiologicalLife[1] << " " << cardinalTempPhysiologicalLife[2] << " " << cardinalTempPhysiologicalLife[3] << std::endl;
        std::cout << "Rh Factor: " << rhFactor << std::endl;
    }

    std::string getSWF() {
        return SWF;
    }

    void setSWF(std::string SWF) {
        this->SWF = SWF;
    }

    std::string getSporeModule() {
        return sporeModule;
    }

    void setSporeModule(std::string sporeModule) {
        this->sporeModule = sporeModule;
    }

    float getSporulationCrowdingFactor(float proportionDiseaseArea);
    int newLesions(float cloudDensity, float healthyAreaProportion);

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
     * Get cohort age set.
     * 
     * @return Set of factors/coefficients related to the cohort age function.
     */
    float* getCohortAgeSet() {
        return &cohortAgeSet[0];
    }

    /**
     * Set cohort age set.
     * 
     * @param cohortAgeSet Set of factors/coefficients related to the cohort age function.
     */
    void setCohortAgeSet(float cohortAgeSet[]) {
        std::copy(cohortAgeSet, cohortAgeSet + 4, this->cohortAgeSet);
    }

    /**
     * Get daily spore production per lesion.
     * 
     * @return Daily spore production per lesion.
     */
    float getDailySporeProductionPerLesion() {
        return dailySporeProductionPerLesion;
    }

    /**
     * Set daily spore production per lesion.
     * 
     * @param dailySporeProductionPerLesion Daily spore production per lesion.
     */
    void setDailySporeProductionPerLesion(float dailySporeProductionPerLesion) {
        this->dailySporeProductionPerLesion = dailySporeProductionPerLesion;
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
     * Get the wetness threshold.
     * 
     * @return Wetness threshold.
     */
    float getWetnessThreshold() {
        return wetnessThreshold;
    }

    /**
     * Set the wetness threshold.
     * 
     * @param wetnessThreshold Wetness threshold.
     */
    void setWetnessThreshold(float wetnessThreshold) {
        this->wetnessThreshold = wetnessThreshold;
    }

    /**
     * Get the disease description.
     * 
     * @return Disease description.
     */
    std::string getDescription() {
        return description;
    }

    /**
     * Set the disease description.
     * 
     * @param description Disease description.
     */
    void setDescription(std::string description) {
        this->description = description;
    }

    /**
     * Get the latent period.
     * 
     * @return Latent period.
     */
    int getLatentPeriod() {
        return latentPeriod;
    }

    /**
     * Set the latent period.
     * 
     * @param latentPeriod Latent period.
     */
    void setLatentPeriod(int latentPeriod) {
        this->latentPeriod = latentPeriod;
    }

    /**
     * Get the infection period.
     * 
     * @return Infection period.
     */
    int getInfectionPeriod() {
        return infectionPeriod;
    }

    /**
     * Set the infection period.
     * 
     * @param infectionPeriod Infection period.
     */
    void setInfectionPeriod(int infectionPeriod) {
        this->infectionPeriod = infectionPeriod;
    }

    /**
     * Get the MRRS.
     * 
     * NOTE: I know it relates to rain susceptibility, but what does it stand for?
     * 
     * @return MRRS.
     */
    float getMRRS() {
        return MRRS;
    }

    /**
     * Set the MRRS.
     * 
     * @param MRRS MRRS.
     */
    void setMRRS(int MRRS) {
        this->MRRS = MRRS;
    }

    /**
     * Get the infection efficiency.
     * 
     * @return Infection efficiency.
     */
    float getInfectionEfficiency() {
        return infectionEfficiency;
    }

    /**
     * Set the infection efficiency.
     * 
     * @param infectionEfficiency Infection efficiency.
     */
    void setInfectionEfficiency(float infectionEfficiency) {
        this->infectionEfficiency = infectionEfficiency;
    }

    /**
     * Get the temperature favorability set.
     * 
     * @copydoc temperatureFavorabilitySet
     * 
     * @return Temperature favorability set.
     */
    float* getTemperatureFavorabilitySet() {
        return &temperatureFavorabilitySet[0];
    }

    /**
     * Set the temperature favorability set.
     * 
     * @copydoc temperatureFavorabilitySet
     * 
     * @param temperatureFavorabilitySet Temperature favorability set.
     */
    void setTemperatureFavorabilitySet(float temperatureFavorabilitySet[]) {
        std::copy(temperatureFavorabilitySet, 
                  temperatureFavorabilitySet + 3, 
                  this->temperatureFavorabilitySet);
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
     * Get the host factor.
     * 
     * @return Host factor.
     */
    float getHostFactor() {
        return hostFactor;
    }

    /**
     * Set the host factor.
     * 
     * @param hostFactor Host factor.
     */
    void setHostFactor(float hostFactor) {
        this->hostFactor = hostFactor;
    }

    /**
     * Get the initial pustule size.
     * 
     * @return Initial pustule size.
     */
    float getInitialPustuleSize() {
        return initialPustuleSize;
    }

    /**
     * Set the initial pustule size.
     * 
     * @param initialPustuleSize Initial pustule size.
     */
    void setInitialPustuleSize(float initialPustuleSize) {
        this->initialPustuleSize = initialPustuleSize;
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
     * Get the sporulation crowding factors set.
     * 
     * @copydoc sporulationCrowdingFactorsSet
     * 
     * NOTE: This is currently unused. Should it be removed, or should the
     *       implementation of sporulation crowding factor be calculated in .json file?
     * 
     * @return Sporulation crowding factors set.
     */
    float* getSporulationCrowdingFactorsSet() {
        return &sporulationCrowdingFactorsSet[0];
    }

    /**
     * Set the sporulation crowding factors set.
     * 
     * @copydoc sporulationCrowdingFactorsSet
     * 
     * @param sporulationCrowdingFactorsSet Sporulation crowding factors set.
     */
    void setSporulationCrowdingFactorsSet(float sporulationCrowdingFactorsSet[]) {
        std::copy(sporulationCrowdingFactorsSet, sporulationCrowdingFactorsSet + 4, 
                  this->sporulationCrowdingFactorsSet);
    }

    /**
     * Get the crop model.
     * 
     * NOTE: This is currently unused.
     * 
     * @return Crop model.
     */
    std::string getCropModel() const {
        return cropModel;
    }

    /**
     * Set the crop model.
     * 
     * NOTE: This is currently unused.
     * 
     * @param cropModel Crop model.
     */
    void setCropModel(std::string cropModel) {
        this->cropModel = cropModel;
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
    std::string getVisibleGrowthFunction() const {
        return visibleGrowthFunction;
    }
    
    /**
     * Set the visible growth function.
     * 
     * @param visibleGrowthFunction Visible growth function.
     */
    void setVisibleGrowthFunction(std::string visibleGrowthFunction) {
        this->visibleGrowthFunction = visibleGrowthFunction;
    }

    /**
     * Get the invisible growth function.
     * 
     * @return Invisible growth function.
     */
    std::string getInvisibleGrowthFunction() {
        return invisibleGrowthFunction;
    }
    
    /**
     * Set the invisible growth function.
     * 
     * @param invisibleGrowthFunction Invisible growth function.
     */
    void setInvisibleGrowthFunction(std::string invisibleGrowthFunction) {
        this->invisibleGrowthFunction = invisibleGrowthFunction;
    }

    /**
     * Get the wetness function.
     * 
     * @return Wetness function.
     */
    std::string getWetnessFunction() const {
        return wetnessFunction;
    }

    /**
     * Set the wetness function.
     * 
     * @param wetnessFunction Wetness function.
     */
    void setWetnessFunction(std::string wetnessFunction) {
        this->wetnessFunction = wetnessFunction;
    }

    /**
     * Get the deposition frequency.
     * 
     * @return Deposition frequency.
     */
    std::string getDispersionFrequency() const {
        return depositionFrequency;
    }

    /**
     * Set the deposition frequency.
     * 
     * @param depositionFrequency Deposition frequency.
     */
    void setDispersionFrequency(std::string depositionFrequency) {
        this->depositionFrequency = depositionFrequency;
    }

    /**
     * Get the RH factor.
     * 
     * @return RH factor.
     */
    std::string getRhFactor() {
        return rhFactor;
    }

    /**
     * Set the RH factor.
     * 
     * @param rhFactor RH factor.
     */
    void setRhFactor(std::string rhFactor) {
        this->rhFactor = rhFactor;
    }

    /**
     * Get the rate function characters.
     * 
     * @return Rate function characters.
     */
    std::string getRateFunctionString() {
        return rateFunctionString;
    }

    /**
     * Set the rate function characters.
     * 
     * @param chars Rate function characters.
     */
    void setRateFunction(char chars[]) {
        this->rateFunctionString = chars;
    }
};

#endif // DISEASE_H
