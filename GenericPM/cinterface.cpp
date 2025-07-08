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
#include "include/utilities.h"
#include "../FlexibleIO/Data/FlexibleIO.hpp"
//#include "../GenericPM-Spores/cinterfaceS.h"

#include <string>
#include <sstream>
#include <cmath>
#include <cstring>

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

float CLWp, SLAp, SDWTp, cloudFp;


// Declare variables used for FHB modifications:
float SW, SL1, SLL1, SSAT1, SDUL1, FSEED, first;


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

    // Set the initial values for the variables used in the coupling with the FHB model
    FSEED = 0; first = 0;

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
    float temp = 0, newOrgan = 0;
    float CloudField = 0;

    float SW = 0, SL1 = 0, SLL1 = 0, SSAT1 = 0, SDUL1 = 0, TAVG = 0;

    float codeOutput = 0.0f;

    // Get necessary instances for speed
    FlexibleIO *fio = FlexibleIO::getInstance();
    Simulator *s = Simulator::getInstance();

    newOrgan = s->getCropInterface()->getOrgansQtd()+1;

    // Set the sowing/planting date
    if(s->getCropInterface()->getPlantingDate() < 0) {
        s->getCropInterface()->setPlantingDate(*YRPLT);
    }    

    // Set the current YearDOY for next Disease step computation
    s->updateCurrentYearDoy(*YRDOY);
    
    TAVG = fio->getReal("PEST", "TAVG");

    fio->setIntegerMemory("PEST", "YRDOY", *YRDOY);
    
    if(*SDWT-SDWTp > 0){
        if(first == 0){
            FSEED = *YRDOY;
            fio->setIntegerMemory("PEST", "FSEED", FSEED);
            first = 1;
        }

        // NOTE: Unsure if this line is needed for the other module as well.
        s->getCropInterface()->setOrganArea(newOrgan, (*SDWT-SDWTp));

        if(s->getPlants().size()>0) {
            // Run rate function from yaml...
            /*
            SL1 = fio->getReal("PEST", "SL1");
            SLL1 = fio->getReal("PEST", "SLL1");
            SDUL1 = fio->getReal("PEST", "SDUL1");
            SSAT1 = fio->getReal("PEST", "SSAT1");

            SW = std::min(100.0f, std::max(0.0f, (SL1-SLL1)/(SSAT1-SLL1)*100));
            CloudField = Utilities::runExpressionFunction(SW, s->getPlants()[0].getCloudsP()[0].getDisease()->getSWF()); 
            */
            // 0.0000005*exp(0.20*x) 

            if (codeOutput < 0) {
                std::cerr << "Error in rate function execution." << std::endl;
                return -1;
            } else {
                CloudField = codeOutput;
            }

            s->getPlants()[0].getCloudsP()[0].getCloudF()->addSporesCreated(CloudField);
        }         
    }
    SDWTp = *SDWT;

    // Feed the Disease Model with weather information
    Weather::getInstance()->update();
    // Disease Simulator Rate
    // NOTE: If it makes more sense to have the rate function in the Simulator class, we can move it there.
    //       that way, more of the disease variables can be accessed directly.
    s->rate();
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
    float dArea = 0, tArea=0, sArea=0;
    int seedAge = 0;
    
    // Get an instance of Simulator
    Simulator *s = Simulator::getInstance();

    // Call the Disease Model Integration function
    s->integration();
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
    // NOTE: Is this actually done here?
    return (1);
}
