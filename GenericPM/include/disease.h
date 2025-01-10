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

#include <string>
#include <vector>
#include <algorithm>

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

    // NOTE: Units for all of these values would be great to include in the documentation.
    double infectionEfficiency = 0.17;
    double initialInoculum = 50;
    int vectorSizeCloudF = 5;
    int vectorSizeCloudP = 7;
    int vectorSizeCloudO = 10;
    double dailySporeProductionPerLesion = 3000;
    double maxSporeCloudsDensity = 12000;
    double proportionFromOrganToPlantCloud = 0.20;
    double proportionFromPlantToFieldCloud = 0.30;   
    int latentPeriod = 7;
    int infectionPeriod = 21;
    int MRRS = 2000;
    std::string visibleGrowthFunction = "0.4*exp(-10*exp(-0.4*x))";
    std::string wetnessFunction = "1/(1+exp(4.948-0.348*x))";
    std::string depositionFrequency = "0.367753*(x+0.001)^0.129605*exp(-0.085252*(x+0.001))";
    double initialPustuleSize = 0.00001;
    std::string invisibleGrowthFunction = "0.8*exp(-10*exp(-0.4*x))";
    double wetnessThreshold = 6;
    double acumulateFavorability = 35;
    double hostFactor = 1;
    static std::vector<Disease*> listDiseases;
    std::string rhFactor = "1*x";

    /**
     * @var cohortAgeSet
     * 
     * Set of factors related to the cohort age. These coefficients are
     * used as an input to the trapezoidal function for the calculation of 
     * new spores created by lesions.
     */ 
    double cohortAgeSet[4] = {4,20,22,30};

    /**
     * @var sporulationCrowdingFactorsSet
     * 
     * Set of coefficients used in an equation to calculate sporulation 
     * crowding factor. This factor is used in the calculation of new 
     * spores created by lesions.
     */
    double sporulationCrowdingFactorsSet[3] = {0.98669, 10.71894, 0.93374};

    /**
     * @var temperatureFavorabilitySet
     * 
     * Set of temperatures in Celsius used to calculate the favorability 
     * of the environment for the disease in terms of temperature. This is 
     * always used in Utilities::TemperatureFavorability.
     */
    double temperatureFavorabilitySet[3] = {29,10,22.5};

    /**
     * @var cardinalTempPhysiologicalLife
     * 
     * Set of temperatures in Celsius used to calculate the physiological life of an
     * organ (an analog of growing degree days). This is used by Utilities::TrapezoidalFunction.
     */
    double cardinalTempPhysiologicalLife[4] = {0, 28, 30, 40};

