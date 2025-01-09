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

/**
 * Field cloud
 * 
 * This class is responsible for the field spore cloud. It is a subclass of the Cloud class.
 * This cloud is accessible by all of the plants in the simulation.
 */
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

    /**
     * Value of the field cloud
     * 
     * This method is responsible for calculating the total value of the field spore cloud.
     * This method is an extension of the Cloud class getValue method.
     * 
     * @return The total value of the field spore cloud.
     */
    double getValue() {
        double total = Cloud::getValue();
        return total;
    }

    /**
     * Rate of the field cloud
     * 
     * This method is responsible for calculating rates of change pertaining to the field cloud.
     * This method is an extension of the Cloud class rate method.
     */
    void rate() {
        Cloud::rate();
    }

    /**
     * Integration of the field cloud
     * 
     * This method is responsible for integrating changes in the field spore cloud. More
     * specifically, it is responsible for removing spores from the cloud, based on the 
     * maximum spore density and the rain effect. Then, it sends the data to a queue to be
     * outputted.
     */
    void integration();

    /**
     * Output of the field cloud
     * 
     * This method is responsible for outputting the data of the field spore cloud to a file.
     */
    void output();

    void addSporesCreated(double sporesCreated) {
        this->sporesCreated += sporesCreated;
    }
    void setSporesCreated(double sporesCreated) {
        values.clear();
        values.push_back(sporesCreated);
    }

    void setFirstSporeCloud(double firstSporeCloud) {
        this->firstSporeCloud = firstSporeCloud;
    }
};

#endif // CLOUDF_H
