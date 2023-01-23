/*=======================================================================
  InputPest.cpp, Felipe de Vargas, Willingthon Pavan, Fabio Oliveira
  Function to read Pest header and data.
-----------------------------------------------------------------------
  REVISION HISTORY
  01/07/2018 FO Added Read Pest header.
========================================================================*/
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <iostream>
#include "../Util/Util.hpp"
#include "../Data/FlexibleIO.hpp"

extern "C" {
    int readPest(char *filePST, char *PESTID, int *FOUND);
}

struct HeaderData
{
    std::string varname;
    std::string section;
    std::string type;
    int size;
    int length;
};

std::vector<HeaderData> pestInp;

int readPestH(std::string file){
    std::string line, varname, sec;
    int test, type = 0;

    std::ifstream myfile(file);
    if(!myfile){return -99;}
    
    if (myfile.is_open())
    {
      //std::cout << "PEST FILE OPEN\n";
        while (getline(myfile, line) )
        {

            test = Util::ignore_line2(line);
            if(test == 3)
            {
                type++;
                line.erase(0, 1);
                std::stringstream ss(line);
                if (type == 1) {
                    sec = "HEADER";
                } else {
                    sec = "DATA";
                }

                while(ss >> varname) {
                    if(varname[0] != '!'){
                          HeaderData pest;
                          std::transform(varname.begin(), varname.end(), varname.begin(), ::toupper);
                          
                          if(varname.find(".") != std::string::npos)
                          {
                            pest.length = varname.size();
                            pest.section = sec;
                            
                            varname.erase(varname.find_first_of("."), varname.size());
                            pest.varname = varname;
                            pest.type = "STRING";
                          }
                          else
                          {
                            pest.section = sec;
                            pest.type = "INTEGER";
                            pest.varname = varname;
                          }
                          pestInp.push_back(pest);                      
                    }
                }
            }

        }
    }
    return 1;

}


int readPest(char *filePST, char *PESTID, int *FOUND) {

    std::string file(filePST), PestID(PESTID), type;
    file = Util::trim(file);
    int readheader;
    PestID = PestID.substr(0,5);
    FlexibleIO *flexibleio = FlexibleIO::getInstance();

    pestInp.clear();
    flexibleio->eraseGroupMemory("PST");

    if (pestInp.empty() && file.find("PST") != std::string::npos)
    {
        file.erase(file.find("PST") + 3, file.size());
        std::string aux;
        std::ifstream myfile(file);
      	if(!myfile){*FOUND = -1; return 0;}
        std::string line;
        
        if (myfile.is_open())
        {
            int test;
            
            readheader = readPestH(file);
            if(readheader == -99){*FOUND = -1; return 0;}
            
            while (getline (myfile, line) )
            {
                if (Util::ignore_line(line) == 1) {
                    if (line.find(PestID) != std::string::npos) {
                        std::string aux = line;

                          for (int i = 0; i < pestInp.size(); i++)
                          {
                              
                              type = pestInp[i].type;
                              std::transform(type.begin(), type.end(), type.begin(), toupper);
                              if (type.compare("STRING") == 0) {
                                  if(aux.size() <= pestInp[i].length){
                                    flexibleio->setCharMemory("PST", pestInp[i].varname, aux);
                                    aux = "";
                                  }
                                  else{
                                    flexibleio->setCharMemory("PST", pestInp[i].varname, aux.substr(0, pestInp[i].length));
                                    aux = aux.substr(pestInp[i].length + 1, aux.size());
                                  }
                                  //std::cout << pestInp[i].varname << " :" << flexibleio->getChar("PST", pestInp[i].varname) << std::endl;
                              
                              } else {
                                
                                  aux = Util::trim(aux);
                                  std::string str = aux.substr(0, aux.find_first_of(" "));
                                  
                                  if(!str.empty()){
                                    if(str.find(",") != std::string::npos){
                                      
                                      std::replace(str.begin(), str.end(), ',', ' ');
                                      std::stringstream ss(str);
                                      int index = 1;
                                      while(ss >> str){
                                        flexibleio->setCharIndexMemory("PST", pestInp[i].varname, str, index);
                                        //std::cout << pestInp[i].varname << " :" << flexibleio->getCharIndex("PST", pestInp[i].varname, index) << " ";
                                      index++;
                                      }
                                    }
                                    else{
                                      flexibleio->setCharMemory("PST", pestInp[i].varname, str);
                                    }
                                  }
                                  else{
                                    flexibleio->setCharMemory("PST", pestInp[i].varname, "-99");
                                  }
                                  //std::cout << pestInp[i].varname << " :" << flexibleio->getChar("PST", pestInp[i].varname) << std::endl;
                                  aux = Util::trim(aux);
                                  aux.erase(0, aux.find_first_of(" "));
                                  aux = Util::trim(aux);
                              }
                          }
                    }
                }
            }

        } else
        {
            *FOUND = -1;
            return 1;
        }
    }
    else
        *FOUND = -1;

    return 1;

}
