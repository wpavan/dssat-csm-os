#ifndef MANAGER_H
#define MANAGER_H

#include <vector>
#include <unordered_map>
#include <string>

#include "coupling.h"
#include "simulator.h"
#include "cropinterface.h"

class Manager : virtual public BasicInterface {
    protected:
        Manager();
        static Manager* instance;
        static std::vector<Simulator*> simulators;
        static int plantingDate;
        static std::vector<CropInterface> cropInterfaces;
        static std::vector<CouplingPointID> couplingPointIDs;

    public:
        static Manager* getInstance();
        static Manager* newInstance();

        void rate();
        void integration();
        void output();

        static Simulator* getSimulator(int index);
        static std::vector<Simulator*>& getSimulators() {
            return simulators;
        }
        static void addSimulator(std::unordered_map<std::string, std::string> diseaseData, CropInterface *ci);

        static void addCropInterface(CouplingPointID cp) {
            cropInterfaces.emplace_back(cp);
        }

        static std::vector<CropInterface>& getCropInterfaces() {
            return cropInterfaces;
        }

        static CropInterface* getCropInterface(CouplingPointID cp) {
            for (auto& ci : cropInterfaces) {
                if (ci.getOrganCP() == cp) {
                    return &ci;
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

        static void setPlantingDate(int YRPLT) {
            plantingDate = YRPLT;
        }

        static int getPlantingDate() {
            return plantingDate;
        }

        static void setCurrentSimDate(int yearDoy);
};

#endif // MANAGER_H