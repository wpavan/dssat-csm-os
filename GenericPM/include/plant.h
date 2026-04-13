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

#define OUTPUT_PLANT 1

#include "basic.h"
#include "basicinterface.h"
#include "cloudp.h"
#include "organ.h"

#include <vector>

struct OrganSet {
    std::vector<Organ> organs;  // The vector of all organs that belong to this coupling point.
    CouplingPointID CP;         // The coupling point ID associated with the organ set.
    float totalValue;           // The total area, mass, or node value associated with this organ set.
    float healthyValue;
    float growthQueue = 0.0f;
                                /* The basis can be determined by calling:
                                 *     Basis b = CouplingPoint().getTrait(CP).basis;
                                 * where b is [Area | Mass | Nodes] and CP is the CouplingPointID 
                                 * from this OrganSet.
                                 */
                            
    void queueHealthyGrowth(float value) {
        growthQueue += value;
    }

    void clearGrowthQueue() {
        growthQueue = 0.0f;
    }

    OrganSet(CouplingPointID cp) : CP(cp), totalValue(0), healthyValue(0) {}
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

    // Value partitions
    float invisibleValue = 0, visibleValue = 0;
    float healthyValue = 0;
    // Previous value partitions
    float invisibleValuePrev = 0, visibleValuePrev = 0;
    float healthyValuePrev = 0;

    // Lesion trackers
    float totalLesions = 0, visibleLesions = 0;
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

    float getTotalValue() {
        return healthyValue + invisibleValue + visibleValue;
    }

    int getDoc() {
        return doc;
    }

    void updatePrev() {
        invisibleValuePrev = invisibleValue;
        visibleValuePrev = visibleValue;
        healthyValuePrev = healthyValue;
    }

    OrganSet& getOrganSet(CouplingPointID cp) {
        for (auto& os : organSets) {
            if (os.CP == cp) {
                return os;
            }
        }
        throw std::runtime_error("OrganSet with specified CouplingPointID not found.");
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
                // should this be false?
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

    float getDailyDiseaseValue() {
        return (invisibleValue - invisibleValuePrev) + (visibleValue - visibleValuePrev);
    }

    float getDiseaseValue() {
        return invisibleValue + visibleValue;
    }

    float getLatentValue() {
        float val = 0;
        for (auto& set : organSets) {
            for (auto& organ : set.organs) {
                val += organ.getLesionCohorts().size() > 0 ? organ.getLesionCohorts().back().getLatentValue() : 0;
            }
        }
        return val;
    }

    float getInfectionValue() {
        float val = 0;
        for (auto& set : organSets) {
            for (auto& organ : set.organs) {
                val += organ.getLesionCohorts().size() > 0 ? organ.getLesionCohorts().back().getInfectionValue() : 0;
            }
        }
        return val;
    }

    float getNecroticValue() {
        float val = 0;
        for (auto& set : organSets) {
            for (auto& organ : set.organs) {
                val += organ.getLesionCohorts().size() > 0 ? organ.getLesionCohorts().back().getNecroticValue() : 0;
            }
        }
        return val;
    }

    float getVisibleValue() {
        return visibleValue;
    }

    float getInvisibleValue() {
        return invisibleValue;
    }

    float getVisibleLesions() {
        return visibleLesions;
    }

    float getTotalLesions() {
        return totalLesions;
    }    
};

#endif // PLANT_H
