#ifndef DISEASES_H
#define DISEASES_H

#include "basicS.h"
#include "utilitiesS.h"

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;

class DiseaseS : public BasicS {
protected:
    int id;
    std::string cropModel = "CRGRO";
    std::string description = "Soybean Leaf Rust";
    float infectionEfficiency = 0.17;
    //float depositionFrequency = 0.15;
    float initialInoculum = 50;
    int vectorSizeCloudF = 5;
    int vectorSizeCloudP = 7;
    int vectorSizeCloudO = 10;
    float dailySporeProductionPerLesion = 3000;
    float cohortAgeSet[4] = {4,20,22,30};
    float sporulationCrowdingFactorsSet[3] = {0.98669, 10.71894, 0.93374}; 
    float maxSporeCloudsDensity = 12000;
    float proportionFromOrganToPlantCloud = 0.20;
    float proportionFromPlantToFieldCloud = 0.30;
    float temperatureFavorabilitySet[3] = {29,10,22.5};
    int latentPeriod = 7;
    int infectionPeriod = 21;
    int MRRS = 2000;
    std::string visibleGrowthFunction = "0.4*exp(-10*exp(-0.4*x))";
    std::string wetnessFunction = "1/(1+exp(4.948-0.348*x))";
    std::string depositionFrequency = "0.367753*(x+0.001)^0.129605*exp(-0.085252*(x+0.001))";
    float initialPustuleSize = 0.00001;
    std::string invisibleGrowthFunction = "0.8*exp(-10*exp(-0.4*x))";
    float wetnessThreshold = 6;
    float acumulateFavorability = 35;
    float hostFactor = 1;
    float cardinalTempPhysiologicalLife[4] = {0, 28, 30, 40};
    static std::vector<DiseaseS*> listDiseasesS;
    std::string rhFactor = "1*x";

public:

    DiseaseS() {
        listDiseasesS.push_back(this);
    }

    static std::vector<DiseaseS*>& getDisease() {
        return listDiseasesS;
    }

