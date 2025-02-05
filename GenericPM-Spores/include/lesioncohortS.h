#ifndef LESIONCOHORTS_H
#define LESIONCOHORTS_H

#include "basicS.h"
#include "basicinterfaceS.h"
#include "cloudoS.h"

#include<string>

class LesionCohortS : public BasicS, virtual public BasicInterfaceS {
private:
    UtilitiesS util;
    
protected:
    float totalArea = 0;
    float visibleArea = 0, invisibleArea = 0;
    float dailyVisibleAreaGrow = 0, dailyInvisibleAreaGrow = 0;
    float latentArea = 0;
    float infectionArea = 0;
    float necroticArea = 0;
    int lesionsInThisCohort;
    int doc = 0; // Day of creation (cohort)
    CloudOS *cloudo;
    int newSpores = 0;
    float physiologicalDaysAcumm = 0; // Physiological days accumulation 
    float physiologicalDay = 0; // Physiological value on that day 
    float organHealthAreaProportion = 0;
    
    static int qtdS;
    int ID = ++qtdS;

public:

    LesionCohortS(int lesionsInThisCohort, CloudOS *cloudo) {
        BasicS::output.push_back("Day of Simulation, Area, Amount of Cohorts, Physiological days,Proportion DiseaseS Area, Latent Area , Infection Area , Necrotic Area, New Spores, Temp.Favorability, dailyVisibleAreaGrow, dailyInvisibleAreaGrow");
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
        return (BasicS::getWeather()->getDoy() - doc);
    }
    void integrationS();
    int getVisibleLesionsS();

    float getVisibleArea() {
        return visibleArea;
    }
    void outputS();
    void rateS();
    bool isInfectionPeriodS();
    bool isLatentPeriodS();
    bool isNecroticPeriodS();

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
