/*=======================================================================
  InputWeather.cpp, Felipe de Vargas, Willingthon Pavan, Fabio Oliveira
  Function to read Weather header and data.
-----------------------------------------------------------------------
  REVISION HISTORY
  09/01/2016 FV Written.
  06/18/2017 FV Create new version for weather data.
	12/11/2017 FO Added new date format.
	12/11/2017 FO Added variable to return error code.
	03/29/2018 FO Added FlexibleIO get/set to manipulate memory data.
========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "../Util/Util.hpp"
#include "../Data/FlexibleIO.hpp"

#define DEFREAD 365;
#define DEFPIECE 50;

extern "C" {
    int readw_(char *fileW, int *YRDOYWY, int *YRDOY, int *YRSIM, int *MULTI, int *ENDFILE, char *fileX, char *modelW, int *DAYB, int *ERRNUM);
}

std::string fileXWTH;
int CFILEH = 0;
int lineWeather = 0;
int DAYS = 0;
int DAYSREAD = 0, lastDay = 0;
bool foundSoi, eof = false, wheader = false, plantOutTitle = true;

struct HeaderData
{
    std::string varname;
    std::string section;
    std::string type;
    int size;
    int length;
};
std::vector<HeaderData> weatherInp;

int readWeatherH(std::string file)
{
    std::string line, varname, sec;
    int test, type = 0;

    std::ifstream myfile(file);
    if(!myfile)
    {
        return -99;
    }

    if (myfile.is_open())
    {
        while (getline (myfile, line) )
        {
            test =  Util::ignore_line3(line);
            if (test == 3)
            {
                type++;
                line.erase(0, 1);
                std::stringstream ss(line);
                if (type == 1)
                {
                    sec = "HEADER";
                }
                else
                {
                    sec = "DATA";
                }

                while (ss >> varname)
                {
                    if(varname[0] != '!')
                    {
                        HeaderData wea;
                        wea.varname = varname;
                        wea.section = sec;
                        weatherInp.push_back(wea);
                    }

                }
            }
        }

        int isect = -1;
        for (int i = 0; i < weatherInp.size(); i++)
        {
            if (weatherInp[i].section.compare("DATA") == 0 && isect == -1)
            {
                isect = i;
            }
        }
        return isect;
    }
}
std::string lastFile;

int readw_(char *fileW, int *YRDOYWY, int *YRDOY, int *YRSIM, int *MULTI, int *ENDFILE, char *fileX, char *modelW, int *DAYB, int *ERRNUM)
{
    int yrdoywy = *YRDOYWY, yrdoy = *YRDOY - *DAYB, yrsim = *YRSIM, findDate;
    int changeSec;
    std::string file(fileW), filexx(fileX), model(modelW);
    FlexibleIO *flexibleio = FlexibleIO::getInstance();

    if (fileXWTH.compare(filexx) != 0 || file.compare(lastFile) != 0)
    {
        fileXWTH = filexx;
        flexibleio->eraseGroupMemory("WTH");
        CFILEH = 0;
        lineWeather = 0;
        DAYS = 0;
        DAYSREAD = 0;
        lastDay = 0;
        eof = false;
        wheader = false;
        weatherInp.clear();

    }

    filexx = filexx.substr(0, filexx.find(".") + 1);
    model = model.substr(0, 5);
    filexx += "json";

    if (yrdoywy <= 0 )
        yrdoy--;

    file.erase(file.find(".WTH") + 4, file.size());
    bool readFile = true;
    std::string subsec, aux, line;

    int countHeader = 0, sec = 0;
    std::vector<int> indices;

    if (CFILEH == 0 )
    {
        lastFile = file;
        changeSec = readWeatherH(file);
        if(changeSec == -99)
        {
            *ERRNUM = 30;
            return 0;
        }
    }

    bool fileExist = false;

    for (int i = 0; i < CFILEH; i++)
    {
        if(flexibleio->getChar("WTH", "fileWTH" + std::to_string(i)) == file)
        {
            fileExist = true;
        }
    }

    if (!fileExist)
    {
        flexibleio->setCharMemory("WTH", "fileWTH" + std::to_string(CFILEH), file);
        CFILEH++;
        wheader = false;
        eof = false;
        DAYSREAD = 0;
    }
    else
    {
        DAYS = DEFPIECE;
    }
    if (DAYS <= 0)
        DAYS = DEFREAD;

    if (DAYSREAD > 0  && !eof)
    {
        DAYS = DEFPIECE;
        DAYSREAD = 0;

        if (Util::Y2K_DOY(lastDay) > yrdoy)
        {
            lastDay += 1;
            yrdoy = Util::Y2K_DOY(lastDay);
        }

    }

    subsec = weatherInp[0].section;


    for (int i = 0; i < weatherInp.size(); i++)
    {
        if (subsec.compare(weatherInp[i].section) != 0)
        {
            subsec = weatherInp[i].section;
            sec++;
            indices.push_back(i);
        }
    }
    for (int i = indices[0]; i < weatherInp.size() ; i++)
    {
        if (weatherInp[i].varname.compare("DATE") == 0)
        {
            findDate = i;
        }
    }
    if (eof)
    {
        *ENDFILE = 1;
        return 1;
    }
    else
    {
        *ENDFILE = 0;
    }

    std::vector<std::string> vecDateidentified;
    std::vector<std::string> vecDateUnidentified;
    static int dateReadFormat = 0; //1 mean dd/mm - 0 mean mm/dd
    int checkDateFormat = 0;// if checkDateFormat is != 0 then file WTH has errors.
    int sizeVec = 0;

    std::ifstream myfile(file);
    if(!myfile)
    {
        *ERRNUM = 29;
        return 0;
    }

    int isect = 0;
    if (myfile.is_open())
    {
        while (true)
        {
            if (!getline (myfile, line))
            {
                eof = true;
                *ENDFILE = 1;

                while(!vecDateUnidentified.empty())
                {
                    checkDateFormat = Util::checkDate(dateReadFormat, vecDateidentified, vecDateUnidentified[findDate - indices[0]]);

                    if(checkDateFormat == 0)
                    {
                        if (strtol(vecDateUnidentified[findDate - indices[0]].c_str(), NULL, 10) >= yrdoy)
                        {

                            for (int i = 0; i < sizeVec; i++)
                            {
                                if(vecDateUnidentified[i].compare(".") == 0)
                                    vecDateUnidentified[i] = "-99";

                                flexibleio->setCharYrdoyMemory("WTH", vecDateUnidentified[findDate - indices[0]], weatherInp[i + indices[0]].varname, vecDateUnidentified[i]);

                                //std::cout << vecDateUnidentified[findDate - indices[0]] + "_" + weatherInp[i + indices[0]].varname << " : "
                                //					<< flexibleio->getCharYrdoy("WTH", vecDateUnidentified[findDate - indices[0]], weatherInp[i + indices[0]].varname) << std::endl;
                            }

                            if(lastDay < strtol(vecDateUnidentified[findDate - indices[0]].c_str(), NULL, 10))
                                lastDay = strtol(vecDateUnidentified[findDate - indices[0]].c_str(), NULL, 10);
                        }
                    }
                    else
                    {
                        *ERRNUM = checkDateFormat;
                        return 0;
                    }

                    vecDateUnidentified.erase(vecDateUnidentified.begin(), vecDateUnidentified.begin()+sizeVec);
                }

                break;
            }

            line = Util::trim(line);
            if (Util::ignore_line2(line) == 1)
            {
                aux = line;
                if (isect < indices[0])
                {
                    if (!wheader)
                    {
                        for (isect; isect < indices[0]; isect++)
                        {
                            if (weatherInp[isect].type.compare("STRING") == 0)
                            {
                                aux = Util::trim(aux);
                                if (weatherInp[isect].length + 1 <= aux.size())
                                {
                                    flexibleio->setCharMemory("WTH", weatherInp[isect].varname, aux.substr(0, weatherInp[isect].length + 1));
                                }
                                else
                                {
                                    flexibleio->setCharMemory("WTH", weatherInp[isect].varname, aux.substr(0, aux.size()));
                                }

                                //std::cout << weatherInp[isect].varname << " : " << flexibleio->getChar("WTH", weatherInp[isect].varname) << std::endl;
                                aux = aux.substr(weatherInp[isect].length, aux.size());

                            }
                            else
                            {
                                aux = Util::trim(aux);
                                std::stringstream ss (aux);
                                std::string str;
                                ss >> str;
                                flexibleio->setCharMemory("WTH", weatherInp[isect].varname, str );
                                //std::cout << weatherInp[isect].varname << " : " << flexibleio->getChar("WTH", weatherInp[isect].varname) << std::endl;
                                aux = Util::trim(aux);
                                aux.erase(0, aux.find_first_of(" "));
                            }
                        }
                        wheader = true;
                    }
                    else
                        isect = changeSec;
                }
                else
                {
                    if (DAYSREAD <= DAYS)
                    {
                        std::vector<std::string> vectorAux;
                        for (int i = isect; i < weatherInp.size(); i++)
                        {
                            if (weatherInp[i].type.compare("STRING") == 0)
                            {
                                aux = Util::trim(aux);
                                if (weatherInp[i].length + 1 <= aux.size())
                                    vectorAux.push_back(aux.substr(0, weatherInp[i].length + 1));
                                else
                                    vectorAux.push_back(aux.substr(0, aux.size()));

                                aux = aux.substr(weatherInp[i].length, aux.size());
                            }
                            else
                            {
                                aux = Util::trim(aux);
                                std::string auxNumeric;
                                std::stringstream ss (aux);
                                ss >> auxNumeric;
                                vectorAux.push_back(auxNumeric);
                                aux = Util::trim(aux);
                                aux.erase(0, aux.find_first_of(" "));
                            }
                        }

                        std::string ss = vectorAux[findDate - indices[0]];
                        bool dateidentified = Util::formatDate(checkDateFormat, dateReadFormat, ss);

                        if(checkDateFormat == 0)
                        {
                            if(dateidentified)
                            {
                                vectorAux[findDate - indices[0]] = ss;
                                vecDateidentified.push_back(ss);

                                if (strtol(vectorAux[findDate - indices[0]].c_str(), NULL, 10) >= yrdoy)
                                {

                                    for (int i = 0; i < vectorAux.size(); i++)
                                    {
                                        if(vectorAux[i].compare(".") == 0)
                                            vectorAux[i] = "-99";

                                        flexibleio->setCharYrdoyMemory("WTH", vectorAux[findDate - indices[0]], weatherInp[i + indices[0]].varname, vectorAux[i]);

                                        //std::cout << vectorAux[findDate - indices[0]] + "_" + weatherInp[i + indices[0]].varname << " : "
                                        //          << flexibleio->getCharYrdoy("WTH", vectorAux[findDate - indices[0]], weatherInp[i + indices[0]].varname) << std::endl;
                                    }

                                    if(lastDay < strtol(vectorAux[findDate - indices[0]].c_str(), NULL, 10))
                                        lastDay = strtol(vectorAux[findDate - indices[0]].c_str(), NULL, 10);

                                    //DAYSREAD++;
                                }

                            }
                            else
                            {
                                sizeVec = vectorAux.size();
                                for(int i=0; i < vectorAux.size(); i++)
                                    vecDateUnidentified.push_back(vectorAux[i]);
                            }

                        }
                        else
                        {
                            *ERRNUM = checkDateFormat;
                            return 0;
                        }

                    }

                }
            }

            line = "";
        }

        myfile.close();
    }
    else
    {
        *ERRNUM = 30;
        return 0;
    }

    return 1;
}
