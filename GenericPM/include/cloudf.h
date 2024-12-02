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
        double total = Cloud::getValue() + firstSporeCloud;
        //printf("CloudF getValue - end\n");
        return total;
    }
    void output();

    void rate() {
        Cloud::rate();
    }

    void addSporesCreated(double sporesCreated) {
        this->sporesCreated = sporesCreated + Cloud::getValue();
    }
    void setSporesCreated(double sporesCreated) {
        //printf("Recebido Spores created: %f\n", sporesCreated);
        this->sporesCreated = sporesCreated;
    }

    void setFirstSporeCloud(double firstSporeCloud) {
        this->firstSporeCloud = firstSporeCloud;
    }

};

#endif // CLOUDF_H
