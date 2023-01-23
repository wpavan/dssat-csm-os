#ifndef BASICS_H
#define BASICS_H

#include "weatherS.h"

#include<vector>
#include<string>
#include<fstream>

class BasicS {
protected:
    std::vector<std::string> output;
    WeatherS *weather;
    int firstCall = 1;

public:

    void getOutput(std::string fileName) {
        std::ofstream out;
//        if(firstCall) {
//            out.open(fileName);    
//            firstCall = 0;
//        } else {
            out.open(fileName, std::ofstream::out | std::ofstream::app);
//        }
        //out << fileName << std::endl;
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
        //out << fileName << std::endl;
        for (unsigned int i = 0; i < output.size(); i++)
            out << output[i] << std::endl;
        out.close();
        output.clear();
    }

    WeatherS* getWeather() {
        return weather = WeatherS::getInstance();
    }

};

#endif // BASIC_H
