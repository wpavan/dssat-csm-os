#include "include/simulator.h"
#include<cmath>
#include "../FlexibleIO/Data/FlexibleIO.hpp"
using namespace std;

extern "C" {
    // Coupling Functions
    int couplingInit(int *YRDOY, int *YRPLT);
    int couplingRate(int *YRDOY,
            float *AREALF, float *CLW, float *CSW, float *PCLMT, float *PCSTMD,
            float *PDLA, float *PLFAD, float *PLFMD, float *PSTMD, float *PVSTGD,
            float *SLA, float *SLDOT, float *SSDOT, float *STMWT, float *TDLA,
            float *VSTGD, float *WLFDOT, float *WSTMD, float *WTLF,
            float *TLFAD, float *TLFMD, float *VSTAGE, float *WLIDOT,
            float *CLAI, float *CLFM, float *CSTEM, float *DISLA, float *DISLAP,
            float *LAIDOT, float *WSIDOT, float *SDWT, float *WSDD, 
            float *PSDD, int *DAS, int *YRPLT);
    int couplingIntegration(int *YRDOY,
            float *AREALF, float *CLW, float *CSW, float *PCLMT, float *PCSTMD,
            float *PDLA, float *PLFAD, float *PLFMD, float *PSTMD, float *PVSTGD,
            float *SLA, float *SLDOT, float *SSDOT, float *STMWT, float *TDLA,
            float *VSTGD, float *WLFDOT, float *WSTMD, float *WTLF,
            float *TLFAD, float *TLFMD, float *VSTAGE, float *WLIDOT,
            float *CLAI, float *CLFM, float *CSTEM, float *DISLA, float *DISLAP,
            float *LAIDOT, float *WSIDOT, float *SDWT, float *WSDD, 
            float *PSDD, int *DAS);
    int couplingOutput(int *doy);

}

float CLWp, SLAp, SDWTp, SW, SL1, SLL1, SSAT1, SDUL1, FSEED, first;

// Coupling Functions Implementation 

int couplingInit(int *YRDOY, int *YRPLT) {
    // Set the start day for Disease Model
    // Get an instance of Simulator
    Simulator *s = Simulator::newInstance();
    // Set the start day for Disease Model
    s->setCurrentYearDoy(*YRDOY);
    // Set the sowing/planting date
    s->getCropInterface()->setPlantingDate(*YRPLT);

    //printf("YRDOY: %i YRPLT: %i\n",
    //        *YRDOY, *YRPLT);
    CLWp=0; SLAp=0; SDWTp=0; FSEED = 0, first = 0;
    return (1);
}

int couplingRate(int *YRDOY,
        float *AREALF, float *CLW, float *CSW, float *PCLMT, float *PCSTMD,
        float *PDLA, float *PLFAD, float *PLFMD, float *PSTMD, float *PVSTGD,
        float *SLA, float *SLDOT, float *SSDOT, float *STMWT, float *TDLA,
        float *VSTGD, float *WLFDOT, float *WSTMD, float *WTLF,
        float *TLFAD, float *TLFMD, float *VSTAGE, float *WLIDOT,
        float *CLAI, float *CLFM, float *CSTEM, float *DISLA, float *DISLAP,
        float *LAIDOT, float *WSIDOT, float *SDWT, 
        float *WSDD, float *PSDD, int *DAS, int *YRPLT) {
    // Temporary variable used for computations 
    float temp = 0, newOrgan = 0, SW = 0, SL1 = 0, SLL1 = 0, SSAT1 = 0, SDUL1 = 0, TAVG = 0;
    double CloudField = 0;
    // Get an instance of Simulator
    Simulator *s = Simulator::getInstance();
    newOrgan = s->getCropInterface()->getOrgansQtd()+1;

    // Set the sowing/planting date
    if(s->getCropInterface()->getPlantingDate() < 0) {
        s->getCropInterface()->setPlantingDate(*YRPLT);
    }    
    // Set the current YearDOY for next Disease step computation
    s->updateCurrentYearDoy(*YRDOY);

    TAVG = FlexibleIO::getInstance()->getReal("PEST", "TAVG");
  
    //printf("YRDOY: %i YRPLT: %i\n",
    //        *YRDOY, *YRPLT);
    FlexibleIO::getInstance()->setIntegerMemory("PEST", "YRDOY", *YRDOY);
    if(*SDWT-SDWTp > 0){ //&& CloudField > 0){
        if(first == 0){
            FSEED = *YRDOY;
            FlexibleIO::getInstance()->setIntegerMemory("PEST", "FSEED", FSEED);
            first = 1;
        }
                  
        s->getCropInterface()->setOrganArea(newOrgan, (*SDWT-SDWTp));
        //printf("SDWT %f SDWTp %f *SDWT-SDWTp %f\n", *SDWT, SDWTp, *SDWT-SDWTp);
        if(s->getPlants().size()>0) {
            SL1 = FlexibleIO::getInstance()->getReal("PEST", "SL1");
            SLL1 = FlexibleIO::getInstance()->getReal("PEST", "SLL1");
            SDUL1 = FlexibleIO::getInstance()->getReal("PEST", "SDUL1");
            SSAT1 = FlexibleIO::getInstance()->getReal("PEST", "SSAT1");

            //TEMP
            SW = std::min(100.0f, std::max(0.0f, (SL1-SLL1)/(SSAT1-SLL1)*100));
            //CloudField = (0.0000005 * exp(0.20 * ((SL1*100)/SSAT1)));
            CloudField = (0.0000005 * exp(0.21 * SW));
            //printf("SDWT3 %f SDWTp %f *SDWT-SDWTp %f\n", *SDWT, SDWTp, *SDWT-SDWTp);
            
            std::cout << *YRDOY<<" CloudField2: "<<CloudField<<" SW:"<<SW<<" SL1:"<<SL1<<" SLL1: "<<SLL1<<" SSAT1: " << SSAT1 <<std::endl;
                    
            s->getPlants()[0].getCloudsP()[0].getCloudF()->addSporesCreated(CloudField);
            
            //get rain for if statement --
            //get phenological stage -- Flowering
            //0.022900 * day^3.612468 * exp(-0.464022 * day);
            

            //std::cout << "FIO: "<<SL1<<" "<<SSAT1<<" "<<SDUL1<<std::endl;
            //std::cout << "Data "<< *YRDOY<< " Alterando SPORES de: "<< s->getPlants()[0].getCloudsP()[0].getCloudF()->getValue() << " para: " << CloudField <<std::endl;     
        }
    }


    SDWTp = *SDWT;

    // Feed the Disease Model with weather information
    Weather::getInstance()->update();
    // Disease Simulator Rate
    s->rate();
    //printf("Returning rate\n");
    return (1);
}

