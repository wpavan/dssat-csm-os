/**
 * @file weather.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */
#ifndef WEATHER_H
#define WEATHER_H

class Weather {
protected:
    Weather();
    static Weather* instance;
    int yearDoy = 0;
    int year = 0;
    int doy = 0;
    double tMin = 0;
    double tMax = 0;
    double tMean = 0;
    double sRad = 0;
    double rain = 0;
    double par = 0;
    double wetDur = 0;
    double hWetDur = 0;
    double rh = 0;
    double rhMax = 0;
    double rhMin = 0;
    double rhMean = 0;
    int hRH90 = 0;

public:
    static Weather* getInstance();

    double getWetDur() {
        if (wetDur == 0)
            return hRH90;
        else
            return wetDur;
    }
    bool update();

    void setRain(double rain) {
        this->rain = rain;
    }

    double getRain() const {
        return rain;
    }

    void setTMean(double tMean) {
        this->tMean = tMean;
    }

    double getTMean() const {
        return tMean;
    }

    void setDoy(int doy) {
        this->doy = doy;
    }

    int getDoy() const {
        return doy;
    }

    void setYearDoy(int yearDoy) {
        this->yearDoy = yearDoy;
    }

    int getYearDoy() const {
        return yearDoy;
    }

    double getRh() const {
        return rh;
    }


};
#endif // WEATHER_H
