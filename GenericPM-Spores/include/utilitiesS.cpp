#include "utilitiesS.h"
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include "diseaseS.h"
#include <iostream>

float UtilitiesS::trapezoidalFunctionS(float value, float v[]) {
    if (value >= v[1] && value <= v[2])
        return 1;
    else if (value >= v[0] && value < v[1])
        return ((value - v[0]) / (v[1] - v[0]));
    else if (value > v[2] && value <= v[3])
        return 1 - ((value - v[2]) / (v[3] - v[2]));
    else
        return 0;
}

float UtilitiesS::temperatureFavorabilityS(float temp, float cardinalTemperatures[]) {
    float tempMax = cardinalTemperatures[0], 
           tempMin = cardinalTemperatures[1], 
           tempOpt = cardinalTemperatures[2];
    float tf, a, b;

    b = ((tempMax - tempOpt) / (tempOpt - tempMin));
    a = (1 / ((tempOpt - tempMin) * pow(tempMax - tempOpt, b)));
//    printf("a: %f b: %f temp: %f tempOpt: %f tempMin: %f tempMax: %f dif: %f\n",a,b,temp,tempOpt,tempMin,tempMax,tempMax - temp);
    // Protection against values out of the range
    if(temp > tempMax) temp = tempMax;
    if(temp < tempMin) temp = tempMin;
    tf = (a * (temp - tempMin) * pow(tempMax - temp, b));
    return (fmax(0,tf));
}

float UtilitiesS::wetnessFavorabilityS(float wetDuration, std::string wetnessFunction) {
    return UtilitiesS::runExpression(wetnessFunction, wetDuration);
}

float UtilitiesS::growthFunction(std::string expressionString, float value) {
    return UtilitiesS::runExpression(expressionString, value);
}

float UtilitiesS::dispersalRainFunctionS(float rain, std::string dispersionFreequency) {
    // NOTE: We need to revisit this function
    if(rain < 0) rain = 0;
    return UtilitiesS::runExpression(dispersionFreequency, rain);
}

float UtilitiesS::runExpressionFunctionS(float value, std::string expressionString) {
    return UtilitiesS::runExpression(expressionString, value);
}

float UtilitiesS::getHealthAreaProportionS(float diseaseArea, float totalArea, float senescedArea) {
    float hap = 1 - ((diseaseArea+senescedArea) / totalArea);
    return (hap > 0 ? hap : 0);
    //float hap = (1 - (diseaseArea / totalArea));
    //return (hap > 0 ? hap : 0);
}

std::string UtilitiesS::formatfloatS(float value) {
    return formatfloatS(value, 2);
}

std::string UtilitiesS::formatfloatS(float value, int decimals) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(decimals) << value;
    return ss.str();
}

int UtilitiesS::addOneDayS(int yearDoy) {
    yearDoy++;
    int year = yearDoy / 1000;
    int doy = yearDoy - (year * 1000);
    if (doy <= 365 || (doy == 366 && isLeapYearS(year))) {
        return yearDoy;
    } else {
        return (1 + ((year + 1)*1000));
    }
}

bool UtilitiesS::isLeapYearS(int year) {
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}