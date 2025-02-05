#ifndef ORGANS_H
#define ORGANS_H

#include "basicS.h"
#include "basicinterfaceS.h"
#include "lesioncohortS.h"
#include "cloudoS.h"
#include "cloudpS.h"

#include <vector>

class OrganS : public BasicS, virtual public BasicInterfaceS {
protected:
    float totalArea = 0;
    float diseaseArea = 0;
    float visibleDiseaseArea = 0;
    float invisibleDiseaseArea = 0;
    float senescenceArea = 0;
    float latentDiseaseArea = 0;
    float infectionDiseaseArea = 0;
    float necroticDiseaseArea = 0;
    int organNumber = 0;
    int newLesionsS = 0;
    int newLesionsFromOrgan, newLesionsFromPlant, newLesionsFromField;
    int totalLesions = 0;
    int visibleLesions = 0;
    float dailyDiseaseArea = 0;
    float dailyVisibleDiseaseArea = 0;
    float dailySenescenceArea = 0;
    float dailyLatentDiseaseArea = 0;
    float dailyInfectionDiseaseArea = 0;
    float dailyNecroticDiseaseArea = 0;
    int dailyTotalLesions = 0;
    int dailyVisibleLesions = 0;
    bool suceptible = false;
    float organLastSize = 0;
    float healthAreaProportion = 0;
    float proportionFromTotalArea = 0;
    int doc = BasicS::getWeather()->getDoy();
    float physiologicalLife = 0;
    std::vector<LesionCohortS> lesionCohorts;
    std::vector<CloudOS> cloudsO;
    static int firstOutputCallS;

public:

    OrganS(std::vector<CloudPS>& cloudsP, int organNumber, float totalArea) {
        //BasicS::output.push_back("OrganS, YearDoy, TotalArea, Senesced, Diseased, VisibleArea, InvisibleArea, LesionDensity, Age, newLesionsS, TotalLesions, CloudOS, CloudPS, CloudFS, HealthAreaProportion");
        this->organNumber = organNumber;
        this->totalArea = totalArea;
        CloudPS *cloud;
        for (unsigned int i = 0; i < cloudsP.size(); i++) {
            cloud = &cloudsP[i];
            this->cloudsO.emplace_back(cloud->getDisease(), cloud);
        }
    }

    void integrationS();
    void cloudIntegrationS();
    float cloudAmountS();
    void outputS();
    void rateS();

    float getAreaIfSeverity(int porcent) {
        if(diseaseArea>(totalArea*(porcent/100.0))) {
            return totalArea;
        }
        return 0.0;
    }
    bool isAlive() {
        if (this->totalArea > 0) {//this->senescenceArea) 
            return true;
        }
        else {
            return false;
        }
    }

    float getDensity() {
        return totalArea > 0 ? (visibleLesions / totalArea) : 0;
    }

    std::vector<LesionCohortS>& getlesionCohorts() {
        return lesionCohorts;
    }

    std::vector<CloudOS>& getCloudsO() {
        return cloudsO;
    }

    void setLesionCohorts(std::vector<LesionCohortS> lesionCohorts) {
        this->lesionCohorts = lesionCohorts;
    }

    std::vector<LesionCohortS> getLesionCohorts() const {
        return lesionCohorts;
    }

    void setPhysiologicalLife(float physiologicalLife) {
        this->physiologicalLife = physiologicalLife;
    }

    float getPhysiologicalLife() const {
        return physiologicalLife;
    }

    void setDoc(int doc) {
        this->doc = doc;
    }
    
    float getProportionFromTotalArea() {
        return proportionFromTotalArea;
    }

    void setProportionFromTotalArea(float proportionFromTotalArea) {
        this->proportionFromTotalArea = proportionFromTotalArea;
    }

    int getDoc() const {
        return doc;
    }

    void setHealthAreaProportion(float healthAreaProportion) {
        this->healthAreaProportion = healthAreaProportion;
    }

    float getHealthAreaProportionS() const {
        return healthAreaProportion;
    }

    void setOrganLastSize(float organLastSize) {
        this->organLastSize = organLastSize;
    }

    float getOrganLastSize() const {
        return organLastSize;
    }

    void setSuceptible(bool suceptible) {
        this->suceptible = suceptible;
    }

    bool isSuceptible() const {
        return suceptible;
    }

    void setDailyVisibleLesions(int dailyVisibleLesions) {
        this->dailyVisibleLesions = dailyVisibleLesions;
    }

    int getDailyVisibleLesions() const {
        return dailyVisibleLesions;
    }

