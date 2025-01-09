/**
 * @file cinterface.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "include/simulator.h"
#include "../GenericPM-Spores/cinterfaceS.h"

#include <cmath>

using namespace std;

extern "C" {
    // Coupling Functions
    int couplingInit(int *YRDOY, int *YRPLT);
    int couplingRate(int *YRDOY,
            double *AREALF, double *CLW, double *CSW, double *PCLMT, double *PCSTMD,
            double *PDLA, double *PLFAD, double *PLFMD, double *PSTMD, double *PVSTGD,
            double *SLA, double *SLDOT, double *SSDOT, double *STMWT, double *TDLA,
            double *VSTGD, double *WLFDOT, double *WSTMD, double *WTLF,
            double *TLFAD, double *TLFMD, double *VSTAGE, double *WLIDOT,
            double *CLAI, double *CLFM, double *CSTEM, double *DISLA, double *DISLAP,
            double *LAIDOT, double *WSIDOT, double *SDWT, double *WSDD, 
            double *PSDD, int *DAS, int *YRPLT);
    int couplingIntegration(int *YRDOY,
            double *AREALF, double *CLW, double *CSW, double *PCLMT, double *PCSTMD,
            double *PDLA, double *PLFAD, double *PLFMD, double *PSTMD, double *PVSTGD,
            double *SLA, double *SLDOT, double *SSDOT, double *STMWT, double *TDLA,
            double *VSTGD, double *WLFDOT, double *WSTMD, double *WTLF,
            double *TLFAD, double *TLFMD, double *VSTAGE, double *WLIDOT,
            double *CLAI, double *CLFM, double *CSTEM, double *DISLA, double *DISLAP,
            double *LAIDOT, double *WSIDOT, double *SDWT, double *WSDD, 
            double *PSDD, int *DAS);
    int couplingOutput(int *doy);
}

double CLWp, SLAp, SDWTp, cloudFp;

// Coupling Functions Implementation 

int couplingInit(int *YRDOY, int *YRPLT) {
    // Set the start day for Disease Model
    // Get an instance of Simulator
    Simulator *s = Simulator::newInstance();
    // Set the start day for Disease Model
    s->setCurrentYearDoy(*YRDOY);
    // Set the sowing/planting date
    s->getCropInterface()->setPlantingDate(*YRPLT);

    //printf("Init - YRDOY: %i YRPLT: %i\n",
    //        *YRDOY, *YRPLT);
    CLWp=0; SLAp=0; SDWTp=0; cloudFp=0;
    return (1);
}

int couplingRate(int *YRDOY,
        double *AREALF, double *CLW, double *CSW, double *PCLMT, double *PCSTMD,
        double *PDLA, double *PLFAD, double *PLFMD, double *PSTMD, double *PVSTGD,
        double *SLA, double *SLDOT, double *SSDOT, double *STMWT, double *TDLA,
        double *VSTGD, double *WLFDOT, double *WSTMD, double *WTLF,
        double *TLFAD, double *TLFMD, double *VSTAGE, double *WLIDOT,
        double *CLAI, double *CLFM, double *CSTEM, double *DISLA, double *DISLAP,
        double *LAIDOT, double *WSIDOT, double *SDWT, 
        double *WSDD, double *PSDD, int *DAS, int *YRPLT) {

    // Temporary variable used for computations 
    double temp = 0, newOrgan = 0;
    double CloudField = 0;

    // Get an instance of Simulator
    //printf("Rate - \n");
    Simulator *s = Simulator::getInstance();
    SimulatorS *sS = SimulatorS::getInstanceS();

    newOrgan = s->getCropInterface()->getOrgansQtd()+1;
    //printf("YRDOY: %i ", *YRDOY);

    // Set the sowing/planting date
    if(s->getCropInterface()->getPlantingDate() < 0) {
        s->getCropInterface()->setPlantingDate(*YRPLT);
    }    

    // Set the current YearDOY for next Disease step computation
    s->updateCurrentYearDoy(*YRDOY);
    
    CloudField = (sS->getPlants()[0].getCloudsP()[0].getCloudF()->getValueS()); // - cloudFp);
    if(CloudField < 0) {
        CloudField = 0;
    }
    cloudFp = sS->getPlants()[0].getCloudsP()[0].getCloudF()->getValueS();

    //std::cout<<"sS->getPlants()[0].getCloudsP()[0].getCloudF()->getValueS(); "<<sS->getPlants()[0].getCloudsP()[0].getCloudF()->getValueS()<<std::endl;

    //CinterfaceSpore spores;
    ////CloudField = spores.couplingIntegrationSpore(*YRDOY);
    //spores.couplingIntegrationSpore(*YRDOY);
    //CloudField = spores.getcouplingCloudSpore();
    // Set the current Leaf area for a specific organ (one big leaf for awhile)
    //printf("WSIDOT %f SDWT: %f WSDD %f PSDD %f DAS %i YRPLT %i SDWTp %f *SDWT-SDWTp %f \n", 
    //                    *WSIDOT, *SDWT, *WSDD, *PSDD, *DAS, *YRPLT, SDWTp, *SDWT-SDWTp);
    if(*SDWT-SDWTp > 0){ //&& CloudField > 0){
        s->getCropInterface()->setOrganArea(newOrgan, (*SDWT-SDWTp));            
    }
    if(s->getPlants().size()>0) {
        //std::cout <<"Spores antes: "<<s->getPlants()[0].getCloudsP()[0].getCloudF()->getValue()<<std::endl;
        //std::cout << *YRDOY<< " Adicionado SPORES para CloudF: corrente: "<< s->getPlants()[0].getCloudsP()[0].getCloudF()->getValue() << " set to: " << CloudField <<std::endl;
        s->getPlants()[0].getCloudsP()[0].getCloudF()->setSporesCreated(CloudField);
        //s->getPlants()[0].getCloudsP()[0].getCloudF()->integration();
        //std::cout << *YRDOY<< " Atual: "<< s->getPlants()[0].getCloudsP()[0].getCloudF()->getValue() << std::endl;
    }

    SDWTp = *SDWT;

    // Feed the Disease Model with weather information
    Weather::getInstance()->update();
    // Disease Simulator Rate
    s->rate();
    return (1);
}

int couplingIntegration(int *YRDOY,
        double *AREALF, double *CLW, double *CSW, double *PCLMT, double *PCSTMD,
        double *PDLA, double *PLFAD, double *PLFMD, double *PSTMD, double *PVSTGD,
        double *SLA, double *SLDOT, double *SSDOT, double *STMWT, double *TDLA,
        double *VSTGD, double *WLFDOT, double *WSTMD, double *WTLF,
        double *TLFAD, double *TLFMD, double *VSTAGE, double *WLIDOT,
        double *CLAI, double *CLFM, double *CSTEM, double *DISLA, double *DISLAP,
        double *LAIDOT, double *WSIDOT, double *SDWT, 
        double *WSDD, double *PSDD, int *DAS) {
    // Temporary variable used for computations 
    double dArea = 0,tArea=0,sArea=0; //,pclaCalc=0,pDArea=0;
     int seedAge = 0;
    
    // Get an instance of Simulator
    Simulator *s = Simulator::getInstance();


    // Call the Disease Model Integration function
    s->integration();
    //printf("Integ - \n");
    //printf("YRDOY: %i ", *YRDOY);
    //printf("Plant size: %i ", s->getPlants().size());
    //if(s->getPlants().size() > 0) {
    //    printf("Organ size: %i\n", s->getPlants()[0].getOrgans().size());
    //} else {
    //    printf("\n");
    //}
    //printf("YRDOY: %i Plant size: %d Organ size: %f\n", *YRDOY, s->getPlants().size(),s->getPlants()[0].getOrgans().size());
    //std::cout<<"s->getPlants().size() "<<s->getPlants().size()<<std::endl;
    //std::cout<<s->getPlants()[0].getOrgans().size()<<std::endl;
    if(s->getPlants().size() > 0 && s->getPlants()[0].getOrgans().size() > 0) {
        dArea = s->getPlants()[0].getDiseaseArea();
        tArea = s->getPlants()[0].getTotalArea();
        sArea = s->getPlants()[0].getSenescenceArea();
        seedAge = s->getPlants()[0].getOrgans().size();
        //pDArea = (dArea/(tArea-sArea)*100);
        //printf("Int YRDOY: %i TArea: %f DArea: %f SArea: %f\n", *YRDOY, tArea,dArea,sArea);
        //*PSDD = (dArea/tArea*5);
        if(tArea > 0) {
            *PSDD = ((dArea/tArea)*15);
        } else {
            *PSDD = 0;
        }        
        //printf("ORIGINAL YRDOY: %i CloudF: %f PSDD %f\n",*YRDOY, s->getPlants()[0].getCloudsP()[0].getCloudF()->getValue(), PSDD);
        //printf("YRDOY: %i Plant Total Area: %f Disease Area: %f Senescence Area: %f AREALF: %f PDLA: %f PLFAD: %f\n", *YRDOY, tArea,dArea,sArea,*AREALF,*PDLA,*PLFAD);
        //printf("YRDOY: %i SDWT: %f PSDD: %f\n", *YRDOY, *SDWT, *PSDD);


    }
    return (1);
}

int couplingOutput(int *doy) {
    // Get an instance of Simulator
    Simulator *s = Simulator::getInstance();
    // Request disease outputs to be written in files
    return (1);
}