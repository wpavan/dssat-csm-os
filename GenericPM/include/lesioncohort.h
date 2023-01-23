#ifndef LESIONCOHORT_H
#define LESIONCOHORT_H

#include "basic.h"
#include "basicinterface.h"
#include "cloudo.h"

#include<string>

class LesionCohort : public Basic, virtual public BasicInterface {
private:
    Utilities util;
    
protected:
    double totalArea = 0;
    double visibleArea = 0, invisibleArea = 0;
    double dailyVisibleAreaGrow = 0, dailyInvisibleAreaGrow = 0;
    double latentArea = 0;
    double infectionArea = 0;
    double necroticArea = 0;
    int lesionsInThisCohort;
    int doc = 0; // Day of creation (cohort)
    CloudO *cloudo;
    int newSpores = 0;
    double physiologicalDaysAcumm = 0; // Physiological days accumulation 
    double physiologicalDay = 0; // Physiological value on that day 
    double organHealthAreaProportion = 0;
    
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

    double getVisibleArea() {
        return visibleArea;
    }
    void output();
    void rate();
    bool isInfectionPeriod();
    bool isLatentPeriod();
    bool isNecroticPeriod();

    double getInfectionArea() {
        return infectionArea;
    }

    double getNecroticArea() {
        return necroticArea;
    }

    double getLatentArea() {
        return latentArea;
    }

    void setInvisibleArea(double invisibleArea) {
        this->invisibleArea = invisibleArea;
    }

    double getInvisibleArea() const {
        return invisibleArea;
    }

    void setVisibleArea(double visibleArea) {
        this->visibleArea = visibleArea;
    }

    void setTotalArea(double totalArea) {
        this->totalArea = totalArea;
    }

    double getTotalArea() const {
        return totalArea;
    }

    void setPhysiologicalDaysAcumm(double physiologicalDaysAcumm) {
        this->physiologicalDaysAcumm = physiologicalDaysAcumm;
    }

    double getPhysiologicalDaysAcumm() const {
        return physiologicalDaysAcumm;
    }

    void setOrganHealthAreaProportion(double organHealthAreaProportion) {
        this->organHealthAreaProportion = organHealthAreaProportion;
    }

    double getOrganHealthAreaProportion() const {
        return organHealthAreaProportion;
    }
    double getOrganDiseasedAreaProportion() const {
        return (1-organHealthAreaProportion);
    }
};

#endif // LESIONCOHORT_H
