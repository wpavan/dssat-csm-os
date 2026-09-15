/**
 * @file manager.h
 * 
 * @author Virginia Covert (virginiacovert454@gmail.com)
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2025–2026, University of Florida
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MANAGER_H
#define MANAGER_H

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include <yaml-cpp/yaml.h>

#include "basicinterface.h"
#include "coupling.h"
#include "cloudf.h"
#include "cropinterface.h"

class Simulator;
class Expression;

template<typename T1, typename T2, typename T3>
using triple = std::tuple<T1, T2, T3>;

struct CustomOutputSpec {
    std::string name;
    std::string expression;
    std::string desc;
};

struct InjectionHolder {
    // One injection is an endpoint, expression, and modification
    std::vector<triple<std::string, std::string, std::string>> injections;

    void add(std::string endpoint, std::string expression, std::string modification) {
        injections.emplace_back(endpoint, expression, modification);
    }

    void add(YAML::Node injectionNode) {
        for (const auto& injection : injectionNode) {
            this->add(injection.first.as<std::string>(),
                      injection["EXPRESSION"].as<std::string>(),
                      injection["MODIFICATION"].as<std::string>());
        }
    }
};

struct CustomOutputHolder {
    std::vector<CustomOutputSpec> outputs;
    std::string fileName;
    std::string format = "TABULAR";

    void add(std::string name, std::string expression, std::string desc = "") {
        outputs.push_back({name, expression, desc});
    }

    void setFileName(std::string name) {
        fileName = name;
    }

    void setFormat(std::string outputFormat) {
        format = outputFormat;
    }
};

class Manager : virtual public BasicInterface {
    protected:
        Manager();
        static Manager* instance;
        static std::vector<std::unique_ptr<Simulator>> simulators;
        static std::unordered_map<std::unique_ptr<Simulator>, std::string> simulatorFamilies;
        static YearDoy plantingDate;  
        static YearDoy currentGDMDate; // The current simulation date. Not updated with each CSM day, but each GDM day.   
        static std::vector<std::string> families;   
        static std::vector<CouplingPointID> couplingPointIDs;
        static std::vector<std::unique_ptr<CropInterface>> cropInterfaces;
        static std::string outfileName;

        static bool outputStatus;
        
    public:
        static Manager* getInstance();
        static Manager* newInstance();

        const bool familiesHas(std::string family) {
            return std::find(families.begin(), families.end(), family) != families.end();
        }

        void rate();
        void integration();
        void output();

        static Simulator* getSimulator(int index);
        static std::vector<std::unique_ptr<Simulator>>& getSimulators() {
            return simulators;
        }
        static void addSimulator(std::unordered_map<std::string, Expression> diseaseData, CropInterface *ci, 
                                 InjectionHolder rateInjections, InjectionHolder integrationInjections, InjectionHolder outputInjections,
                                 CustomOutputHolder customOutputs);

        void updateCurrentYearDoy(YearDoy yearDoy);

        static void addCropInterface(CouplingPointID cp, Expression ORGAN_AGE) {
            cropInterfaces.emplace_back(std::make_unique<CropInterface>(cp, ORGAN_AGE));
        }

        static void addCropInterface(CouplingPointID cp, Expression ORGAN_AGE, float value) {
            cropInterfaces.emplace_back(std::make_unique<CropInterface>(cp, ORGAN_AGE, value));
        }

        static std::vector<std::unique_ptr<CropInterface>>& getCropInterfaces() {
            return cropInterfaces;
        }

        static CropInterface* getCropInterface(CouplingPointID cp) {
            for (auto& ci : cropInterfaces) {
                if (ci->getOrganCP() == cp) {
                    return ci.get();
                }
            }
            return nullptr;
        }

        static void setCouplingPointIDs(const std::vector<CouplingPointID>& cpIDs) {
            couplingPointIDs = cpIDs;
        }

        static std::vector<CouplingPointID> getCouplingPointIDs() {
            return couplingPointIDs;
        }

        static void setOutputStatus(bool newStatus){
            outputStatus = newStatus;
        }

        static bool getOutputStatus(){
            return outputStatus;
        }

        static void setPlantingDate(YearDoy YRPLT) {
            plantingDate = YRPLT;
        }

        static YearDoy getPlantingDate() {
            return plantingDate;
        }

        static std::string getOutfileName() {
            return outfileName;
        }
        
        static void setOutfileName(std::string name) {
            outfileName = name;
        }
        
        static void setCurrentSimDate(YearDoy yearDoy);

        static YearDoy getCurrentSimDate();

        static void addUniqueFamily(std::string fam) {
            families.push_back(fam);
        }

        static std::vector<std::string> getFamilies() {
            return families;
        }
};

#endif // MANAGER_H
