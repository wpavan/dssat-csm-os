#ifndef CLOUDPS_H
#define CLOUDPS_H

#include "cloudS.h"
#include "basicinterfaceS.h"
#include "cloudfS.h"

class CloudPS : public CloudS, virtual public BasicInterfaceS {
private:
    CloudFS *cloudF;

protected:
    static int qtdS;
    int ID = ++qtdS;
    static int firstOutputCallS;

public:

    CloudPS(DiseaseS *disease, CloudFS *cloudF) {
        this->disease = disease;
        this->cloudF = cloudF;
    }

    int getID() {
        return ID;
    }

    CloudFS* getCloudF() {
        return cloudF;
    }

    void rateS() {
        CloudS::rateS();
        cloudF->rateS();
    }
    void integrationS();
    void outputS();
    void addSporesCreatedS(double sporesCreated);
};

#endif // CLOUDP_H
