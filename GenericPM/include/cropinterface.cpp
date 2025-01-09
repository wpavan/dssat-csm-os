/**
 * @file cropinterface.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "cropinterface.h"

#include <new>

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
