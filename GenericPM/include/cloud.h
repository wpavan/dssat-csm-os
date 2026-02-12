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

class Cloud : public Basic, virtual public BasicInterface {
private:
    bool removeByAge = false;

protected:
    std::vector<float> values;
    Disease *disease;
    // Use float to avoid truncation and preserve fractional spores
    float sporesCreated = 0.0f;
    float sporesToBeRemoved = 0.0f;
    
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

    float getValue();

    virtual void addSporesCreated(float sporesCreated) = 0;
    
    void removeSporesVal(float toBeRemove);
    void removeSporesPct(float percent);
    void incrementSporesAge();

    // Diagnostic helper: print a compact snapshot of this cloud's internal state
    void diagSnapshot(const char *ctx) {
#if DIAG_SPORES
        std::ostringstream ss;
        ss << "[DIAG] " << ctx << " :: " << "sporesCreated=" << sporesCreated << ", sporesToBeRemoved=" << sporesToBeRemoved << ", total=" << getValue();
        ss << ", values=[";
        for (unsigned i=0;i<values.size();++i) {
            if (i) ss << ",";
            ss << values[i];
        }
        ss << "]";
        std::cout << ss.str() << std::endl;
#endif
    }

    float getSporesToBeRemoved() {
        return sporesToBeRemoved;
    }

    void setSporesToBeRemoved(float sporesToBeRemoved) {
        this->sporesToBeRemoved = sporesToBeRemoved;
    }

    void addSporesToBeRemoved(float sporesToBeRemoved) {
        this->sporesToBeRemoved += sporesToBeRemoved;
    }

    void queueAgeRemoval() {
        removeByAge = true;
    }

    void reset() {
        values.clear();
        sporesCreated = 0.0f;
        sporesToBeRemoved = 0.0f;
    }
};

#endif // CLOUD_H
