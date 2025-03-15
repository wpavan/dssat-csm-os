#ifndef CINTERFACES_H
#define CINTERFACES_H

#include <iostream>
#include "include/simulatorS.h"
#include "include/utilitiesS.h"
#include <cmath>
#include <vector>
#include <fstream>
#include "../FlexibleIO/Data/FlexibleIO.hpp"


class CinterfaceSpore {

private:
int YRDOY;
float CloudValue = 0;

public:
    float couplingIntegrationSpore(int YRDOY);

    void setcouplingCloudSpore(float CloudValue) {
        //std::cout<<"CloudValue H: "<<CloudValue<<std::endl;
        this->CloudValue = CloudValue;
    }

    float getcouplingCloudSpore() {
        return CloudValue;
    }

};

#endif // CINTERFACES_H


