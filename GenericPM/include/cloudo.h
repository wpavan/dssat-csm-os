/**
 * @file cloudo.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */
#ifndef CLOUDO_H
#define CLOUDO_H

#include "cloud.h"
#include "basicinterface.h"
#include "cloudp.h"

class CloudO : public Cloud, virtual public BasicInterface, public std::enable_shared_from_this<CloudO> {
private:
    std::shared_ptr<CloudP> cloudP;

protected:
    static int qtd;
    int ID = ++qtd;
    static int firstOutputCall;

public:
    CloudO(Disease *disease, std::shared_ptr<CloudP> cloudP) {
        this->disease = disease;
        this->cloudP = cloudP;
    }

    static constexpr CloudLevel cloudLevel = CloudLevel::ORGAN;
    
    CloudLevel getLevel() const override {
        return cloudLevel;
    }

    int getID() {
        return ID;
    }

    void rate();

    std::shared_ptr<CloudP> getCloudP() {
        return cloudP;
    }
    
    void integration();
    void output();

    /**
     * Increase number of spores in the cloud.
     * 
     * The number of spores created is partitioned into the CloudO and parent CloudP objects 
     * according to the proportion defined in the Disease object.
     * 
     * @param activeInoculumCreated Number of spores created.
     */
    void addInoculumCreated(float activeInoculumCreated) override;
    void addInoculumCreated(float inoculumCreated, int destination) override;
    
    void addInoculumRemoved(float activeInoculumRemoved) override;
    void addInoculumRemoved(float inoculumRemoved, int destination) override;
};

#endif // CLOUDO_H
