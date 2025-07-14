#ifndef MANAGER_H
#define MANAGER_H

#include <vector>

#include "include/simulator.h"

class Manager {
    protected:
        Manager();
        static Manager* instance;
        static std::vector<Simulator*> simulators;

    public:
        static Manager* getInstance();
        static Manager* newInstance();

        static Simulator* getSimulator(int index);
        static void pushSimulator(Simulator* simulator);

        static void setCurrentSimDate(int yearDoy);
        static void setPlantingDate(int yearPlt);
};

#endif // MANAGER_H