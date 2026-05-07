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
    static std::unordered_map<std::string, float> amountByFamily;
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
        instance = nullptr;
        return getInstance();
    }

    void setFamilyInoculum(std::string family, float amount) {
        amountByFamily[family] = amount;
    }

    bool hasInoculum() {
        return !amountByFamily.empty();
    }

    const float getFamilyInoculum(std::string family) {
        auto it = amountByFamily.find(family);
        if (it != amountByFamily.end()) {
            return it->second;
        } else {
            return -99.0f;
        }
    }

    const std::unordered_map<std::string, float>& getDormantInoc() {
        return amountByFamily;
    }

    void addDormantInoculum(float amount, std::string family) {
        amountByFamily[family] += amount;
    }

    void clear(std::string family) {
        amountByFamily.erase(family);
    }

    void clear() {
        amountByFamily.clear();
    }

    void show() {
        std::cout << "Dormant Inoculum by Family:" << std::endl;
        for (const auto& pair : amountByFamily) {
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
    Disease *disease;

    // Use float to avoid truncation and preserve fractional spores
    float activeInoculumCreated, activeInoculumRemoved = 0.0f;
    float dormantInoculumCreated, dormantInoculumRemoved = 0.0f;
    
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
