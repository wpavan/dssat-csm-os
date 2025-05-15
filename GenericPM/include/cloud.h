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

class Cloud : public Basic, virtual public BasicInterface {
protected:
    std::vector<float> values;
    Disease *disease;
    int sporesCreated = 0;
    int sporesToBeRemoved = 0;
    
public:
    Disease* getDisease() {
        return disease;
    }
    virtual void addSporesCreated(float sporesCreated) = 0;

    void rate() {
    }
    
    void integration();

    void output() {
    }

    float getValue();
    
    void removeSporesCloud(float toBeRemove);
    void removeSporesCloudByRain(float percent);

    int getSporesToBeRemoved() {
        return sporesToBeRemoved;
    }

    void setSporesToBeRemoved(int sporesToBeRemoved) {
        this->sporesToBeRemoved = sporesToBeRemoved;
    }

    void addSporesToBeRemoved(int sporesToBeRemoved) {
        this->sporesToBeRemoved += sporesToBeRemoved;
    }
};

#endif // CLOUD_H
