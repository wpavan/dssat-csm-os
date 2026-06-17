/**
 * @file cropinterface.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#ifndef RINTERFACE_H
#define RINTERFACE_H

#include "coupling.h"
#include "expression.h"
#include "../../FlexibleIO/Data/FlexibleIO.hpp"

#include <vector>
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

struct OrganData {
    float totalArea{0.0f};             // cm2/m2
    float senescenceArea{0.0f};        // cm2/m2
    float previousArea{0.0f};          // cm2/m2
    float previousSenescenceArea{0.0f}; // cm2/m2
    float ratioDueDefoliation{0.0f};   // ratio of area reduction due to defoliation (0-1)

    // Default constructor - all members already initialized above
    OrganData() = default;

    // Parameterized constructor using member initializer list
    OrganData(float t, float s, float p, float ps, float r)
        : totalArea(t)
        , senescenceArea(s)
        , previousArea(p)
        , previousSenescenceArea(ps)
        , ratioDueDefoliation(r) {}
};

class CropInterface {
protected:
    CouplingPointID organCP;
    float constValue = 0;
    int lastOrgan = 0;
    int plantingDate = 0;
    float newDailySenescenceArea = 0;
    bool newOrgan = false;
    // NOTE: I should make this not a vector but an ordered map or something similar for quick indexing by organ number. 
    std::vector<OrganData> data;

    Expression ORGAN_AGE;
public:
    CropInterface(CouplingPointID CP, Expression age_expr) : organCP(CP), ORGAN_AGE(age_expr) {};
    CropInterface(CouplingPointID CP, Expression ORGAN_AGE, float value) : organCP(CP), ORGAN_AGE(ORGAN_AGE), constValue(value) {};

    void start() {
        this->lastOrgan = 0;
        this->plantingDate = 0;
        this->newOrgan = false;
        try {
            this->data.clear();
            // Initialize with one element to avoid empty vector issues
            this->data.emplace_back(0.0f, 0.0f, 0.0f, 0.0f, 0.0f); // Use float literals to match OrganData's type
        } catch (const std::bad_alloc& e) {
            std::cerr << "Memory allocation failed in CropInterface::start(): " << e.what() << std::endl;
            throw; // Re-throw the exception after logging
        }
    }

    int hasNewOrgan() {
        FlexibleIO* fio = FlexibleIO::getInstance();
        if (newOrgan) {
            newOrgan = false;
            std::cout << "YEARDOY: " << fio->getInteger("CONTROL", "YEARDOY") << " newOrgan = true, size: " << data.size() << std::endl;
            showData();
            return data.size();
        }
        return 0;
    }

    Expression getORGAN_AGE() {
        return this->ORGAN_AGE;
    }

    void setORGAN_AGE(Expression ORGAN_AGE) {
        this->ORGAN_AGE = ORGAN_AGE;
    }

    CouplingPointID getOrganCP() {
        return organCP;
    }

    void setOrganCP(CouplingPointID organCP) {
        this->organCP = organCP;
    }

    int getConstValue() {
        return constValue;
    }

    int getOrgansQtd() {
        return data.size();
    }

    float getOrganArea(int organ) {
        return data[organ - 1].totalArea;
    }

    float getSenescenceOrganArea(int organ) {
        //std::cout << "Getting Senescence Organ Area: " << organ << std::endl;
        //std::cout << "= " << data[organ - 1][1] << std::endl;
        return data[organ - 1].senescenceArea;
    }

    float getPreviousOrganArea(int organ) {
        return data[organ - 1].previousArea;
    }

    float getPreviousSenescenceOrganArea(int organ) {
        return data[organ - 1].previousSenescenceArea;
    }

    float getRatioDueDefoliation(int organ) const {
        return data[organ - 1].ratioDueDefoliation;
    }

    void setOrganArea(int organ, float area) {
        //std::cout << "Setting Organ Area: " << organ << " set to " << area << std::endl;
        if (organ > data.size()) {
            // area Organ Area
            // 0    Senescence Area
            // area Previous Organ Area
            // 0    Previous Senescence Area
            // 1    Ratio Reduction Due Defoliation
            data.emplace_back(area, 0, area, 0, 1);
            newOrgan = true;
        } else {
            data[organ - 1].previousArea = data[organ - 1].totalArea;
            data[organ - 1].totalArea = area;
        }
    }

    void setSenescenceOrganArea(int organ, float area) {
        if (organ > data.size()) {
            // 0    Organ Area
            // area Senescence Area
            // 0    Previous Organ Area
            // area Previous Senescence Area
            // 1    Ratio Reduction Due Defoliation
            data.emplace_back(0, area, 0, area, 1);
            newOrgan = true;
        } else {
            data[organ - 1].previousSenescenceArea = data[organ - 1].senescenceArea;
            data[organ - 1].senescenceArea = area;
        }
    }

    /* Set Senescence area (cm2/m2) in a given day (DSSAT - PEST - SLDOT)
       This area will be used to remove organs as daily senescence (older first)
    */
    void setDailySenescenceArea(float area) {
        newDailySenescenceArea = area;
        float diff=0;
        for (auto& organData : data) {
            // set the senescence area to each organ and just call organ rate if it has area to be affected 
            if(organData.senescenceArea < organData.totalArea) {
                if(newDailySenescenceArea > 0) { // then we need to set senescence area to organ
                    diff = fmin(organData.totalArea - organData.senescenceArea, newDailySenescenceArea);
                    newDailySenescenceArea = newDailySenescenceArea - diff;
                    organData.senescenceArea += diff;
                }
            }
        }
    }

    /* Return the senescence area (cm2/m2) set (DSSAT - PEST - SLDOT)
       This represents the area that need to be removed (organs - older first)
    */
    float getDailySenescenceArea() {
        return(newDailySenescenceArea);
    }

    void setRatioDueDefoliation(int organ, float ratio) {
        if (organ > data.size()) {
            // 0    Organ Area
            // 0    Senescence Area
            // 0    Previous Organ Area
            // 0    Previous Senescence Area
            // ratioRatio Reduction Due Defoliation
            data.emplace_back(0, 0, 0, 0, ratio);
            newOrgan = true;
        } else {
            data[organ - 1].ratioDueDefoliation = ratio; // Ratio Reduction Due Defoliation
        }
    }

    void showData() {
        std::cout << "CropInterface Data:" << std::endl;
        // Initialize a 2-digit, 0-padded, index counter
        size_t index = 0; 

        std::ostringstream padded;

        for (auto& organData : data) {
            padded.str("");
            padded.clear();
            padded << std::setw(2) << std::setfill('0') << index + 1;

            std::cout << "Organ " << padded.str() << ": Area: " << organData.totalArea
                      << ", Senescence Area: " << organData.senescenceArea
                      << ", Previous Area: " << organData.previousArea
                      << ", Previous Senescence Area: " << organData.previousSenescenceArea
                      << ", Ratio Due Defoliation: " << organData.ratioDueDefoliation
                      << std::endl;
            index++;
        }
    }
};

#endif // RINTERFACE_H