    void printDiseaseS() {
        std::cout << "Disease ID: " << id << std::endl;
        std::cout << "Description: " << description << std::endl;
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

    float getSporulationCrowdingFactorS(float proportionDiseaseArea);
    int newLesionsS(float cloudDensity, float healthyAreaProportion);

    float getProportionFromOrganToPlantCloud() {
        return proportionFromOrganToPlantCloud;
    }

    float getProportionFromPlantToFieldCloud() {
        return proportionFromPlantToFieldCloud;
    }

    void setProportionFromOrganToPlantCloud(float proportionFromOrganToPlantCloud) {
        this->proportionFromOrganToPlantCloud = proportionFromOrganToPlantCloud;
    }

    void setProportionFromPlantToFieldCloud(float proportionFromPlantToFieldCloud) {
        this->proportionFromPlantToFieldCloud = proportionFromPlantToFieldCloud;
    }

    void setMaxSporeCloudsDensity(float maxSporeCloudsDensity) {
        this->maxSporeCloudsDensity = maxSporeCloudsDensity;
    }

    float getMaxSporeCloudsDensity() {
        return maxSporeCloudsDensity;
    }

    void setCohortAgeSet(float cohortAgeSet[]) {
        std::copy(cohortAgeSet, cohortAgeSet + 4, this->cohortAgeSet);
    }

    float* getCohortAgeSet() {
        return &cohortAgeSet[0];
    }

    float getDailySporeProductionPerLesion() {
        return dailySporeProductionPerLesion;
    }

    int getId() {
        return id;
    }

    void setId(int id) {
        this->id = id;
    }

    float getWetnessThreshold() {
        return wetnessThreshold;
    }

    void setWetnessThreshold(float wetnessThreshold) {
        this->wetnessThreshold = wetnessThreshold;
    }

    std::string getDescription() {
        return description;
    }

    void setDescription(std::string description) {
        this->description = description;
    }

    int getLatentPeriod() {
        return latentPeriod;
    }

    void setLatentPeriod(int latentPeriod) {
        this->latentPeriod = latentPeriod;
    }

    int getInfectionPeriod() {
        return infectionPeriod;
    }

    void setInfectionPeriod(int infectionPeriod) {
        this->infectionPeriod = infectionPeriod;
    }
  
    void setMRRS(int MRRS) {
        this->MRRS = MRRS;
    }

    float getMRRS() {
        return MRRS;
    }

    float getInfectionEfficiency() {
        return infectionEfficiency;
    }
    
    void setInfectionEfficiency(float infectionEfficiency) {
        this->infectionEfficiency = infectionEfficiency;
    }

    float* getTemperatureFavorabilitySet() {
        return &temperatureFavorabilitySet[0];
    }

    void setTemperatureFavorabilitySet(float temperatureFavorabilitySet[]) {
        std::copy(temperatureFavorabilitySet, temperatureFavorabilitySet + 3, this->temperatureFavorabilitySet);
    }

    int getVectorSizeCloudF() {
        return vectorSizeCloudF;
    }

    int getVectorSizeCloudO() {
        return vectorSizeCloudO;
    }

    int getVectorSizeCloudP() {
        return vectorSizeCloudP;
    }

    float getHostFactor() {
        return hostFactor;
    }

    float getInitialPustuleSize() {
        return initialPustuleSize;
    }

    void setInitialPustuleSize(float initialPustuleSize) {
        this->initialPustuleSize = initialPustuleSize;
    }

    std::string getInvisibleGrowthFunction() {
        return invisibleGrowthFunction;
    }

    float getInitialInoculum() {
        return initialInoculum;
    }

    void setInitialInoculum(float initialInoculum) {
        this->initialInoculum = initialInoculum;
    }

    float getAcumulateFavorability() {
        return acumulateFavorability;
    }

    void setVectorSizeCloudF(int vectorSizeCloudF) {
        this->vectorSizeCloudF = vectorSizeCloudF;
    }

    void setVectorSizeCloudP(int vectorSizeCloudP) {
        this->vectorSizeCloudP = vectorSizeCloudP;
    }

    void setVectorSizeCloudO(int vectorSizeCloudO) {
        this->vectorSizeCloudO = vectorSizeCloudO;
    }

    void setDailySporeProductionPerLesion(float dailySporeProductionPerLesion) {
        this->dailySporeProductionPerLesion = dailySporeProductionPerLesion;
    }

    void setSporulationCrowdingFactorsSet(float sporulationCrowdingFactorsSet[]) {
        std::copy(sporulationCrowdingFactorsSet, sporulationCrowdingFactorsSet + 4, this->sporulationCrowdingFactorsSet);
    }

    float* getSporulationCrowdingFactorsSet() {
        return &sporulationCrowdingFactorsSet[0];
    }

    void setInvisibleGrowthFunction(std::string invisibleGrowthFunction) {
        this->invisibleGrowthFunction = invisibleGrowthFunction;
    }

    void setAcumulateFavorability(float acumulateFavorability) {
        this->acumulateFavorability = acumulateFavorability;
    }

    void setHostFactor(float hostFactor) {
        this->hostFactor = hostFactor;
    }

    void setCropModel(std::string cropModel) {
        this->cropModel = cropModel;
    }

    std::string getCropModel() const {
        return cropModel;
    }
    
    void setCardinalTempPhysiologicalLife(float cardinalTempPhysiologicalLife[]) {
        std::copy(cardinalTempPhysiologicalLife, cardinalTempPhysiologicalLife + 4, this->cardinalTempPhysiologicalLife);
    }

    float* getCardinalTempPhysiologicalLife() {
        return &cardinalTempPhysiologicalLife[0];
    }

    void setVisibleGrowthFunction(std::string visibleGrowthFunction) {
        this->visibleGrowthFunction = visibleGrowthFunction;
    }

    void setWetnessFunction(std::string wetnessFunction) {
        this->wetnessFunction = wetnessFunction;
    }

    std::string getWetnessFunction() const {
        return wetnessFunction;
    }

    std::string getVisibleGrowthFunction() const {
        return visibleGrowthFunction;
    }

    void setDispersionFreequency(std::string depositionFrequency) {
        this->depositionFrequency = depositionFrequency;
    }

    std::string getDispersionFreequency() const {
        return depositionFrequency;
    }

    
    std::string getRhFactor() {
        return rhFactor;
    }

    void setRhFactor(std::string rhFactor) {
        this->rhFactor = rhFactor;
    }
};

#endif // DISEASE_H
