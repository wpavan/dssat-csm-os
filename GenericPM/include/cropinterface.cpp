#include "cropinterface.h"

#include<new>

CropInterface::CropInterface() {
}

CropInterface* CropInterface::instance = nullptr;

CropInterface* CropInterface::getInstance() {
    if (instance == nullptr)
        instance = new CropInterface();
    return instance;
}

CropInterface* CropInterface::newInstance() {
    instance = nullptr;
    return getInstance();
}
