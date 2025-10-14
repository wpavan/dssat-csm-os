/**
 * @file simulator.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "simulator.h"
#include "disease.h"
#include "cropinterface.h"
#include "initialcondition.h"
#include "weather.h"
#include "manager.h"
#include "../../FlexibleIO/Data/FlexibleIO.hpp"
#include "utilities.h"

#include <sstream>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>

/**
 * Simulator rate function
 * 
 * For each of the initial conditions and each of the plants in the simulation, call their 
 * rate functions. These calls propagate downwards (e.g. to organs).
 */
void Simulator::rate() {
    CouplingData *couplingData = CouplingData::getInstance(); 

    InitialCondition *ic;
    FlexibleIO *fio = FlexibleIO::getInstance();

    float CloudField;
    float newOrgan = cropinterface->getOrgansQtd()+1;

    // NOTE: This code (using the soil information) is for FHB, not WB
    //       We should get the WB code from GenericPM-Spores and parameterize
    //       a new disease in the YAML file which corresponds to external inoculum.
    //
    // This is also confirmed the only place that the spores were being
    // generated for real once the spores module was removed.
    /*if(plants.size()>0) {
        // Run rate function from yaml...
        SL1 = fio->getReal("PEST", "SL1");
        SLL1 = fio->getReal("PEST", "SLL1");
        SDUL1 = fio->getReal("PEST", "SDUL1");
        SSAT1 = fio->getReal("PEST", "SSAT1");

        SW = std::min(100.0f, std::max(0.0f, (SL1-SLL1)/(SSAT1-SLL1)*100));
        CloudField = Utilities::runExpressionFunction(SW, plants[0].getCloudsP()[0].getDisease()->getSWF()); 
        // 0.0000005*exp(0.20*x) 

        plants[0].getCloudsP()[0].getCloudF()->addSporesCreated(CloudField);
    }*/

    CouplingPointID organCP = this->disease->getOrganCP();
    if (organCP != CouplingPointID::VALUE) {
        float *organCPVal = couplingData->getCouplingValue(organCP);
        float organCPValPrev = couplingData->getCouplingValuePrev(organCP);
    
        // This should generically perform the organ addition step regardless of the coupling point.
        if (*organCPVal - organCPValPrev > 0) {
            // NOTE: Ask Dr. Pavan about this following commented code:
            /*if(first == 0){
                FSEED = *YRDOY;
                fio->setIntegerMemory("PEST", "FSEED", FSEED);
                first = 1;
            }*/
            
            #ifdef DEBUGX
            std::cout << "New organ growth detected: " 
                    << "Disease: " << this->disease->getDescription() 
                    << " Coupling Point: " << cpIDToStr(organCP) 
                    << " Organ number: " << newOrgan 
                    << " Growth: " << (*organCPVal - organCPValPrev) 
                    << std::endl;
            #endif // DEBUG

            cropinterface->setOrganArea(newOrgan, (*organCPVal - organCPValPrev));
            organCPValPrev = *organCPVal;
        }
    } else {
        #ifdef DEBUGX
            std::cout << "Constant value organ: " 
                    << "Disease: " << this->disease->getDescription() 
                    << " Coupling Point: " << cpIDToStr(organCP) 
                    << " Value: " << cropinterface->getConstValue()
                    << std::endl;
            #endif // DEBUG
    }

    /** For each Initial Condition call the rate function */
    initialCondition.rate();

    /** Call the rate function for the Plant */
    plant->rate();
}


void Simulator::integration() {
    float dArea = 0, tArea=0, sArea=0;
    int seedAge = 0;

    initialCondition.integration(disease);

    CouplingData *couplingData = CouplingData::getInstance();
    CouplingPointID cp = initialCondition.getCloud()->getDisease()->getDamageCP();

    plant->integration();

    if (plant != nullptr && plant->getOrgans().size() > 0) {
        dArea = plant->getDiseaseArea();
        tArea = plant->getTotalArea();
        sArea = plant->getSenescenceArea();
        seedAge = plant->getOrgans().size();
        //pDArea = (dArea/(tArea-sArea)*100);
        //printf("Int YRDOY: %i TArea: %f DArea: %f SArea: %f\n", *YRDOY, tArea,dArea,sArea);
        //*PSDD = (dArea/tArea*5);
        // NOTE: This needs to be recalibrated to remove hardcoded 
        //       values. And to only happen once because as it 
        //       currently stands, only the last value of PSDD will
        //       get sent back to DSSAT.
        if (tArea > 0) {
            couplingData->overwriteCouplingValue(cp, (dArea/tArea)*15);
        } else {
            couplingData->overwriteCouplingValue(cp, 0);
        }        
        //printf("ORIGINAL YRDOY: %i CloudF: %f PSDD %f\n",*YRDOY, s->getPlants()[0].getCloudsP()[0].getCloudF()->getValue(), PSDD);
        //printf("YRDOY: %i Plant Total Area: %f Disease Area: %f Senescence Area: %f AREALF: %f PDLA: %f PLFAD: %f\n", *YRDOY, tArea,dArea,sArea,*AREALF,*PDLA,*PLFAD);
        //printf("YRDOY: %i SDWT: %f PSDD: %f\n", *YRDOY, *SDWT, *PSDD);
    }
}

void Simulator::output() {
    initialCondition.output();
    plant->output();
}

/**
 * Synchronize the current DSSAT and simulator dates
 * 
 * This function takes the current YEARDOY from DSSAT and updates the simulator date accordingly. 
 * This can happen because DSSAT may run for days without calling the GDM, leading to a mismatch. 
 * This function runs the integration and rate functions of the simulator as well, meaning 
 * information is synchronized.
 * 
 * @param yearDoy The current DSSAT date
 */
void Simulator::updateCurrentYearDoy(int yearDoy) {
    // std::cout << "- Updating current YEARDOY from " << getCurrentYearDoy() << " to " << yearDoy << std::endl;
    while(util.addOneDay(getCurrentYearDoy()) < yearDoy) {
        setCurrentYearDoy(util.addOneDay(getCurrentYearDoy()));
        Weather::getInstance()->update();
        rate();
        integration();
    }
    setCurrentYearDoy(yearDoy);
    // std::cout << "- Current YEARDOY is now " << getCurrentYearDoy() << std::endl;
}
