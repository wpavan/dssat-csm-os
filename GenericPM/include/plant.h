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
#include "debug_control.h"

#include <vector>

struct OrganSet {
    std::vector<Organ> organs;  // The vector of all organs that belong to this coupling point.
    CouplingPointID CP;         // The coupling point ID associated with the organ set.
    float totalValue;           // The total area, mass, or node value associated with this organ set.
    float healthyValue;
    float senescenceQueue = 0.0f;
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

    void doSenescence() {
        float remainingToApply = senescenceQueue;
        
        while (remainingToApply > 0.0f) {
            // Calculate current total healthy value
            float currentHealthy = 0.0f;
            for (auto& organ : organs) {
                currentHealthy += organ.getHealthyValue();
            }
            
            // If no healthy tissue remains, discard the remainder
            if (currentHealthy <= 0.0f) {
                break;
            }
            
            // Distribute remaining senescence proportionally
            float totalRemainder = 0.0f;
            for (auto& organ : organs) {
                if (organ.getHealthyValue() > 0.0f) {
                    float senescenceValue = remainingToApply * (organ.getHealthyValue() / currentHealthy);
                    float remainder = organ.doSenescence(senescenceValue);
                    totalRemainder += remainder;
                    
#if (GENERICPM_DEBUG_ENABLED)
                    std::cout << "Organ senescence: " << senescenceValue 
                            << " (applied: " << (senescenceValue - remainder)
                            << ", remainder: " << remainder 
                            << ", organ healthy: " << organ.getHealthyValue() 
                            << ", total healthy: " << currentHealthy << ")" << std::endl;
#endif
                }
            }
            
            // Update remaining senescence to apply
            remainingToApply = totalRemainder;
            
            // Safety check to prevent infinite loops
            if (totalRemainder >= remainingToApply * 0.99f) {
                break;
            }
        }
        
        senescenceQueue = 0.0f;
    }

    OrganSet(CouplingPointID cp) : CP(cp), totalValue(0), healthyValue(0) {}
};

class Plant : public Basic, virtual public BasicInterface {
protected:
    static Plant* instance;
    int doc = FlexibleIO::getInstance()->getInteger("CONTROL", "YEARDOY"); // Day of plant creation
    // NOTE: When do we create the organ set?
    std::vector<OrganSet> organSets;
    std::vector<std::shared_ptr<CloudP>> cloudsP;
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

    // NOTE: Disease pointers should also be handled safely, 
    //       as this call seems to break in between seasons
    std::shared_ptr<CloudP> getCloudP(std::shared_ptr<Disease> disease) {
        for (auto& cloudP : cloudsP) {
            if (cloudP->getDisease() == disease) {
                return cloudP;
            }
        }
        throw std::runtime_error("CloudP for specified disease not found.");
    }

    float getTotalValue(std::shared_ptr<Disease> disease) {
        float val = 0.0;
        for (auto& organ : this->getOrganSet(disease->getOrganCP()).organs) {
            val += organ.getTotalValue();
        }
        return val;
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

    std::vector<std::shared_ptr<CloudP>>& getCloudsP() {
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

    float getVisibleValue(std::shared_ptr<Disease> disease) {
        // Implementation for getting visible value for a specific disease
        float val = 0;
        for (auto& organ : this->getOrganSet(disease->getOrganCP()).organs) {
            for (auto& cohort : organ.getLesionCohorts()) {
                if (cohort.getDisease() == disease) {
                    val += cohort.getVisibleValue();
                }
            }
        }
        return val;
    }

    float getInvisibleValue() {
        return invisibleValue;
    }

#if GENERICPM_DEBUG_ENABLED
    float getInvisibleValue(std::shared_ptr<Disease> disease) {
        std::cout << "\n=== DEBUG: getInvisibleValue for Disease: " << disease->getDiseaseID() 
                << " (Family: " << disease->getFamily() << ") ===" << std::endl;
        std::cout << "Disease OrganCP: " << cpIDToStr(disease->getOrganCP()) << std::endl;
        
        auto organSet = this->getOrganSet(disease->getOrganCP());
        std::cout << "OrganSet size: " << organSet.organs.size() << " organs" << std::endl;
        
        float val = 0;
        int organCount = 0;
        int totalCohorts = 0;
        int matchingCohorts = 0;
        
        for (auto& organ : organSet.organs) {
            organCount++;
            auto cohorts = organ.getLesionCohorts();
            std::cout << "Organ " << organCount << " has " << cohorts.size() << " lesion cohorts" << std::endl;
            
            int cohortNum = 0;
            for (auto& cohort : cohorts) {
                cohortNum++;
                totalCohorts++;
                
                Disease* cohortDisease = cohort.getDisease();
                std::cout << "  Cohort " << cohortNum << ": Disease=" << cohortDisease->getDiseaseID() 
                        << " (Family: " << cohortDisease->getFamily() << ")" << std::endl;
                std::cout << "    Cohort Disease Pointer: " << cohortDisease << std::endl;
                std::cout << "    Target Disease Pointer: " << disease << std::endl;
                
                if (cohortDisease == disease) {
                    matchingCohorts++;
                    float cohortInvisibleValue = cohort.getInvisibleValue();
                    std::cout << "    *** MATCH! Invisible value: " << cohortInvisibleValue << std::endl;
                    val += cohortInvisibleValue;
                    std::cout << "    Running total: " << val << std::endl;
                } else {
                    std::cout << "    No match (pointer comparison failed)" << std::endl;
                    // Additional check by ID if pointers don't match
                    if (cohortDisease->getDiseaseID() == disease->getDiseaseID()) {
                        std::cout << "    BUT Disease IDs match! Pointer mismatch issue!" << std::endl;
                    }
                }
            }
        }
        
        std::cout << "SUMMARY for " << disease->getDiseaseID() << ":" << std::endl;
        std::cout << "  Total organs processed: " << organCount << std::endl;
        std::cout << "  Total cohorts found: " << totalCohorts << std::endl;
        std::cout << "  Matching cohorts: " << matchingCohorts << std::endl;
        std::cout << "  Final invisible value: " << val << std::endl;
        std::cout << "=== END DEBUG ===" << std::endl;
        
        return val;
    }
#else
    float getInvisibleValue(std::shared_ptr<Disease> disease) {
        // Implementation for getting invisible value for a specific disease
        float val = 0;
        for (auto& organ : this->getOrganSet(disease->getOrganCP()).organs) {
            for (auto& cohort : organ.getLesionCohorts()) {
                if (cohort.getDisease() == disease) {
                    val += cohort.getInvisibleValue();
                }
            }
        }
        return val;
    }
#endif // GENERICPM_DEBUG_ENABLED

    float getHealthyValue(std::shared_ptr<Disease> disease) {
        float val = 0;
        for (auto& organ : this->getOrganSet(disease->getOrganCP()).organs) {
            val += organ.getHealthyValue();
        }
        return val;
    }

    float getVisibleLesions() {
        return visibleLesions;
    }

    float getTotalLesions() {
        return totalLesions;
    }    
};

#endif // PLANT_H
