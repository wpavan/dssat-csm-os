#ifndef CLOUDOS_H
#define CLOUDOS_H

#include "cloudS.h"
#include "basicinterfaceS.h"
#include "cloudpS.h"

class CloudOS : public CloudS, virtual public BasicInterfaceS {
private:
    CloudPS *cloudP;

protected:
    static int qtdS;
    int ID = ++qtdS;
    static int firstOutputCallS;

public:

    CloudOS(DiseaseS *disease, CloudPS *cloudP) {
        this->disease = disease;
        this->cloudP = cloudP;
    }

    int getID() {
        return ID;
    }

    void rateS() {
        CloudS::rateS();
        cloudP->rateS();
    }

    CloudPS* getCloudP() {
        return cloudP;
    }
    void integrationS();
    void outputS();
    void addSporesCreatedS(double sporesCreated);


};

#endif // CLOUDO_H
