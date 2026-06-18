/**
 * @file cloudp.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */
#ifndef CLOUDP_H
#define CLOUDP_H

#include "cloud.h"
#include "basicinterface.h"
#include <memory>
#include "cloudf.h"

class CloudP : public Cloud, virtual public BasicInterface, public std::enable_shared_from_this<CloudP> {
private:
    std::shared_ptr<CloudF> cloudF;

protected:
    static int qtd;
    int ID = ++qtd;
    static int firstOutputCall;

public:
    CloudP(std::shared_ptr<Disease> disease, std::shared_ptr<CloudF> cloudF) : cloudF(cloudF) {
        this->disease = disease;
        // Should we use this opportunity to use parent class constructors?
        this->values = {0};
    } 

    bool operator==(const CloudP& other) const {
        return this->cloudF->getFamily() == other.cloudF->getFamily();
    }

    int getID() {
        return ID;
    }

    const std::string getFamily() {
        return cloudF->getFamily();
    }

    static constexpr CloudLevel cloudLevel = CloudLevel::PLANT;
    
    CloudLevel getLevel() const override {
        return cloudLevel;
    }

    std::shared_ptr<CloudF>& getCloudF() {
        if (!cloudF) {
            throw std::runtime_error("CloudP has nullptr CloudF.");
        }
        return cloudF;
    }

    void rate();    
    void integration();
    void output();

    /**
     * Increase number of spores in the plant cloud.
     * 
     * The number of spores created is partitioned into the CloudP and parent CloudF objects 
     * according to the proportion defined in the Disease object.
     * 
     * @param activeInoculumCreated Number of spores created.
     */
    void addInoculumCreated(float activeInoculumCreated) override;
    void addInoculumCreated(float inoculumCreated, int destination) override;
    
    void addInoculumRemoved(float activeInoculumRemoved) override;
    void addInoculumRemoved(float inoculumRemoved, int destination) override;
};

#endif // CLOUDP_H
