#ifndef RINTERFACE_H
#define RINTERFACE_H

#include<vector>
#include <cmath>
#include <iostream>


class CropInterface {
protected:
    CropInterface();
    static CropInterface *instance;
    int lastOrgan = 0;
    int plantingDate = 0;
    double newDailySenescenceArea = 0;
    bool newOrgan = false;
    std::vector<std::vector<double>> data;

public:
    static CropInterface* getInstance();
    static CropInterface* newInstance();

    void start() {
        lastOrgan = 0;
        plantingDate = 0;
        newOrgan = false;
        data.clear();
    }

    int hasNewOrgan() {
        if (newOrgan) {
            newOrgan = false;
            return data.size();
        }
        return 0;
    }

    int getOrgansQtd() {
        return data.size();
    }

    double getOrganArea(int organ) {
        return data[organ - 1][0];
    }

    double getSenescenceOrganArea(int organ) {
        return data[organ - 1][1];
    }

    double getPreviewsOrganArea(int organ) {
        return data[organ - 1][2];
    }

    double getPreviewsSenescenceOrganArea(int organ) {
        return data[organ - 1][3];
    }

    double getRatioDueDefoliation(int organ) const {
        return data[organ - 1][4];
        ;
    }

    void setOrganArea(int organ, double area) {
        //std::cout<<"DANO organ "<<organ<<" area "<<area<<std::endl;
        if (organ > data.size()) {
            std::vector<double> vetLine;
            vetLine.push_back(area); // Organ Area
            vetLine.push_back(0); // Senescence Area
            vetLine.push_back(area); // Previews Organ Area
            vetLine.push_back(0); // Previews Senescence Area
            vetLine.push_back(1); // Ratio Reduction Due Defoliation
            data.push_back(vetLine);
            newOrgan = true;

        } else {
            data[organ - 1][2] = data[organ - 1][0];
            data[organ - 1][0] = area;

        }
    }

    void setSenescenceOrganArea(int organ, double area) {
        if (organ > data.size()) {
            std::vector<double> vetLine;
            vetLine.push_back(0); // Organ Area
            vetLine.push_back(area); // Senescence Area
            vetLine.push_back(0); // Previews Organ Area
            vetLine.push_back(0); // Previews Senescence Area
            vetLine.push_back(1); // Ratio Reduction Due Defoliation
            data.push_back(vetLine);
            newOrgan = true;
        } else {
            data[organ - 1][3] = data[organ - 1][1];
            data[organ - 1][1] = area;
        }
    }

    /* Set Senescence area (cm2/m2) in a given day (DSSAT - PEST - SLDOT)
       This area will be used to remove organs as daily senescence (older first)
    */
    void setDailySenescenceArea(double area) {
        newDailySenescenceArea = area;
        double diff=0;
        for (unsigned int i = 0; i < data.size(); i++) {
            // set the senescence area to each organ and just call organ rate if it has area to be affected 
            //printf("A. Organ: %d TotalArea: %f SenescenceArea: %f DailySenesc: %f\n",i,data[i][0],data[i][1],newDailySenescenceArea);
            if(data[i][1] < data[i][0]) { // senescence < Organ Area
                if(newDailySenescenceArea>0) { // we need to set senescence area to organ
                    diff = fmin(data[i][0] - data[i][1],newDailySenescenceArea);
                    newDailySenescenceArea = newDailySenescenceArea-diff;
                    data[i][1] = data[i][1]+diff;
                }
            }
            //printf("B. Organ: %d TotalArea: %f SenescenceArea: %f DailySenesc: %f diff: %f\n",i,data[i][0],data[i][1],newDailySenescenceArea,diff);
        }
        /*
        for (unsigned int i = 0; i < organs.size(); i++) {
        o = &organs[i];
        // set the senescence area to each organ and just call organ rate if it has area to be affected 
        if(o->getSenescenceArea() < o->getTotalArea()) {
            if(Simulator::getInstance()->getCropInterface()->getDailySenescenceArea()>0) { // we need to set senescence area to organ
                diff = std::min(o->getTotalArea() - o->getSenescenceArea(),Simulator::getInstance()->getCropInterface()->getDailySenescenceArea());
                printf("Organ: %d TotalArea: %f SenescenceArea: %f DailySenesc: %f diff: %f\n",o->getOrganNumber(),o->getTotalArea(),o->getSenescenceArea(),Simulator::getInstance()->getCropInterface()->getDailySenescenceArea(),diff);
                Simulator::getInstance()->getCropInterface()->setDailySenescenceArea(Simulator::getInstance()->getCropInterface()->getDailySenescenceArea()-diff);
                o->setSenescenceArea(o->getSenescenceArea()+diff);
            }
            if(o->getSenescenceArea() < o->getTotalArea()) {
                o->rate();
            }
        }
    }
        */
    }
    /* Return the senescence area (cm2/m2) set (DSSAT - PEST - SLDOT)
       This represents the area that need to be removed (organs - older first)
    */
    double getDailySenescenceArea() {
        return(newDailySenescenceArea);
    }

    void setRatioDueDefoliation(int organ, double ratio) {
        if (organ > data.size()) {
            std::vector<double> vetLine;
            vetLine.push_back(0); // Organ Area
            vetLine.push_back(0); // Senescence Area
            vetLine.push_back(0); // Previews Organ Area
            vetLine.push_back(0); // Previews Senescence Area
            vetLine.push_back(ratio); // Ratio Reduction Due Defoliation
            data.push_back(vetLine);
            newOrgan = true;
        } else {
            data[organ - 1][4] = ratio; // Ratio Reduction Due Defoliation
        }
    }

    void setPlantingDate(int plantingDate) {
        this->plantingDate = plantingDate;
    }

    int getPlantingDate() const {
        return plantingDate;
    }




};

#endif // RINTERFACE_H