int couplingIntegration(int *YRDOY,
        float *AREALF, float *CLW, float *CSW, float *PCLMT, float *PCSTMD,
        float *PDLA, float *PLFAD, float *PLFMD, float *PSTMD, float *PVSTGD,
        float *SLA, float *SLDOT, float *SSDOT, float *STMWT, float *TDLA,
        float *VSTGD, float *WLFDOT, float *WSTMD, float *WTLF,
        float *TLFAD, float *TLFMD, float *VSTAGE, float *WLIDOT,
        float *CLAI, float *CLFM, float *CSTEM, float *DISLA, float *DISLAP,
        float *LAIDOT, float *WSIDOT, float *SDWT, 
        float *WSDD, float *PSDD, int *DAS) {
    // Temporary variable used for computations 
    float dArea = 0,tArea=0,sArea=0; //,pclaCalc=0,pDArea=0;
     int seedAge = 0;
    
    // Get an instance of Simulator
    Simulator *s = Simulator::getInstance();


    // Call the Disease Model Integration function
    s->integration();
    //printf("YRDOY: %i Plant size: %f Organ size: %f\n", *YRDOY, s->getPlants().size(),s->getPlants()[0].getOrgans().size());
    //std::cout<<"s->getPlants().size() "<<s->getPlants().size()<<std::endl;
    //std::cout<<s->getPlants()[0].getOrgans().size()<<std::endl;
    if(s->getPlants().size() > 0 && s->getPlants()[0].getOrgans().size() > 0) {
        dArea = s->getPlants()[0].getDiseaseArea();
        tArea = s->getPlants()[0].getTotalArea();
        sArea = s->getPlants()[0].getSenescenceArea();
        seedAge = s->getPlants()[0].getOrgans().size();
        //pDArea = (dArea/(tArea-sArea)*100);

        //*PSDD = (dArea/tArea*5);
        *PSDD= ((dArea/tArea)*100); // PSDD       Percent of seed mass destroyed (%/d)
        //printf("ORIGINAL YRDOY: %i CloudF: %f PSDD %f\n",*YRDOY, s->getPlants()[0].getCloudsP()[0].getCloudF()->getValue(), PSDD);
        //printf("YRDOY: %i Plant Total Area: %f Disease Area: %f Senescence Area: %f AREALF: %f PDLA: %f PLFAD: %f\n", *YRDOY, tArea,dArea,sArea,*AREALF,*PDLA,*PLFAD);
        //printf("YRDOY: %i SDWT: %f PSDD: %f\n", *YRDOY, *SDWT, *PSDD);


    }
    
    //printf("YRDOY: %i PCLMT: %f\n", *YRDOY,*PCLMT);
    //printf("Returning integration\n");
    return (1);
}
int couplingOutput(int *doy) {
    // Get an instance of Simulator
    Simulator *s = Simulator::getInstance();
    // Request disease outputs to be written in files
    //s->output();
    
    return (1);
}