/**
 * @file weather.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "weather.h"
#include "manager.h"
#include "../../FlexibleIO/Data/FlexibleIO.hpp"

#include <new>
#include <iostream>

Weather::Weather() {
}

Weather* Weather::instance = nullptr;

Weather* Weather::getInstance() {
    if (instance == nullptr) {
        instance = new Weather();
    }
    return instance;
}

bool Weather::update() {
    try {
        FlexibleIO *fio = FlexibleIO::getInstance();
        yearDoy = fio->getInteger("PEST", "YRDOY");
        //std::cout << "Weather::update YRDOY from PEST group: " << yearDoy << std::endl;
        
        year = yearDoy / 1000;
        doy = yearDoy - (year * 1000);
        
        sRad = fio->getRealYrdoy("WTH", std::to_string(yearDoy), "SRAD");
        tMax = fio->getRealYrdoy("WTH", std::to_string(yearDoy), "TMAX");
        tMin = fio->getRealYrdoy("WTH", std::to_string(yearDoy), "TMIN");
        rain = fio->getRealYrdoy("WTH", std::to_string(yearDoy), "RAIN");
        hRH90 = fio->getRealYrdoy("WTH", std::to_string(yearDoy), "RH90");
        rh = fio->getRealYrdoy("WTH", std::to_string(yearDoy), "RH");

        tMean = (tMax + tMin) / 2;
        par = 0;
        wetDur = 0;
        hWetDur = 0;
        rhMax = 0;
        rhMin = 0;
        //        getRealYrdoy(&yearDoy,(char *) "RHUM=",&rhMean);  //?????
        rhMean = 0;
    } catch (bool) {
        return false;
    }
    return true;
}

bool Weather::update(int _yearDoy) {
    try {
        FlexibleIO *fio = FlexibleIO::getInstance();
        //std::cout << "Weather::update YRDOY from PEST group: " << yearDoy << std::endl;
        yearDoy = _yearDoy;
        year = yearDoy / 1000;
        doy = yearDoy - (year * 1000);
        
        sRad = fio->getRealYrdoy("WTH", std::to_string(yearDoy), "SRAD");
        tMax = fio->getRealYrdoy("WTH", std::to_string(yearDoy), "TMAX");
        tMin = fio->getRealYrdoy("WTH", std::to_string(yearDoy), "TMIN");
        rain = fio->getRealYrdoy("WTH", std::to_string(yearDoy), "RAIN");
        hRH90 = fio->getRealYrdoy("WTH", std::to_string(yearDoy), "RH90");
        rh = fio->getRealYrdoy("WTH", std::to_string(yearDoy), "RH");

        tMean = (tMax + tMin) / 2;
        par = 0;
        wetDur = 0;
        hWetDur = 0;
        rhMax = 0;
        rhMin = 0;
        //        getRealYrdoy(&yearDoy,(char *) "RHUM=",&rhMean);  //?????
        rhMean = 0;
    } catch (bool) {
        return false;
    }
    return true;
}
