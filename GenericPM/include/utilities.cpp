/**
 * @file utilities.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "utilities.h"
#include "disease.h"

#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

double Utilities::trapezoidalFunction(double value, double v[]) {
    if (value >= v[1] && value <= v[2])
        return 1;
    else if (value >= v[0] && value < v[1])
        return ((value - v[0]) / (v[1] - v[0]));
    else if (value > v[2] && value <= v[3])
        return 1 - ((value - v[2]) / (v[3] - v[2]));
    else
        return 0;
}

double Utilities::temperatureFavorability(double temp, double cardinalTemperatures[]) {
    double tempMax = cardinalTemperatures[0], 
           tempMin = cardinalTemperatures[1], 
           tempOpt = cardinalTemperatures[2];
    double tf, a, b;

    b = ((tempMax - tempOpt) / (tempOpt - tempMin));
    a = (1 / ((tempOpt - tempMin) * pow(tempMax - tempOpt, b)));
    if(temp > tempMax) temp = tempMax;
    if(temp < tempMin) temp = tempMin;
    tf = (a * (temp - tempMin) * pow(tempMax - temp, b));
    return (fmax(0,tf));
}

double Utilities::wetnessFavorability(double wetDuration, std::string wetnessFunction) {
    return Utilities::runExpression(wetnessFunction, wetDuration); 
}

double Utilities::growthFunction(double value, std::string expressionString) {
    return Utilities::runExpression(expressionString, value);
}

double Utilities::runExpressionFunction(double value, std::string expressionString) {
    return Utilities::runExpression(expressionString, value);
}

double Utilities::getHealthAreaProportion(double diseaseArea, double totalArea, double senescedArea) {
    double hap = 1 - ((diseaseArea+senescedArea) / totalArea);
    return (hap > 0 ? hap : 0);
}

std::string Utilities::formatDouble(double value) {
    return formatDouble(value, 2);
}

std::string Utilities::formatDouble(double value, int decimals) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(decimals) << value;
    return ss.str();
}

int Utilities::addOneDay(int yearDoy) {
    yearDoy++;
    int year = yearDoy / 1000;
    int doy = yearDoy - (year * 1000);
    if (doy <= 365 || (doy == 366 && isLeapYear(year))) {
        return yearDoy;
    } else {
        return (1 + ((year + 1)*1000));
    }
}

bool Utilities::isLeapYear(int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}
