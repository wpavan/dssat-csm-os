#ifndef PLANTS_H
#define PLANTS_H

#include "basicS.h"
#include "basicinterfaceS.h"
#include "cloudpS.h"
#include "organS.h"

#include <vector>

class PlantS : public BasicS, virtual public BasicInterfaceS {
protected:
    int doc = BasicS::getWeather()->getDoy();
    std::vector<OrganS> organs;
    std::vector<CloudPS> cloudsP;
    static int qtdS;
    int ID = ++qtdS;
    float totalArea = 0, diseaseArea = 0, latentDiseaseArea = 0, infectionDiseaseArea = 0,
            necroticDiseaseArea = 0, visibleDiseaseArea = 0, invisibleDiseaseArea = 0, visibleLesions = 0,
            totalLesions = 0, senescenceArea = 0;
    static int firstOutputCallS;
public:
    PlantS();

    int getID() {
        return ID;
    }
    void integrationS();
    void outputS();
    void rateS();

    float getTotalArea() {
        return totalArea;
    }

    int getDoc() {
        return doc;
    }

    std::vector<OrganS>& getOrgans() {
        return organs;
    }

    std::vector<CloudPS>& getCloudsP() {
        return cloudsP;
    }

    bool isAlive() {
        if (organs.size() == 0)
            return true;
        for (int i = organs.size() - 1; i >= 0; i--) {
            OrganS *o = &organs[i];
            if (o->isAlive())
                return true;
        }
        return false;
    }

    float getDiseaseArea() {
        return diseaseArea;
    }

    float getLatentDiseaseArea() {
        return latentDiseaseArea;
    }

    float getInfectionDiseaseArea() {
        return infectionDiseaseArea;
    }

    float getNecroticDiseaseArea() {
        return necroticDiseaseArea;
    }

    float getVisibleDiseaseArea() {
        return visibleDiseaseArea;
    }

    float getInvisibleDiseaseArea() {
        return invisibleDiseaseArea;
    }

    float getVisibleLesionsS() {
        return visibleLesions;
    }

    float getTotalLesions() {
        return totalLesions;
    }

    float getSenescenceArea() {
        return senescenceArea;
    }
    
};

#endif // PLANT_H
