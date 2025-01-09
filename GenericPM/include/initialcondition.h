/**
 * @file initialcondition.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */
#ifndef INITIALCONDITION_H
#define INITIALCONDITION_H

#include "basic.h"
#include "basicinterface.h"
#include "cloudf.h"

class InitialCondition : public Basic, virtual public BasicInterface {
private:
    CloudF cloudf;

protected:
    double acumulateFavorability = 0, dailyFavorability = 0;
    bool stop = false;
    int doc = Basic::getWeather()->getDoy();
    static int qtd;
    int ID = ++qtd;

public:
    InitialCondition(Disease *disease) : cloudf{disease}
    {
        Basic::output.push_back("Day of Simulation, Acumulated Favorability");
    }

    int getID() {
        return ID;
    }

    CloudF* getCloud() {
        return &cloudf;
    }

    int getDoc() {
        return doc;
    }
    void rate();
    void integration();
    void output();

};

#endif // INITIALCONDITION_H
