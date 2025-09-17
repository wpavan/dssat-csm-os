/**
 * @file organ.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#ifndef ORGAN_H
#define ORGAN_H

#include "basic.h"
#include "basicinterface.h"
#include "lesioncohort.h"
#include "cloudo.h"
#include "cloudp.h"

#include <vector>

class Organ : public Basic, virtual public BasicInterface {
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
    int newLesions = 0;
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
    // NOTE: can we clarify this name a bit or add documentation for hovering over it?
    int doc = Basic::getWeather()->getDoy();
    float physiologicalLife = 0;
    std::vector<LesionCohort> lesionCohorts;
    std::vector<CloudO> cloudsO;
    static int firstOutputCall;
    CouplingPointID organCP;

public:
    Organ(CouplingPointID cp, std::vector<CloudP>& cloudsP, int organNumber, float totalArea) : organCP(cp) {
        //Basic::output.push_back("Organ, YearDoy, TotalArea, Senesced, Diseased, VisibleArea, InvisibleArea, LesionDensity, Age, NewLesions, TotalLesions, CloudO, CloudP, CloudF, HealthAreaProportion");
        this->organNumber = organNumber;
        this->totalArea = totalArea;
        CloudP *cloud;
        for (unsigned int i = 0; i < cloudsP.size(); i++) {
            cloud = &cloudsP[i];
            this->cloudsO.emplace_back(cloud->getDisease(), cloud);
        }
    }

    void integration();
    void cloudIntegration();
    float cloudAmount();
    void output();
    void rate();

    bool isAlive() {
        if (this->totalArea > 0) {
            return true;
        } else {
            return false;
        }
    }

    float getDensity() {
        return totalArea > 0 ? (visibleLesions / totalArea) : 0;
    }

    std::vector<LesionCohort>& getlesionCohorts() {
        return lesionCohorts;
    }

    std::vector<CloudO>& getCloudsO() {
        return cloudsO;
    }

    void setLesionCohorts(std::vector<LesionCohort> lesionCohorts) {
        this->lesionCohorts = lesionCohorts;
    }

    std::vector<LesionCohort> getLesionCohorts() const {
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

    float getHealthAreaProportion() const {
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

    int getVisibleLesions() const {
        return visibleLesions;
    }

    void setTotalLesions(int totalLesions) {
        this->totalLesions = totalLesions;
    }

    int getTotalLesions() const {
        return totalLesions;
    }

    void setNewLesions(int newLesions) {
        this->newLesions = newLesions;
    }

    int getNewLesions() const {
        return newLesions;
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