public:
    Disease() {
        listDiseases.push_back(this);
    }

    static std::vector<Disease*>& getDisease() {
        return listDiseases;
    }

    double getSporulationCrowdingFactor(double proportionDiseaseArea);
    int newLesions(double cloudDensity, double healthyAreaProportion);

    /**
     * Get proportion of spores from organ to plant cloud.
     * 
     * @return Proportion of spores from organ to plant cloud.
     */
    double getProportionFromOrganToPlantCloud() {
        return proportionFromOrganToPlantCloud;
    }

    /**
     * Set proportion of spores from organ to plant cloud.
     * 
     * @param proportionFromOrganToPlantCloud Proportion of spores from organ to plant cloud.
     */
    void setProportionFromOrganToPlantCloud(double proportionFromOrganToPlantCloud) {
        this->proportionFromOrganToPlantCloud = proportionFromOrganToPlantCloud;
    }

    /**
     * Get proportion of spores from plant to field cloud.
     * 
     * @return Proportion of spores from plant to field cloud.
     */
    double getProportionFromPlantToFieldCloud() {
        return proportionFromPlantToFieldCloud;
    }

    /**
     * Set proportion of spores from plant to field cloud.
     * 
     * @param proportionFromPlantToFieldCloud Proportion of spores from plant to field cloud.
     */
    void setProportionFromPlantToFieldCloud(double proportionFromPlantToFieldCloud) {
        this->proportionFromPlantToFieldCloud = proportionFromPlantToFieldCloud;
    }

    /**
     * Get maximum spore clouds density.
     * 
     * @return Maximum spore clouds density.
     */
    double getMaxSporeCloudsDensity() {
        return maxSporeCloudsDensity;
    }

    /**
     * Set maximum spore clouds density.
     * 
     * @param maxSporeCloudsDensity Maximum spore clouds density.
     */
    void setMaxSporeCloudsDensity(double maxSporeCloudsDensity) {
        this->maxSporeCloudsDensity = maxSporeCloudsDensity;
    }

    /**
     * Get cohort age set.
     * 
     * @return Set of factors/coefficients related to the cohort age function.
     */
    double* getCohortAgeSet() {
        return &cohortAgeSet[0];
    }

    /**
     * Set cohort age set.
     * 
     * @param cohortAgeSet Set of factors/coefficients related to the cohort age function.
     */
    void setCohortAgeSet(double cohortAgeSet[]) {
        std::copy(cohortAgeSet, cohortAgeSet + 4, this->cohortAgeSet);
    }

    /**
     * Get daily spore production per lesion.
     * 
     * @return Daily spore production per lesion.
     */
    double getDailySporeProductionPerLesion() {
        return dailySporeProductionPerLesion;
    }

    /**
     * Set daily spore production per lesion.
     * 
     * @param dailySporeProductionPerLesion Daily spore production per lesion.
     */
    void setDailySporeProductionPerLesion(double dailySporeProductionPerLesion) {
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
    double getWetnessThreshold() {
        return wetnessThreshold;
    }

    /**
     * Set the wetness threshold.
     * 
     * @param wetnessThreshold Wetness threshold.
     */
    void setWetnessThreshold(double wetnessThreshold) {
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
    double getMRRS() {
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
    double getInfectionEfficiency() {
        return infectionEfficiency;
    }

    /**
     * Set the infection efficiency.
     * 
     * @param infectionEfficiency Infection efficiency.
     */
    void setInfectionEfficiency(double infectionEfficiency) {
        this->infectionEfficiency = infectionEfficiency;
    }

    /**
     * Get the temperature favorability set.
     * 
     * @copydoc temperatureFavorabilitySet
     * 
     * @return Temperature favorability set.
     */
    double* getTemperatureFavorabilitySet() {
        return &temperatureFavorabilitySet[0];
    }

    /**
     * Set the temperature favorability set.
     * 
     * @copydoc temperatureFavorabilitySet
     * 
     * @param temperatureFavorabilitySet Temperature favorability set.
     */
    void setTemperatureFavorabilitySet(double temperatureFavorabilitySet[]) {
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
    double getHostFactor() {
        return hostFactor;
    }

    /**
     * Set the host factor.
     * 
     * @param hostFactor Host factor.
     */
    void setHostFactor(double hostFactor) {
        this->hostFactor = hostFactor;
    }

    /**
     * Get the initial pustule size.
     * 
     * @return Initial pustule size.
     */
    double getInitialPustuleSize() {
        return initialPustuleSize;
    }

    /**
     * Set the initial pustule size.
     * 
     * @param initialPustuleSize Initial pustule size.
     */
    void setInitialPustuleSize(double initialPustuleSize) {
        this->initialPustuleSize = initialPustuleSize;
    }

    /** 
     * Get the initial inoculum.
     * 
     * @return Initial inoculum.
     */
    double getInitialInoculum() {
        return initialInoculum;
    }

    /**
     * Set the initial inoculum.
     * 
     * @param initialInoculum Initial inoculum.
     */
    void setInitialInoculum(double initialInoculum) {
        this->initialInoculum = initialInoculum;
    }

    /**
     * Get the acumulate favorability.
     * 
     * @return Acumulate favorability.
     */
    double getAcumulateFavorability() {
        return acumulateFavorability;
    }
    
    /**
     * Set the acumulate favorability.
     * 
     * @param acumulateFavorability Acumulate favorability.
     */
    void setAcumulateFavorability(double acumulateFavorability) {
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
    double* getSporulationCrowdingFactorsSet() {
        return &sporulationCrowdingFactorsSet[0];
    }

    /**
     * Set the sporulation crowding factors set.
     * 
     * @copydoc sporulationCrowdingFactorsSet
     * 
     * @param sporulationCrowdingFactorsSet Sporulation crowding factors set.
     */
    void setSporulationCrowdingFactorsSet(double sporulationCrowdingFactorsSet[]) {
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
    double* getCardinalTempPhysiologicalLife() {
        return &cardinalTempPhysiologicalLife[0];
    }

    /**
     * Set the cardinal temperature physiological life temperature vector.
     * 
     * @copydoc cardinalTempPhysiologicalLife
     * 
     * @param cardinalTempPhysiologicalLife Cardinal temperature physiological life vector (size = 4).
     */
    void setCardinalTempPhysiologicalLife(double cardinalTempPhysiologicalLife[]) {
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
};

#endif // DISEASE_H
