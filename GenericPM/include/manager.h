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

class Manager : virtual public BasicInterface {
    protected:
        Manager();
        static Manager* instance;
        static std::vector<std::unique_ptr<Simulator>> simulators;
        static int plantingDate;     
        static std::vector<std::string> families;   
        static std::vector<CouplingPointID> couplingPointIDs;
        static std::vector<std::unique_ptr<CropInterface>> cropInterfaces;
        static std::vector<std::shared_ptr<CloudF>> cloudsF;
        static std::string outfileName;

        static bool outputStatus;
        
    public:
        static Manager* getInstance();
        static Manager* newInstance();

        void rate();
        void integration();
        void output();

        static Simulator* getSimulator(int index);
        static std::vector<std::unique_ptr<Simulator>>& getSimulators() {
            return simulators;
        }
        static void addSimulator(std::unordered_map<std::string, Expression> diseaseData, CropInterface *ci, 
                                 InjectionHolder rateInjections, InjectionHolder integrationInjections, InjectionHolder outputInjections);

        static void addCloudF(std::string family) {
            cloudsF.emplace_back(std::make_shared<CloudF>(family));
        }

        void updateCurrentYearDoy(int yearDoy);

        static void resetCloudsF() {
            for (auto& cloudF : cloudsF) {
                cloudF->reset();
            }
        }

        static std::shared_ptr<CloudF> getCloudF(std::string family) {
            for (auto& cF : cloudsF) {
                if (cF->getFamily() == family) {
                    return cF;
                }
            }
            return std::shared_ptr<CloudF>();
        }

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

        static void setPlantingDate(int YRPLT) {
            plantingDate = YRPLT;
        }

        static int getPlantingDate() {
            return plantingDate;
        }

        static std::string getOutfileName() {
            return outfileName;
        }
        
        static void setOutfileName(std::string name) {
            outfileName = name;
        }
        
        static void setCurrentSimDate(int yearDoy);

        static int getCurrentSimDate();

        static void addUniqueFamily(std::string fam) {
            families.push_back(fam);
        }

        static std::vector<std::string> getFamilies() {
            return families;
        }

        static void createCloudsF();
};

#endif // MANAGER_H