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
#include <utility>

class Organ : public Basic, virtual public BasicInterface {
protected:
    float initialValue = 0;
    float invisibleValue = 0, visibleValue = 0;
    float healthyValue = 0;

    // Tracks the plant healthy value to divide up new spores. This will
    // store the plant's previous healthy value.
    float organSetHealthyValue = 0;

    int organNumber = 0;
    int newLesions = 0;
    float lesionQueue = 0.0f;
    int newLesionsFromOrgan, newLesionsFromPlant, newLesionsFromField;
    int totalLesions = 0;
    int visibleLesions = 0;

    float dailyInvisibleValue = 0, dailyVisibleValue = 0;
    float dailyHealthyValue = 0;

    int dailyTotalLesions = 0;
    int dailyVisibleLesions = 0;
    bool suceptible = false;
    float organLastSize = 0;

    float proportionFromTotalValue = 0;
    // NOTE: can we clarify this name a bit or add documentation for hovering over it?
    int doc = Basic::getWeather()->getDoy();
    float physiologicalLife = 0;
    std::vector<LesionCohort> lesionCohorts;
    std::vector<CloudO> cloudsO;
    static int firstOutputCall;
    CouplingPointID organCP;

public:
    Organ(CouplingPointID cp, std::vector<CloudP>& cloudsP, int organNumber, float initialValue) : organCP(cp) {
        //Basic::output.push_back("Organ, YearDoy, TotalValue, Senesced, Diseased, VisibleValue, InvisibleValue, LesionDensity, Age, NewLesions, TotalLesions, CloudO, CloudP, CloudF, HealthValueProportion");
        this->organNumber = organNumber;
        this->initialValue += initialValue;
        this->healthyValue += initialValue;
        CloudP *cloud;
        for (auto& cloud : cloudsP) {
            this->cloudsO.emplace_back(cloud.getDisease(), &cloud);
        }
    }

    void integration();
    void cloudIntegration();
    float cloudAmount();
    void output();
    void rate();

    float getTotalValue() const {
        return healthyValue + invisibleValue + visibleValue;
    }

    bool isAlive() {
        if (getTotalValue() > 0) {
            return true;
        } else {
            return false;
        }
    }

    float getDensity() {
        return getTotalValue() > 0 ? (visibleLesions / getTotalValue()) : 0;
    }

    std::vector<LesionCohort>& getlesionCohorts() {
        return lesionCohorts;
    }

    std::vector<CloudO>& getCloudsO() {
        return cloudsO;
    }

    void setLesionCohorts(std::vector<LesionCohort> lesionCohorts) {
        this->lesionCohorts = std::move(lesionCohorts);
    }

    const std::vector<LesionCohort>& getLesionCohorts() const {
        return lesionCohorts;
    }

    void setPhysiologicalLife(float physiologicalLife) {
        this->physiologicalLife = physiologicalLife;
    }

    float getPhysiologicalLife() const {
        return physiologicalLife;
    }

    void grow(float newValue) {
        this->healthyValue += newValue;
    }

    void setDoc(int doc) {
        this->doc = doc;
    }
    
    void setOrganSetHealthyValue(float value) {
        this->organSetHealthyValue = value;
    }

    float getOrganSetHealthyValue() {
        return organSetHealthyValue;
    }

    float getProportionFromTotalValue() {
        return proportionFromTotalValue;
    }

    void setProportionFromTotalValue(float proportionFromTotalValue) {
        this->proportionFromTotalValue = proportionFromTotalValue;
    }

    int getDoc() const {
        return doc;
    }

    float getHealthValueProportion() const {
        return healthyValue / getTotalValue();
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

    float getLatentDiseaseValue() const {
        float val = 0;
        for (auto& lc : lesionCohorts) {
            val += lc.getLatentValue();
        }
        return val;
    }

    float getInfectionValue() const {
        float val = 0;
        for (auto& lc : lesionCohorts) {
            val += lc.getInfectionValue();
        }
        return val;
    }

    float getNecroticValue() const {
        float val = 0;
        for (auto& lc : lesionCohorts) {
            val += lc.getNecroticValue();
        }
        return val;
    }

    float getHealthyValue() const {
        return healthyValue;
    }

    float getInvisibleValue() const {
        return invisibleValue;
    }

    void readInvisibleValue() {
        float val = 0;
        for (auto& lc : lesionCohorts) {
            val += lc.getInvisibleValue();
        }
        this->invisibleValue = val;
    }

    float getVisibleValue() const {
        return visibleValue;
    }

    void readVisibleValue() {
        float val = 0;
        for (auto& lc : lesionCohorts) {
            val += lc.getVisibleValue();
        }
        this->visibleValue = val;
    }

    void readDiseaseValues() {
        float invVal = 0, visVal = 0;
        for (auto& lc : lesionCohorts) {
            invVal += lc.getInvisibleValue();
            visVal += lc.getVisibleValue();
        }
        float healthyReduction = (invisibleValue + visibleValue) - (invVal + visVal);
        this->invisibleValue = invVal;
        this->visibleValue = visVal;
        this->healthyValue += healthyReduction;
    }

    float getDiseaseValue() const {
        return getInvisibleValue() + getVisibleValue();
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
