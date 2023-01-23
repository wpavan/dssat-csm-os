#include "cloud.h"
#include "disease.h"
#include <cmath>
#include <iostream>
#include "simulator.h"
//#include "../Coupling-Interface/CPP/coupling_IO_cpp.h"
//#include "../../FlexibleIO/Data/FlexibleIO.hpp"

//extern FlexibleIO *flexibleio;
void Cloud::integration()
{
    int qtd = 0;
    if (sporesToBeRemoved > 0)
    { // Remove from cloud spores used to infect tissue
        removeSporesCloud(sporesToBeRemoved);
        /*        for (unsigned int i = 0; i < values.size() && sporesToBeRemoved>0; i++) {
            qtd = fmin(values[i],sporesToBeRemoved);
            sporesToBeRemoved -= qtd;
            values[i] = values[i] - qtd;
        }*/
    }
    //std::cout<<"Rain: "<< Basic::getWeather()->getRain()<<std::endl;
    //std::cout<<"New function: "<< Utilities::dispersalRainFunction(Basic::getWeather()->getRain(),disease->getDispersionFreequency())<<std::endl;
    values.push_back(Utilities::dispersalRainFunction(Basic::getWeather()->getRain(), disease->getDispersionFreequency()) * sporesCreated);
    //std::cout << disease->getDispersionFreequency() * sporesCreated << std::endl;
    sporesCreated = sporesToBeRemoved = 0;
}

double Cloud::getValue()
{
    double sum = 0;
    for (unsigned int i = 0; i < values.size(); i++)
        sum += values[i];

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

void Cloud::removeSporesCloudFByAge(void)
{
    int yearDoy = Simulator::getInstance()->getCurrentYearDoy();

    for (int i = values.size(); i > 0; i--)
    {
        values[values.size() - i] = values[values.size() - i] * (1.61 * exp(-0.369 * (values.size() - i + 1))) > 0 ? values[values.size() - i] * (1.61 * exp(-0.369 * (values.size() - i + 1))) : 0;
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