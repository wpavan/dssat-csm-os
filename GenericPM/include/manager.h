#ifndef MANAGER_H
#define MANAGER_H

#include <vector>

#include "simulator.h"
#include "cropinterface.h"

class Manager {
    protected:
        Manager();
        static Manager* instance;
        static std::vector<Simulator*> simulators;
        CropInterface *cropinterface;

    public:
        static Manager* getInstance();
        static Manager* newInstance();

        void readYAMLFile();

        static Simulator* getSimulator(int index);
        static std::vector<Simulator*>& getSimulators() {
            return simulators;
        }
        static void pushSimulator(Simulator* simulator);

        CropInterface* getCropInterface() {
            return cropinterface;
        }

        static void setCurrentSimDate(int yearDoy);
};

#endif // MANAGER_H