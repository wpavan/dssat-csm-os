/**
 * @file plant.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */
#ifndef PLANT_H
#define PLANT_H

#include "basic.h"
#include "basicinterface.h"
#include "cloudp.h"
#include "organ.h"

#include <vector>

struct OrganSet {
    std::vector<Organ> organs;  // The vector of all organs that belong to this coupling point.
    CouplingPointID CP;         // The coupling point ID associated with the organ set.
    float totalValue;           // The total area, mass, or node value associated with this organ set.
                                /* The basis can be determined by calling:
                                 *     Basis b = CouplingPoint().getTrait(CP).basis;
                                 * where b is [Area | Mass | Nodes] and CP is the CouplingPointID 
                                 * from this OrganSet.
                                 */

    OrganSet(CouplingPointID cp) : CP(cp), totalValue(0) {}
};

class Plant : public Basic, virtual public BasicInterface {
protected:
    static Plant* instance;
    int doc = Basic::getWeather()->getDoy();
    // NOTE: When do we create the organ set?
    std::vector<OrganSet> organSets;
    std::vector<CloudP> cloudsP;
    static int qtd;
    int ID = ++qtd;
    float totalArea = 0, diseaseArea = 0, latentDiseaseArea = 0, infectionDiseaseArea = 0,
            necroticDiseaseArea = 0, visibleDiseaseArea = 0, invisibleDiseaseArea = 0, visibleLesions = 0,
            totalLesions = 0, senescenceArea = 0;
    static int firstOutputCall;

    Plant();

public:
    static Plant* getInstance() {
        if (instance == nullptr) {
            instance = new Plant();
        }
        return instance;
    }

    static Plant* newInstance() {
        instance = nullptr;
        return getInstance();
    }

    int getID() {
        return ID;
    }

    void rate();
    void integration();
    void output();

    float getTotalArea() {
        return totalArea;
    }

    int getDoc() {
        return doc;
    }

    std::vector<OrganSet>& getOrgans() {
        return organSets;
    }

    std::vector<CloudP>& getCloudsP() {
        return cloudsP;
    }

    bool isAlive() {
        Organ *o = nullptr;
        for (auto& set : organSets) {
            if (set.organs.size() == 0) {
                return true;
            } else {
                for (int i = set.organs.size() - 1; 1 >= 0; i--) {
                    o = &set.organs[i];
                    if (o->isAlive()) {
                        return true;
                    }
                }
            }
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

    float getVisibleLesions() {
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
