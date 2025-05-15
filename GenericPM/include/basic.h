/**
 * @file basic.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */
#ifndef BASIC_H 
#define BASIC_H

#include "weather.h"

#include <vector>
#include <string>
#include <fstream>

class Basic {
protected:
    std::vector<std::string> output;
    Weather *weather;
    int firstCall = 1;

public:
    void getOutput(std::string fileName) {
        std::ofstream out;
            out.open(fileName, std::ofstream::out | std::ofstream::app);
        for (unsigned int i = 0; i < output.size(); i++)
            out << output[i] << std::endl;
        out.close();
        output.clear();
    }

    void getOutput(std::string fileName, int updateFile) {
        std::ofstream out;
        if(updateFile) {
            out.open(fileName, std::ofstream::out | std::ofstream::app);            
        } else {
            out.open(fileName);    
        }
        
        for (unsigned int i = 0; i < output.size(); i++) {
            out << output[i] << std::endl;
        }
        out.close();
        output.clear();
    }

    Weather* getWeather() {
        return weather = Weather::getInstance();
    }
};

#endif // BASIC_H
