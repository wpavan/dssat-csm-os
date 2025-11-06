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
#include "include/manager.h"
#include "include/injection.h"
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

// couplingInit refers to the seasonal initialization of the GDM
int couplingInit(int *YRDOY, int *YRPLT) {
    std::cout << "Running SEASINIT for year: " << *YRDOY / 1000 << std::endl;
    Manager *manager = Manager::getInstance();
    CouplingData *couplingData = CouplingData::getInstance();

    // Initialize the simulators with relevant data
    manager->setCurrentSimDate(*YRDOY);
    manager->setPlantingDate(*YRPLT);

    // Reset existing crop interface data year to year
    for (auto& s : manager->getSimulators()) {
        s->getCropInterface()->start();
    }

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
    // Get necessary instances for future operations
    FlexibleIO *fio = FlexibleIO::getInstance();
    Manager *manager = Manager::getInstance();
    CouplingData *couplingData = CouplingData::getInstance();

    couplingData->setCouplingValue(CouplingPointID::AREALF, AREALF);
    couplingData->setCouplingValue(CouplingPointID::CLW, CLW);
    couplingData->setCouplingValue(CouplingPointID::CSW, CSW);
    couplingData->setCouplingValue(CouplingPointID::PCLMT, PCLMT);
    couplingData->setCouplingValue(CouplingPointID::PCSTMD, PCSTMD);
    couplingData->setCouplingValue(CouplingPointID::PDLA, PDLA);
    couplingData->setCouplingValue(CouplingPointID::PLFAD, PLFAD);
    couplingData->setCouplingValue(CouplingPointID::PLFMD, PLFMD);
    couplingData->setCouplingValue(CouplingPointID::PSTMD, PSTMD);
    couplingData->setCouplingValue(CouplingPointID::PVSTGD, PVSTGD);
    couplingData->setCouplingValue(CouplingPointID::SLA, SLA);
    couplingData->setCouplingValue(CouplingPointID::SLDOT, SLDOT);
    couplingData->setCouplingValue(CouplingPointID::SSDOT, SSDOT);
    couplingData->setCouplingValue(CouplingPointID::STMWT, STMWT);
    couplingData->setCouplingValue(CouplingPointID::TDLA, TDLA);
    couplingData->setCouplingValue(CouplingPointID::VSTGD, VSTGD);
    couplingData->setCouplingValue(CouplingPointID::WLFDOT, WLFDOT);
    couplingData->setCouplingValue(CouplingPointID::WSTMD, WSTMD);
    couplingData->setCouplingValue(CouplingPointID::WTLF, WTLF);
    couplingData->setCouplingValue(CouplingPointID::TLFAD, TLFAD);
    couplingData->setCouplingValue(CouplingPointID::TLFMD, TLFMD);
    couplingData->setCouplingValue(CouplingPointID::VSTAGE, VSTAGE);
    couplingData->setCouplingValue(CouplingPointID::WLIDOT, WLIDOT);
    couplingData->setCouplingValue(CouplingPointID::CLAI, CLAI);
    couplingData->setCouplingValue(CouplingPointID::CLFM, CLFM);
    couplingData->setCouplingValue(CouplingPointID::CSTEM, CSTEM);
    couplingData->setCouplingValue(CouplingPointID::DISLA, DISLA);
    couplingData->setCouplingValue(CouplingPointID::DISLAP, DISLAP);
    couplingData->setCouplingValue(CouplingPointID::LAIDOT, LAIDOT);
    couplingData->setCouplingValue(CouplingPointID::WSIDOT, WSIDOT);
    couplingData->setCouplingValue(CouplingPointID::SDWT, SDWT);
    couplingData->setCouplingValue(CouplingPointID::WSDD, WSDD);
    couplingData->setCouplingValue(CouplingPointID::PSDD, PSDD);

    // Multiple instances of simulator to ensure multiple diseases can be run.
    // Because each simulator might couple with different coupling points,
    // we need to have a crop interface for each simulator.

    // Set the sowing/planting date
    if(manager->getPlantingDate() < 0) {
        manager->setPlantingDate(*YRPLT);
    }

    for (auto& s : manager->getSimulators()) {
        // Set the current YearDOY for next Disease step computation
        s->updateCurrentYearDoy(*YRDOY);
    }
    fio->setIntegerMemory("PEST", "YRDOY", *YRDOY);

    Weather::getInstance()->update();

    if (*YRPLT == *YRDOY) {
        Plant::newInstance();
    }

    manager->rate();
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
    Manager *manager = Manager::getInstance();
    CouplingData *couplingData = CouplingData::getInstance();

    manager->integration();
    couplingData->updatePrevValues();
    
    return (1);
}


int couplingOutput(int *doy) {
    // Get an instance of Simulator
    // Simulator *s = Simulator::getInstance();
    // Request disease outputs to be written in files
    // NOTE: Is this actually done here?
    Manager *manager = Manager::getInstance();
    manager->output();
    
    return (1);
}

// This is the entry point for the standalone GDM
int mainStandalone() {
    return 0;
}
