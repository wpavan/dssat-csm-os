/**
 * @file simulator.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
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

bool diseaseHasOutput(Disease *disease);

struct Output {
    std::string varName;
    float value;

    Output(std::string vn, float val) : varName(vn), value(val) {}
};

class Simulator : virtual public BasicInterface {
private:
    Utilities util;

protected:
    static int currentYearDoy;
    InitialCondition initialCondition;
    Disease *disease;
    CropInterface *cropinterface;
    std::vector<Output> loggedOutputs;

    const bool hasOutput;
    std::string outputFileName;
    std::ofstream outputFile;

public:
    Simulator();
    Simulator(Disease *dis, CropInterface *ci) : initialCondition(dis->getFamily()), disease(dis), cropinterface(ci), hasOutput(diseaseHasOutput(dis)) {
        outputFileName = Manager::getOutfileName() + "_" + disease->getDiseaseID() + ".OUT";
        std::filesystem::path outputPath(outputFileName);

        if (std::filesystem::exists(outputPath)) {
            outputPath = Utilities::safeRenameFile(outputPath);
            outputFileName = outputPath.filename().string();
        }
        outputFile.open(outputPath, std::ios::app);
        outputFile << "YEARDOY\tOUTPUT_NAME\tVALUE\n";
        outputFile.close();
    };

    void inputPST_FromYaml();
    void inputPST();
    void integration();
    void output();
    void rate();
    void updateCurrentYearDoy(int yearDoy);
    bool allPlantsSenesced();

    void clearOutputLog();
    void logOutput(std::string varName, float value);

    Plant* getPlant() {
        return Plant::getInstance();
    }

    InitialCondition* getInitialCondition() {
        InitialCondition* ic = &initialCondition;
        return ic;
    }

    void setCurrentYearDoy(int currentYearDoy) {
        this->currentYearDoy = currentYearDoy;
    }

    int getCurrentYearDoy() const {
        return currentYearDoy;
    }

    Disease* getDisease() {
        return disease;
    }

    void setDisease(Disease *disease) {
        this->disease = disease;
        initialCondition = InitialCondition(disease->getFamily());
    }

    CropInterface* getCropInterface() {
        return cropinterface;
    }

    /*void resetCropInterface() {
        delete cropinterface;
        cropinterface = new CropInterface();
        cropinterface->start();
    }*/

    /*void setRateInjection(Injection *inj) {
        rateInj = inj;
    }

    Injection* getRateInjection() {
        if (rateInj == nullptr) {
            rateInj = new Injection();
        }
        return rateInj;
    }*/
};

#endif // SIMULATOR_H
