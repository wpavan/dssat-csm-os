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
#include "cloudf.h"

class CloudP : public Cloud, virtual public BasicInterface {
private:
    CloudF *cloudF;

protected:
    static int qtd;
    int ID = ++qtd;
    static int firstOutputCall;

public:

    CloudP(Disease *disease, CloudF *cloudF) {
        this->disease = disease;
        this->cloudF = cloudF;
    }

    int getID() {
        return ID;
    }

    CloudF* getCloudF() {
        return cloudF;
    }

    void rate() {
        Cloud::rate();
        cloudF->rate();
    }
    void integration();
    void output();
    void addSporesCreated(double sporesCreated);
};

#endif // CLOUDP_H
