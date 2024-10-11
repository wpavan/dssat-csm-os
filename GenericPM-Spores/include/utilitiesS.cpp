#include "utilitiesS.h"
#include<cmath>
#include<string>
#include<sstream>
#include<iomanip>
#include "diseaseS.h"
#include<iostream>

double UtilitiesS::trapezoidalFunctionS(double value, double v[]) {
    if (value >= v[1] && value <= v[2])
        return 1;
    else if (value >= v[0] && value < v[1])
        return ((value - v[0]) / (v[1] - v[0]));
    else if (value > v[2] && value <= v[3])
        return 1 - ((value - v[2]) / (v[3] - v[2]));
    else
        return 0;
}

double UtilitiesS::temperatureFavorabilityS(double temp, double cardinalTemperatures[]) {
    double tempMax = cardinalTemperatures[0], 
           tempMin = cardinalTemperatures[1], 
           tempOpt = cardinalTemperatures[2];
    double tf, a, b;

    b = ((tempMax - tempOpt) / (tempOpt - tempMin));
    a = (1 / ((tempOpt - tempMin) * pow(tempMax - tempOpt, b)));
    tf = (a * (temp - tempMin) * pow(tempMax - temp, b));
    return (fmax(0,tf));
}

double UtilitiesS::wetnessFavorabilityS(double wetDuration, std::string wetnessFunction) {
//    double wf;
    double wf, k = 0.99374, b = 350.81064, r = 0.88591;
    //wf = (k / (1 + b * exp(-r * wetDuration)));
    wf = UtilitiesS::growthFunction(wetnessFunction, wetDuration);
    //std::cout<<"WF: "<<wf<<std::endl;
    //std::cout<<"WF: "<<wf<<" wetnessFunction: "<<wetnessFunction<<" wetDuration: "<<wetDuration<<std::endl;

    //printf("WF: %f Wetness Function: %s WetDuration: %f",wetnessFunction, wf,wetDuration);  
    return wf;
}

double UtilitiesS::dispersalRainFunctionS(double rain, std::string dispersionFreequency) {
    double dre;
    dre = UtilitiesS::growthFunction(dispersionFreequency, rain);
    //std::cout<<"DRE: "<<dre<<" Rain: "<<rain<<" DisperifonF: "<<dispersionFreequency<<std::endl;
    return dre;
}
double UtilitiesS::runExpressionFunctionS(double value, std::string expressionString) {
    return UtilitiesS::runExpression(expressionString, value);
}

double UtilitiesS::getHealthAreaProportionS(double diseaseArea, double totalArea, double senescedArea) {
        double hap = 1 - ((diseaseArea+senescedArea) / totalArea);
        return (hap > 0 ? hap : 0);
    //double hap = (1 - (diseaseArea / totalArea));
    //return (hap > 0 ? hap : 0);
}

std::string UtilitiesS::formatDoubleS(double value) {
    return formatDoubleS(value, 2);
}

std::string UtilitiesS::formatDoubleS(double value, int decimals) {
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