#ifndef CLOUDFS_H
#define CLOUDFS_H

#include "cloudS.h"
#include "basicinterfaceS.h"
#include "diseaseS.h"
#include "cloudS.h"

class CloudFS : public CloudS, virtual public BasicInterfaceS {
protected:
    static int qtdS;
    int ID = ++qtdS;
    float firstSporeCloud = 0;
    static int firstOutputCallS;

public:

    CloudFS(DiseaseS *disease) {
        this->disease = disease;
    }

    int getID() {
        return ID;
    }
    void integrationS();

    float getValueS() {
        return CloudS::getValueS() + firstSporeCloud;
    }
    void setValue(float value) {
        CloudS::values.clear();
        CloudS::values.push_back(value);
    }
    void outputS();

    void rateS() {
        CloudS::rateS();
    }

    void addSporesCreatedS(float sporesCreated) {
        this->sporesCreated += sporesCreated;
    }

    void setFirstSporeCloud(float firstSporeCloud) {
        this->firstSporeCloud = firstSporeCloud;
    }

};

#endif // CLOUDF_H