    void setDailyTotalLesions(int dailyTotalLesions) {
        this->dailyTotalLesions = dailyTotalLesions;
    }

    int getDailyTotalLesions() const {
        return dailyTotalLesions;
    }

    void setDailyNecroticDiseaseArea(float dailyNecroticDiseaseArea) {
        this->dailyNecroticDiseaseArea = dailyNecroticDiseaseArea;
    }

    float getDailyNecroticDiseaseArea() const {
        return dailyNecroticDiseaseArea;
    }

    void setDailyInfectionDiseaseArea(float dailyInfectionDiseaseArea) {
        this->dailyInfectionDiseaseArea = dailyInfectionDiseaseArea;
    }

    float getDailyInfectionDiseaseArea() const {
        return dailyInfectionDiseaseArea;
    }

    void setDailyLatentDiseaseArea(float dailyLatentDiseaseArea) {
        this->dailyLatentDiseaseArea = dailyLatentDiseaseArea;
    }

    float getDailyLatentDiseaseArea() const {
        return dailyLatentDiseaseArea;
    }

    void setDailySenescenceArea(float dailySenescenceArea) {
        this->dailySenescenceArea = dailySenescenceArea;
    }

    float getDailySenescenceArea() const {
        return dailySenescenceArea;
    }

    void setDailyVisibleDiseaseArea(float dailyVisibleDiseaseArea) {
        this->dailyVisibleDiseaseArea = dailyVisibleDiseaseArea;
    }

    float getDailyVisibleDiseaseArea() const {
        return dailyVisibleDiseaseArea;
    }

    void setDailyDiseaseArea(float dailyDiseaseArea) {
        this->dailyDiseaseArea = dailyDiseaseArea;
    }

    float getDailyDiseaseArea() const {
        return dailyDiseaseArea;
    }

    void setVisibleLesions(int visibleLesions) {
        this->visibleLesions = visibleLesions;
    }

    int getVisibleLesionsS() const {
        return visibleLesions;
    }

    void setTotalLesions(int totalLesions) {
        this->totalLesions = totalLesions;
    }

    int getTotalLesions() const {
        return totalLesions;
    }

    void setNewLesions(int newLesionsS) {
        this->newLesionsS = newLesionsS;
    }

    int getNewLesions() const {
        return newLesionsS;
    }

    void setOrganNumber(int organNumber) {
        this->organNumber = organNumber;
    }

    int getOrganNumber() const {
        return organNumber;
    }

    void setNecroticDiseaseArea(float necroticDiseaseArea) {
        this->necroticDiseaseArea = necroticDiseaseArea;
    }

    float getNecroticDiseaseArea() const {
        return necroticDiseaseArea;
    }

    void setInfectionDiseaseArea(float infectionDiseaseArea) {
        this->infectionDiseaseArea = infectionDiseaseArea;
    }

    float getInfectionDiseaseArea() const {
        return infectionDiseaseArea;
    }

    void setLatentDiseaseArea(float latentDiseaseArea) {
        this->latentDiseaseArea = latentDiseaseArea;
    }

    float getLatentDiseaseArea() const {
        return latentDiseaseArea;
    }

    void setSenescenceArea(float senescenceArea) {
        this->senescenceArea = senescenceArea;
    }

    float getSenescenceArea() const {
        return senescenceArea;
    }

    void setVisibleDiseaseArea(float visibleDiseaseArea) {
        this->visibleDiseaseArea = visibleDiseaseArea;
    }

    float getVisibleDiseaseArea() const {
        return visibleDiseaseArea;
    }

    float getInvisibleDiseaseArea() const {
        return invisibleDiseaseArea;
    }

    void setDiseaseArea(float diseaseArea) {
        this->diseaseArea = diseaseArea;
    }

    float getDiseaseArea() const {
        return diseaseArea;
    }

    void setTotalArea(float totalArea) {
        this->totalArea = totalArea;
    }

    float getTotalArea() const {
        return totalArea;
    }

    int getNewLesionsFromOrgan() {
        return newLesionsFromOrgan;
    }

    void setNewLesionsFromOrgan(int newLesionsFromOrgan) {
        this->newLesionsFromOrgan = newLesionsFromOrgan;
    }

    int getNewLesionsFromPlant() {
        return newLesionsFromPlant;
    }

    void setNewLesionsFromPlant(int newLesionsFromPlant) {
        this->newLesionsFromPlant = newLesionsFromPlant;
    }

    int getNewLesionsFromField() {
        return newLesionsFromField;
    }

    void setNewLesionsFromField(int newLesionsFromField) {
        this->newLesionsFromField = newLesionsFromField;
    }

};

#endif // ORGAN_H
