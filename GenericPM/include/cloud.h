/**
 * @file cloud.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */
#ifndef CLOUD_H
#define CLOUD_H

#include "basic.h"
#include "basicinterface.h"
#include "disease.h"
#include "debug_control.h"

#include <vector>
#include <iostream>
#include <sstream>

enum class CloudLevel {
    FIELD,
    PLANT,
    ORGAN
};

class DormantInoculum {
protected:
    static std::unordered_map<std::string, float> amountByID;
    static DormantInoculum* instance;
    DormantInoculum() {}
public:
    static DormantInoculum* getInstance(void) {
        if (!instance) {
            return new DormantInoculum();
        } else {
            return instance;
        }
    }

    DormantInoculum* newInstance(void) {
        delete instance;
        instance = nullptr;
        return getInstance();
    }

    void setDiseaseInoculum(std::string pestID, float amount) {
        amountByID[pestID] = amount;
    }

    bool hasInoculum() {
        return !amountByID.empty();
    }

    const float getDiseaseInoculum(std::string pestID) {
        auto it = amountByID.find(pestID);
        if (it != amountByID.end()) {
            return it->second;
        } else {
            return -99.0f;
        }
    }

    const std::unordered_map<std::string, float>& getDormantInoc() {
        return amountByID;
    }

    void addDormantInoculum(float amount, std::string pestID) {
        amountByID[pestID] += amount;
    }

    void removeDormantInoculum(float amount, std::string pestID) {
        amountByID[pestID] -= amount;
        if (amountByID[pestID] <= 0.0f) {
            amountByID.erase(pestID);
        }
    }

    void clear(std::string pestID) {
        amountByID.erase(pestID);
    }

    void clear() {
        amountByID.clear();
    }

    void show() {
        std::cout << "Dormant Inoculum by ID:" << std::endl;
        for (const auto& pair : amountByID) {
            std::cout << "  " << pair.first << ": " << pair.second << std::endl;
        }
    }
};

class Cloud : public Basic, virtual public BasicInterface {
private:
    bool removeByAge = false;

protected:
    // Declare storage variables for cloud inoculum.
    std::vector<float> values;    // Infective inoculum values optionally divided into age cohorts
    float dormantInoculum = 0.0f; // Inoculum that is present but not yet infective

    // Reference to the associated disease
    Disease* disease;

    // Use float to avoid truncation and preserve fractional spores
    float activeInoculumCreated = 0.0f;
    float activeInoculumRemoved = 0.0f;
    
public:
    void rate() {}
    void integration();
    void output() {}

    Disease* getDisease() {
        return disease;
    }
    
    void setDisease(Disease *disease) {
        if (disease == nullptr) {
            std::cout << "Warning: Setting Cloud disease to nullptr." << std::endl;
        }
        this->disease = disease;
    }

    float getValue(); // Returns active inoculum value (sum of values vector)

    virtual void addInoculumCreated(float activeInoculumCreated) = 0;
    virtual void addInoculumCreated(float inoculumCreated, int destination) = 0;
    
    virtual void addInoculumRemoved(float activeInoculumRemoved) = 0;
    virtual void addInoculumRemoved(float inoculumRemoved, int destination) = 0;
    
    void removeSporesVal(float toBeRemove);
    void removeSporesPct(float percent);
    void incrementSporesAge();

    // Diagnostic helper: print a compact snapshot of this cloud's internal state
    void diagSnapshot(const char *ctx) {
#if DIAG_SPORES
        std::ostringstream ss;
        ss << "[DIAG] " << ctx << " :: " << "activeInoculumCreated=" << activeInoculumCreated << ", activeInoculumRemoved=" << activeInoculumRemoved << ", total=" << getValue();
        ss << ", values=[";
        for (unsigned i=0;i<values.size();++i) {
            if (i) ss << ",";
            ss << values[i];
        }
        ss << "]";
        std::cout << ss.str() << std::endl;
#endif
    }

    virtual CloudLevel getLevel() const = 0;

    float getSporesToBeRemoved() {
        return activeInoculumRemoved;
    }

    void setSporesToBeRemoved(float activeInoculumRemoved) {
        this->activeInoculumRemoved = activeInoculumRemoved;
    }

    void addSporesToBeRemoved(float activeInoculumRemoved) {
        this->activeInoculumRemoved += activeInoculumRemoved;
    }

    void queueAgeRemoval() {
        removeByAge = true;
    }

    void reset() {
        values.clear();
        activeInoculumCreated = 0.0f;
        activeInoculumRemoved = 0.0f;
    }
};

#endif // CLOUD_H
