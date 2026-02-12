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
#include "project_config.h"

/**
 * Field cloud
 * 
 * This class is responsible for the field spore cloud. It is a subclass of the Cloud class.
 * This cloud is accessible by all of the plants in the simulation.
 */
class CloudF : public Cloud, virtual public BasicInterface {
protected:
    std::string family;
    int qtd;
    int ID = ++qtd;
    float firstSporeCloud = 0;
    static int firstOutputCall;
    // Disease* disease = nullptr;
    int lastRate = -99;
    int lastIntegration = -99;
    int lastOutput = -99;

public:
    CloudF(std::string fam) : family(fam) {
        Basic::output.push_back("Day of Simulation, Total Spores in CloudF");
    }

    // Disease* getDisease() {
    //     return disease;
    // }

    std::string getFamily() {
        return family;
    }

    void setFamily(std::string family) {
        this->family = family;
    }

    void setDisease(Disease *disease) {
        if (disease == nullptr) {
            std::cout << "Warning: Setting CloudF disease to nullptr for family: " << family << std::endl;
        }
        this->disease = disease;
    }

    int getID() {
        return ID;
    }

    /**
     * Rate of the field cloud
     * 
     * This method is responsible for calculating rates of change pertaining to the field cloud.
     * This method is an extension of the Cloud class rate method.
     */
    void rate();

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

    void addSporesCreated(float sporesCreated) {
#if DIAG_SPORES
        std::cout << "[DIAG] YEARDOY:" << Basic::getWeather()->getYearDoy() << " CloudF::addSporesCreated sporesCreated=" << sporesCreated << std::endl;
#endif
        this->sporesCreated += sporesCreated;
    }

    void setSporesCreated(float sporesCreated) {
        values.clear();
        values.push_back(sporesCreated);
    }

    void setFirstSporeCloud(float firstSporeCloud) {
        this->firstSporeCloud += firstSporeCloud;
    }
};

#endif // CLOUDF_H
