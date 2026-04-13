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

class CloudP : public Cloud, virtual public BasicInterface {
private:
    std::shared_ptr<CloudF> cloudF;

protected:
    static int qtd;
    int ID = ++qtd;
    static int firstOutputCall;

public:
    CloudP(Disease *disease, std::shared_ptr<CloudF> cloudF) {
        this->disease = disease;
        this->cloudF = cloudF;
        // Should we use this opportunity to use parent class constructors?
        this->values = {0};
    } 

    int getID() {
        return ID;
    }

    static constexpr CloudLevel cloudLevel = CloudLevel::FIELD;
    
    CloudLevel getLevel() const override {
        return cloudLevel;
    }

    std::shared_ptr<CloudF> getCloudF() {
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
     * @param sporesCreated Number of spores created.
     */
    void addSporesCreated(float sporesCreated);
};

#endif // CLOUDP_H
