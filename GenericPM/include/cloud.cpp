/**
 * @file cloud.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "cloud.h"
#include "disease.h"
#include "simulator.h"
//#include "../../FlexibleIO/Data/FlexibleIO.hpp"

#include <cmath>
#include <iostream>

void Cloud::integration() {
    int qtd = 0;
    if (sporesToBeRemoved > 0) {
        // Remove from cloud spores used to infect tissue
        removeSporesCloud(sporesToBeRemoved);
    }
    values.push_back(sporesCreated);
    sporesCreated = sporesToBeRemoved = 0;
}

float Cloud::getValue() {
    float sum = 0;
    for (auto& value : values) {
        sum += value;
    }
    return sum;
}

void Cloud::removeSporesCloud(float toBeRemoved) {
    float total = getValue();
    for (auto& value : values) {
        value -= (toBeRemoved * (value / total));
    }
}

void Cloud::removeSporesCloudByRain(float percent) {
    for (auto& value : values) {
        value *= percent;
    }
}