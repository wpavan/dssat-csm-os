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
#include <fstream>
#include <cstdio>
#include <cmath>
#include <vector>
#include <cstring>

#include "include/project_config.h"

using namespace std;

extern "C" {
    // Coupling Functions
    int couplingInit(int *YRDOY, int *YRPLT);
    int couplingRate(int *YRDOY, float *AREALF, float *CLW, float *CSW,
        float *PCLMT, float *PCSTMD, float *PDLA, float *PLFAD, 
        float *PLFMD, float *PSTMD, float *PVSTGD, float *SLA, 
        float *SLDOT, float *SSDOT, float *STMWT, float *TDLA,
        float *VSTGD, float *WLFDOT, float *WSTMD, float *WTLF, 
        float *TLFAD, float *TLFMD, float *VSTAGE, float *WLIDOT, 
        float *CLAI, float *CLFM, float *CSTEM, float *DISLA, 
        float *DISLAP, float *LAIDOT, float *WSIDOT, float *SDWT, 
        float *WSDD, float *PSDD, int *DAS, int *YRPLT);
    int couplingIntegration(int *YRDOY,
        float *AREALF, float *CLW, float *CSW, float *PCLMT, float *PCSTMD,
        float *PDLA, float *PLFAD, float *PLFMD, float *PSTMD, float *PVSTGD,
        float *SLA, float *SLDOT, float *SSDOT, float *STMWT, float *TDLA,
        float *VSTGD, float *WLFDOT, float *WSTMD, float *WTLF, float *TLFAD,
        float *TLFMD, float *VSTAGE, float *WLIDOT, float *CLAI, float *CLFM,
        float *CSTEM, float *DISLA, float *DISLAP, float *LAIDOT, 
        float *WSIDOT, float *SDWT, float *WSDD, float *PSDD, int *DAS);
    int couplingOutput(int *doy);
    int logger(int *YRDOY, int *YRSIM, float *SL1, float *SLL1, float *SSAT1, 
        float *TMAX, float *TMIN, float *RAIN, float *SRAD, float *ZSTAGE);
}

// Coupling Functions Implementation 

// couplingInit refers to the seasonal initialization of the GDM
int couplingInit(int *YRDOY, int *YRPLT) {
    std::cout << "Running SEASINIT for year: " << *YRDOY / 1000 << std::endl;
    Manager *manager = Manager::getInstance();

    // Initialize the simulators with relevant data
    manager->setCurrentSimDate(*YRDOY);
    std::cout << "Incoming Planting Date (INIT): " << (YRPLT ? *YRPLT : -1) << std::endl;
    manager->setPlantingDate(*YRPLT);

    manager->resetCloudsF();

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

    // Set the sowing/planting date
    if(manager->getPlantingDate() < 0) {
        manager->setPlantingDate(*YRPLT);
    }

    manager->updateCurrentYearDoy(*YRDOY);
    fio->setIntegerMemory("PEST", "YRDOY", *YRDOY);

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
    Plant::getInstance()->updatePrev();

    #if ENABLE_ODE_GDM
    // Cohort-based damage accumulation using daily increments from ODE model
    // Static storage for cohorts and tracking per-simulation-year
    static std::vector<DiseaseCohort> cohorts;
    static double total_damaged_tissue = 0.0;
    static int last_YRDOY = -1;
    static int simulation_year = -1;

    FlexibleIO *fio = FlexibleIO::getInstance();
    int YRSIM = fio->getReal("PEST", "YRSIM");
    double ZSTAGE = fio->getReal("PEST", "ZSTAGE");
    if (simulation_year != YRSIM) {
        cohorts.clear();
        total_damaged_tissue = 0.0;
        simulation_year = YRSIM;
        last_YRDOY = -1;
    }

    double HSDWT = std::max(0.0, *SDWT - total_damaged_tissue);

    if (HSDWT > 0) {
        if (dIdt > 0.0) {
            DiseaseCohort new_cohort;
            new_cohort.infection_day = *YRDOY;
            new_cohort.age = 0;
            double B0 = 0.001;
            new_cohort.biomass = B0 * (dIdt);
            new_cohort.damaged_tissue = 0.0;
            cohorts.push_back(new_cohort);
        }

        double total_cohort_biomass = 0.0;
        double daily_new_damage = 0.0;

        for (auto& cohort : cohorts) {
            cohort.age = (*YRDOY - cohort.infection_day);
            double lag_slope = 1.0;
            double t_lag = 5.0;
            double r_max = 0.3;
            double Yld = 0.4;
            double activation = 1.0 / (1.0 + std::exp(-lag_slope * (cohort.age - t_lag)));
            double r_eff = r_max * activation;

            total_cohort_biomass = 0.0;
            for (const auto& c : cohorts) total_cohort_biomass += c.biomass;

            double growth_limit = 1.0 - (total_cohort_biomass / (Yld * HSDWT));
            growth_limit = std::max(0.0, std::min(1.0, growth_limit));

            double dB = r_eff * cohort.biomass * growth_limit;
            cohort.biomass += dB;

            double tissue_consumed = (1.0 / Yld) * dB;
            cohort.damaged_tissue += tissue_consumed;
            daily_new_damage += tissue_consumed;
        }

        total_damaged_tissue += daily_new_damage;
        *WSDD = total_damaged_tissue;
        if (total_damaged_tissue > HSDWT) total_damaged_tissue = HSDWT;
    }

    double total_biomass = 0.0;
    for (const auto& c : cohorts) total_biomass += c.biomass;

    printf("YRDOY: %i, ZSTAGE: %.4f, dIdt: %.4f, HSDWT: %.4f, total_damaged_tissue: %.4f, cohorts.size(): %zu, WSDD: %.6f, total_biomass: %.6f\n",
           *YRDOY, ZSTAGE, dIdt, HSDWT, total_damaged_tissue,
           cohorts.size(), *WSDD, total_biomass);
    #endif // ENABLE_ODE_GDM

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

static int YRSIMp = -1;

// coupling function for logging values in DSSAT
int logger(int *YRDOY, int *YRSIM, float *SL1, float *SLL1, float *SSAT1, 
    float *TMAX, float *TMIN, float *RAIN, float *SRAD, float *ZSTAGE) {

    std::string fileName = "gdm_environmental_logging_" + std::to_string(*YRSIM) + ".csv";

    float SW = 0.0f;
    if ((*SSAT1 - *SLL1) != 0.0f) {
        SW = std::min(100.0f, std::max(0.0f, (*SL1 - *SLL1) / (*SSAT1 - *SLL1)));
    }

    if (*YRSIM != YRSIMp) {
        std::remove(fileName.c_str());
        std::ofstream fout(fileName);
        fout << "YRDOY,Soil_Water(%),SRAD,TMAX,TMIN,RAIN,ZSTAGE\n";
        fout.close();

        YRSIMp = *YRSIM;
    }

    std::ofstream fout(fileName, std::ios::app);
    fout << *YRDOY << ',' << SW << ',' << *SRAD << ',' << *TMAX << ',' << *TMIN << ',' << *RAIN << ',' << *ZSTAGE << '\n';
    fout.close();
    return 1;
}

