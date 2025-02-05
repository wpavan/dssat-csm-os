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
#include <functional>

#include <yaml-cpp/yaml.h>

#include "../Util/Util.hpp"
#include "../Data/FlexibleIO.hpp"


// NOTE: This is where we will read the new yaml files and store the data in memory.

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


std::string hashDisease(std::string diseaseName) {
  std::size_t hashValue = std::hash<std::string>{}(diseaseName);
  std::cout << "Disease (" << diseaseName << ") has hash value: " << hashValue << std::endl;
  std::string groupName = Util::base52Encode(hashValue);
  std::cout << "Group name evaluates to: " << groupName << std::endl;
  return groupName;
}

void addPestParam(std::string paramName, YAML::Node valueNode, std::string groupName) {
  FlexibleIO* flexIO = FlexibleIO::getInstance();
  switch (valueNode["VALUE"].Type()) {
    case YAML::NodeType::Null:
      std::cout << "Storing " << paramName << " value of " << "-99" << " in " << groupName << std::endl;
      flexIO->setCharMemory(groupName, paramName, "-99");
      break;

    case YAML::NodeType::Scalar:
      std::cout << "Storing " << paramName << " value of " << valueNode["VALUE"].as<std::string>() << " in " << groupName << std::endl;
      flexIO->setCharMemory(groupName, paramName, valueNode["VALUE"].as<std::string>());
      break;

    case YAML::NodeType::Sequence:
      break;

    case YAML::NodeType::Map:
      break;

    case YAML::NodeType::Undefined:
      break;

    default:
      break;
  }
  
}


int readPestYaml(std::string filePST) {
  /* Write pseudocode here for reading from yaml file. 
   * This is the main function that is coupled
   *
   * 1. setup necessary variables, get flexibleIO instance
   * 2. determine number of diseases in file by document tags
   * 3. for each disease, create a hash for 
   */
  FlexibleIO* flexIOInstance = FlexibleIO::getInstance();
  std::vector<YAML::Node> diseases;
  std::string groupName;
  int diseaseNum = 0;

  flexIOInstance->eraseGroupMemory("PST");

  // Try to read the input YAML file and throw an error if it doesn't work.
  // NOTE: How should we address errors in GDM/FlexibleIO?
  try {
    diseases = YAML::LoadAllFromFile(filePST);
  }
  catch (const std::exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
    return 0; // NOTE: Check and make sure this is the right return.
  }

  // Iterate through the 'documents' in the YAML file which each enumerate 
  // one disease.
  for (const auto& disease : diseases) {
    // Disease not formatted correctly.
    if (!disease["IS_ACTIVE"]) {
      std::cout << "No IS_ACTIVE tag found" << std::endl;
      continue;
    }

    // If the disease is not set to active, then ignore it.
    if (!disease["IS_ACTIVE"].as<bool>()) {
      continue;
    } else {
      if (disease.IsMap()) {
      // Disease YAML::Node is active and of proper type, so load it.
      std::cout << "Loading disease: " << disease["DISEASE"].as<std::string>() << " With size: " << disease.size() << std::endl;
      // Create the disease groupName for FlexibleIO. Hashing ensures uniqueness.
      groupName = hashDisease(disease["DISEASE"].as<std::string>());
      flexIOInstance->setCharIndexMemory("PST", "DISEASES", groupName, diseaseNum);

        for (auto it=disease.begin(); it!=disease.end(); ++it) {
          std::string key = it->first.as<std::string>();
          YAML::Node value = it->second;

          switch (value.Type()) {
            // Maybe throw an exception or determine a fixed behavior for NA values.
            // NOTE: also alias -99 as NA.
            case YAML::NodeType::Null:
              break;

            // These will include all of the metadata for the disease.
            // NOTE: -99 conversions to NA should be done here and checked top-level.
            case YAML::NodeType::Scalar:
              break;

            // This is seen mostly inside parameter nodes. May not need to check here.
            case YAML::NodeType::Sequence:
              break;
            
            // Every functional disease parameter must fit into this category.
            // We should write a function above which adds a single FlexibleIO 
            // variable at a time. To lessen the time impact, pass one shared instance 
            // of FlexibleIO. (I think this is already handled by FlexibleIO )
            case YAML::NodeType::Map:
              addPestParam(key, value, groupName);
              break;          

            // Unknown where this would come up, need to check documentation. 
            // May only be if user breaks yaml file
            case YAML::NodeType::Undefined:
              break;

            // BP would be to include for edge cases (I think?)
            default:
              std::cout << "Unexpected type encountered for: " << key << std::endl;
              break;
          }
        }
      }
    }
    // Move on to the next disease in the YAML file, if relevant.
    diseaseNum++;
  }
  // No more diseases found in the YAML file.
  return 1;
} 

int readPest(char *filePST, char *PESTID, int *FOUND) {
  std::cout << filePST << std::endl;
  readPestYaml("WHGEN048.yaml");

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