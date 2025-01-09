/**
 * @file cloudf.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */
#ifndef CLOUDF_H
#define CLOUDF_H

#include "cloud.h"
#include "basicinterface.h"
#include "disease.h"
#include "cloud.h"

class CloudF : public Cloud, virtual public BasicInterface {
protected:
    static int qtd;
    int ID = ++qtd;
    double firstSporeCloud = 0;
    static int firstOutputCall;

public:

    CloudF(Disease *disease) {
        this->disease = disease;
    }

    int getID() {
        return ID;
    }
    void integration();

    double getValue() {
        //printf("CloudF getValue - start\n");
        double total = Cloud::getValue(); // + firstSporeCloud;
        //printf("CloudF getValue - end\n");
        return total;
    }
    void output();

    void rate() {
        Cloud::rate();
    }

    void addSporesCreated(double sporesCreated) {
        this->sporesCreated += sporesCreated;
    }
    void setSporesCreated(double sporesCreated) {
        //printf("Recebido Spores created: %f\n", sporesCreated);
        values.clear();
        values.push_back(sporesCreated);
    }

    void setFirstSporeCloud(double firstSporeCloud) {
        this->firstSporeCloud = firstSporeCloud;
    }

};

#endif // CLOUDF_H
