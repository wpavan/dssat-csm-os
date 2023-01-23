#include "cropinterfaceS.h"

#include<new>

CropInterfaceS::CropInterfaceS() {
}

CropInterfaceS* CropInterfaceS::instance = nullptr;

CropInterfaceS* CropInterfaceS::getInstance() {
    if (instance == nullptr)
        instance = new CropInterfaceS();
    return instance;
}

CropInterfaceS* CropInterfaceS::newInstanceS() {
    instance = nullptr;
    return getInstance();
}
