/**
 * @file lesioncohort.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */
#ifndef LESIONCOHORT_H
#define LESIONCOHORT_H

#include "basic.h"
#include "basicinterface.h"
#include "cloudo.h"

#include <string>

class LesionCohort : public Basic, virtual public BasicInterface {
private:
    Utilities util;
    
protected:
    // Partitions of the lesion cohort total value.
    float visibleValue = 0, invisibleValue = 0;

    // Corresponding daily increments
    float dailyVisibleValue = 0, dailyInvisibleValue = 0;
   
    float organHealthyValue = 0.0f, organDiseaseValue = 0.0f;

    int lesionsInThisCohort;
    int doc = 0; // Day of creation (cohort)
    CloudO *cloudo;
    int newSpores = 0;
    float physiologicalAge = 0; // Physiological days accumulation 
    float dailyAge = 0; // Physiological value on that day 
    float organHealthyValueProportion = 0;
    
    static int qtd;
    int ID = ++qtd;

public:

    LesionCohort(int lesionsInThisCohort, CloudO *cloudo) {
        Basic::output.push_back("Day of Simulation, Area, Amount of Cohorts, Physiological days,Proportion Disease Area, Latent Area , Infection Area , Necrotic Area, New Spores, Temp.Favorability, dailyVisibleAreaGrow, dailyInvisibleAreaGrow");
        this->lesionsInThisCohort = lesionsInThisCohort;
        this->cloudo = cloudo;
        this->visibleValue = 0;
        this->invisibleValue = (lesionsInThisCohort * cloudo->getDisease()->getInitialLesionSize().evaluate());
        this->doc = FlexibleIO::getInstance()->getInteger("CONTROL", "YEARDOY");
    }

    int getID() {
        return ID;
    }
    int getAge() {
        return (FlexibleIO::getInstance()->getInteger("CONTROL", "YEARDOY") - doc);
    }
    void integration();
    int getVisibleLesions();

    float getVisibleArea() {
        return visibleValue;
    }
    void output();

    /**
     * @brief Rate method for the LesionCohort class.
     * 
     * This method calculates the daily changes for the following state variables:
     *  - physiologicalAge
     *  - visibleValue
     *  - invisibleValue
     * 
     * Additionally, it calculates the lesion's contribution to new spore production.
     */
    void rate();
    
    bool isInfectionPeriod() const;
    bool isLatentPeriod() const;
    bool isNecroticPeriod() const;

    float getInfectionValue() const {
        return isInfectionPeriod() ? getTotalValue() : 0;
    }

    float getNecroticValue() const {
        return isNecroticPeriod() ? getTotalValue() : 0;
    }

    float getLatentValue() const {
        return isLatentPeriod() ? getTotalValue() : 0;
    }

    float getInvisibleValue() const {
        return invisibleValue;
    }

    float getVisibleValue() const {
        return visibleValue;
    }

    float getTotalValue() const {
        return visibleValue + invisibleValue;
    }

    void setPhysiologicalDaysAcumm(float physiologicalAge) {
        this->physiologicalAge = physiologicalAge;
    }

    float getPhysiologicalDaysAcumm() const {
        return physiologicalAge;
    }

    void setOrganHealthValueProportion(float organHealthyValueProportion) {
        this->organHealthyValueProportion = organHealthyValueProportion;
    }

    void setOrganDiseaseValue(float organDiseaseValue) {
        this->organDiseaseValue = organDiseaseValue;
    }

    float getOrganDiseaseValue() const {
        return this->organDiseaseValue;
    }

    void setOrganHealthyValue(float organHealthyValue) {
        this->organHealthyValue = organHealthyValue;
    }

    float getOrganHealthyValue() const {
        return this->organHealthyValue;
    }

    float getOrganHealthValueProportion() const {
        return organHealthyValueProportion;
    }
    float getOrganDiseasedValueProportion() const {
        return (1-organHealthyValueProportion);
    }
};

#endif // LESIONCOHORT_H
