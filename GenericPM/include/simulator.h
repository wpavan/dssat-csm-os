/**
 * @file simulator.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * @author Virginia Covert (virginiacovert454@gmail.com)
 * 
 * @copyright Copyright (c) 2017–2026, University of Florida
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "basicinterface.h"
#include "cropinterface.h"
#include "initialcondition.h"
#include "injection.h"
#include "plant.h"
#include "manager.h"

#include <vector>
#include <filesystem>
#include <map>
#include <set>

bool diseaseHasOutput(std::shared_ptr<Disease> disease);

struct Inoculum {
    Inoculum(float disease_amt, float family_amt) : diseaseAmount(disease_amt), familyAmount(family_amt) {}
    float diseaseAmount;
    float familyAmount;
};

struct Output {
    std::string varName;
    float value;

    Output(std::string vn, float val) : varName(vn), value(val) {}
};

class Simulator : virtual public BasicInterface {
private:
    Utilities util;
    static std::vector<std::string> columnOrder;  // Track column order across all instances
    static std::map<std::string, int> columnWidths;  // Track width of each column
    static bool headerWritten;                     // Flag to write header only once

protected:
    InitialCondition* initialCondition;
    std::shared_ptr<Disease> disease;
    CropInterface* cropinterface;
    std::shared_ptr<CloudF> cloudF;

    std::vector<Output> loggedOutputs;
    std::map<std::string, float> currentDayOutputs;  // Map to accumulate outputs for a day
    std::vector<std::pair<int, std::map<std::string, float>>> bufferedRows;  // Buffer for all rows written so far
    std::set<std::string> initializedCustomOutputFiles;
    int lastExternalInoculumRate = -99;

    const bool hasOutput;
    std::string outputFileName;
    std::ofstream outputFile;

    void _create_CloudF() {
        this->cloudF = std::make_shared<CloudF>(disease);
    }

    void _create_InitialCondition() {
        if (!cloudF) {
            this->_create_CloudF();
        }
        this->initialCondition = new InitialCondition(this->cloudF);
    }

    void _del_InitialCondition() {
        delete initialCondition;
        initialCondition = nullptr;
    }

    void _del_CloudF() {
        cloudF = nullptr;
    }
    
public:
    Simulator();
    Simulator(std::shared_ptr<Disease> dis, CropInterface *ci) : disease(dis), cropinterface(ci), hasOutput(diseaseHasOutput(dis)) {
        outputFileName = Manager::getOutfileName() + "_" + disease->getDiseaseID() + ".OUT";
        std::filesystem::path outputPath(outputFileName);

        this->_create_CloudF();
        this->_create_InitialCondition();

        if (std::filesystem::exists(outputPath)) {
            outputPath = Utilities::safeRenameFile(outputPath);
            outputFileName = outputPath.filename().string();
        }
        // Note: Header will be written on first output() call with writeOutputHeader()
    };

    void inputPST_FromYaml();
    void inputPST();
    void integration();
    void output();
    void rate();
    void updateCurrentYearDoy(YearDoy yearDoy);
    bool allPlantsSenesced();

    void clearOutputLog();
    void logOutput(std::string varName, float value);
    void writeCustomOutput(const std::map<std::string, float>& customOutputs, int yearDoy);
    void formatAndWriteOutputRow(int yearDoy, const std::map<std::string, float>& outputs, bool newColumnsDiscovered = false);
    void rebuildOutputFile();
    void writeOutputHeader();
    std::string formatValue(float value, int width);
    
    // Methods to retrieve daily output metrics
    float getOrganCPValue();
    float getEvaluatedDamageValue();
    float getPlantInvisibleDiseaseArea();
    float getPlantVisibleDiseaseArea();
    float getPlantTotalLesionNumber();
    Inoculum getTotalInoculum();

    Plant* getPlant() {
        return Plant::getInstance();
    }

    void resetInitialCondition() {
        this->_del_InitialCondition();
        this->_create_InitialCondition();
    }

    void resetCloudF() {
        if (cloudF) {
            cloudF->reset();
        }
    }

    std::shared_ptr<CloudF> getCloudF() {
        return cloudF;
    }

    InitialCondition* getInitialCondition() {
        return initialCondition;
    }

    std::shared_ptr<Disease> getDisease() {
        return disease;
    }

    CropInterface* getCropInterface() {
        return cropinterface;
    }

    void setDisease(std::shared_ptr<Disease> disease) {
        this->disease = disease;

        this->_del_CloudF();
        this->_create_CloudF();

        this->_del_InitialCondition();
        this->_create_InitialCondition();
    }
};

#endif // SIMULATOR_H
