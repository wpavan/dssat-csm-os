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
    float totalArea = 0;
    float visibleArea = 0, invisibleArea = 0;
    float dailyVisibleAreaGrow = 0, dailyInvisibleAreaGrow = 0;
    float latentArea = 0;
    float infectionArea = 0;
    float necroticArea = 0;
    int lesionsInThisCohort;
    int doc = 0; // Day of creation (cohort)
    CloudO *cloudo;
    int newSpores = 0;
    float physiologicalDaysAcumm = 0; // Physiological days accumulation 
    float physiologicalDay = 0; // Physiological value on that day 
    float organHealthAreaProportion = 0;
    
    static int qtd;
    int ID = ++qtd;

public:

    LesionCohort(int lesionsInThisCohort, CloudO *cloudo) {
        Basic::output.push_back("Day of Simulation, Area, Amount of Cohorts, Physiological days,Proportion Disease Area, Latent Area , Infection Area , Necrotic Area, New Spores, Temp.Favorability, dailyVisibleAreaGrow, dailyInvisibleAreaGrow");
        this->lesionsInThisCohort = lesionsInThisCohort;
        this->cloudo = cloudo;
        this->visibleArea = 0;
        this->invisibleArea = (lesionsInThisCohort * cloudo->getDisease()->getInitialPustuleSize());
        this->totalArea = this->visibleArea + this->invisibleArea;
        this->doc = getWeather()->getDoy();
    }

    int getID() {
        return ID;
    }
    int getAge() {
        return (Basic::getWeather()->getDoy() - doc);
    }
    void integration();
    int getVisibleLesions();

    float getVisibleArea() {
        return visibleArea;
    }
    void output();
    void rate();
    bool isInfectionPeriod();
    bool isLatentPeriod();
    bool isNecroticPeriod();

    float getInfectionArea() {
        return infectionArea;
    }

    float getNecroticArea() {
        return necroticArea;
    }

    float getLatentArea() {
        return latentArea;
    }

    void setInvisibleArea(float invisibleArea) {
        this->invisibleArea = invisibleArea;
    }

    float getInvisibleArea() const {
        return invisibleArea;
    }

    void setVisibleArea(float visibleArea) {
        this->visibleArea = visibleArea;
    }

    void setTotalArea(float totalArea) {
        this->totalArea = totalArea;
    }

    float getTotalArea() const {
        return totalArea;
    }

    void setPhysiologicalDaysAcumm(float physiologicalDaysAcumm) {
        this->physiologicalDaysAcumm = physiologicalDaysAcumm;
    }

    float getPhysiologicalDaysAcumm() const {
        return physiologicalDaysAcumm;
    }

    void setOrganHealthAreaProportion(float organHealthAreaProportion) {
        this->organHealthAreaProportion = organHealthAreaProportion;
    }

    float getOrganHealthAreaProportion() const {
        return organHealthAreaProportion;
    }
    float getOrganDiseasedAreaProportion() const {
        return (1-organHealthAreaProportion);
    }
};

#endif // LESIONCOHORT_H
