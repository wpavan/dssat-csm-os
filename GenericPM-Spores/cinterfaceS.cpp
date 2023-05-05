#include <iostream>
#include "include/simulatorS.h"
#include "include/utilitiesS.h"
#include <cmath>
#include <vector>
#include <fstream>
#include "../FlexibleIO/Data/FlexibleIO.hpp"
#include "cinterfaceS.h"


using namespace std;


vector<double> areaslf;
vector<double> outSpore;

double AREALF = UtilitiesS::runExpressionFunctionS(1,"50000");

extern "C"
{
    // Coupling Functionsa
    int couplingInitSpore(int *YRDOY, int *YRPLT);
    int couplingRateSpore(int *YRDOY);
    double couplingIntegrationSpore(int *YRDOY);
    int couplingOutputSpore(int *doy);
}

float CLWpS, SLApS;

// Coupling Functions Implementation

int couplingInitSpore(int *YRDOY, int *YRPLT)
{
    // Set the start day for Disease Model
    // Get an instance of Simulator
    SimulatorS *sS = SimulatorS::newInstanceS();
    // Set the start day for Disease Model
    sS->setCurrentYearDoy(*YRDOY);
    // Set the sowing/planting date
    sS->getCropInterface()->setPlantingDate(*YRPLT);
    sS->getCropInterface()->setOrganAreaS(1, AREALF);
    //printf("YRDOY: %i YRPLT: %i\n", *YRDOY, *YRPLT);

    CLWpS=0; SLApS=0;
    return (1);
}

int couplingRateSpore(int *YRDOY)
{
    // Temporary variable used for computations 
    float temp = 0, newOrgan = 0;
    double CloudField = 0;
    // Get an instance of Simulator
    SimulatorS *sS = SimulatorS::getInstanceS();
    newOrgan = sS->getCropInterface()->getOrgansQtd()+1;
    //std::cout<<"newOrgan "<<newOrgan<<" sS->getCropInterface()->getOrganArea(newOrgan) "
    //<<sS->getCropInterface()->getOrganArea(newOrgan-1)<<std::endl;
    sS->getCropInterface()->setOrganAreaS(1, AREALF);
    // Set the current YearDOY for next Disease step computation
    sS->updateCurrentYearDoyS(*YRDOY);

    //CinterfaceSpore spores;
    //spores.couplingIntegrationSpore(*YRDOY);
    //CloudField = spores.getcouplingCloudSpore();
    
    //ver pq ta vindo 0 e se realmenta ta chamando as funcoes pra estimar a nuvem de esporos
    
    // Set the current Leaf area for a specific organ (one big leaf for awhile)

    sS->getCropInterface()->setOrganAreaS(newOrgan, AREALF);

    
    // Feed the Disease Model with weather information
    WeatherS::getInstance()->updateS();
    // Disease Simulator Rate
    sS->rateS();
    
    //CloudField = couplingIntegrationSpore(*YRDOY);
    //std::cout<<*YRDOY<<" CLOUD FIELD "<<CloudField<< " AREALF "<< AREALF<<std::endl;
    
    return (1);
}

double couplingIntegrationSpore(int *YRDOY)
{
    // Temporary variable used for computations
    float dArea = 0, tArea = 0, pDArea = 0, sArea = 0, pclaCalc = 0;
    double CloudField = 0;

    // Get an instance of SimulatorS
    SimulatorS *sS = SimulatorS::getInstanceS();

    // Call the DiseaseS Model Integration function
    sS->integrationS();
    //std::cout<<*YRDOY<<" sS->getPlants().size() "<<sS->getPlants().size()<<std::endl;
    if (sS->getPlants().size() > 0 && sS->getPlants()[0].getOrgans().size() > 0)
    {
        dArea = sS->getPlants()[0].getDiseaseArea();
        tArea = sS->getPlants()[0].getTotalArea();
        sArea = sS->getPlants()[0].getSenescenceArea();
        pDArea = (dArea / (tArea - sArea) * 100);
        //printf("YRDOY: %i Plant Total Area: %f Disease Area: %f Senescence Area: %f  pDArea: %f\n", YRDOY, tArea,dArea,sArea,pDArea);
        if (pDArea > 99.9)
        {
            pDArea = 99.9;
        }
        for (int i = 0; i < sS->getPlants()[0].getOrgans().size(); i++)
        {
            if (sS->getPlants()[0].getOrgans().at(i).getVisibleLesionsS() /
                    (sS->getPlants()[0].getOrgans().at(i).getTotalArea() - sS->getPlants()[0].getOrgans().at(i).getSenescenceArea()) >=
                10)
            {
                pclaCalc += fmax(0, sS->getPlants()[0].getOrgans().at(i).getTotalArea() - sS->getPlants()[0].getOrgans().at(i).getSenescenceArea());
            }
        }
        CloudField = sS->getPlants()[0].getCloudsP()[0].getCloudF()->getValueS();
        //printf("pclaCalc: %f def: %f\n", pclaCalc, pclaCalc / tArea * 100);

    }
    //setcouplingCloudSpore(CloudField);
    std::ofstream out;    
    out.open("Daily_CloudF.txt", std::ofstream::out | std::ofstream::app);

    out << *YRDOY<< " " << CloudField << std::endl;
    out.close();
    
    //std::cout << "MODELO SPOROS -- Data "<< *YRDOY<< " NUVEM CAMPO " << CloudField <<std::endl;
    return (CloudField);
}

int couplingOutputSpore(int *doy)
{
    // Get an instance of SimulatorS
    SimulatorS *sS = SimulatorS::getInstanceS();
    // Request disease outputs to be written in files
    sS->outputS();

    return (1);
}

