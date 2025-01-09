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

//extern FlexibleIO *flexibleio;
void Cloud::integration()
{
    int qtd = 0;
    if (sporesToBeRemoved > 0)
    { // Remove from cloud spores used to infect tissue
        removeSporesCloud(sporesToBeRemoved);
    }
    values.push_back(sporesCreated);
    sporesCreated = sporesToBeRemoved = 0;
}

double Cloud::getValue()
{
    double sum = 0;
    for (unsigned int i = 0; i < values.size(); i++) {
        sum += values[i];
    }
    return sum;
}

void Cloud::removeSporesCloud(double toBeRemoved)
{
    double total = getValue();
    for (unsigned int i = 0; i < values.size() && total > 0; i++)
    {
        values[i] -= (toBeRemoved * (values[i] / total));
    }
}

void Cloud::removeSporesCloudByRain(double percent)
{
    double oldValue = 0;
    for (unsigned int i = 0; i < values.size(); i++)
    {
        oldValue = values[i];
        values[i] = (oldValue * percent);
    }
}

void Cloud::removeSporesCloudPByAge(void)
{

    for (int i = values.size(); i > 0; i--)
    {
        values[values.size() - i] = values[values.size() - i] * (1.40 * exp(-0.2030 * (values.size() - i + 1))) > 0 ? values[values.size() - i] * (1.40 * exp(-0.2030 * (values.size() - i + 1))) : 0;
    }
}

void Cloud::removeSporesCloudOByAge(void)
{

    for (int i = values.size(); i > 0; i--)
    {
        values[values.size() - i] = values[values.size() - i] * (1.4268 * exp(-0.2184 * (values.size() - i + 1))) > 0 ? values[values.size() - i] * (1.4268 * exp(-0.2184 * (values.size() - i + 1))) : 0;
    }
}