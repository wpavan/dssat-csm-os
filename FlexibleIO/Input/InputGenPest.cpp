/*=======================================================================
  InputPest.cpp, Felipe de Vargas, Willingthon Pavan, Fabio Oliveira
  Function to read Pest header and data.
-----------------------------------------------------------------------
  REVISION HISTORY
  01/07/2018 FO Added Read Pest header.
========================================================================*/
#include <string>
#include <regex>
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
  return Util::base52Encode(std::hash<std::string>{}(diseaseName));
}


std::string replacePlaceholders(std::string originalValue, std::string originalType, YAML::Node disease){
  // First, look for the special charcter '$' which indicates a variable reference.
  // If the variable reference is not found, then return the original value.
  // If the variable reference is found, then look for the variable in the rest of the file.
  std::regex varPattern(R"(\$\w+)");
  std::smatch matchResults;

  bool replaced = true;

  while (replaced){
    replaced = false;
    std::string tempStr = originalValue;

    while (std::regex_search(tempStr, matchResults, varPattern)){
      std::string placeholder = matchResults.str();
      std::string key = placeholder.substr(1); // Remove the '$'
      // Then, look for the variable name in the disease YAML::Node.
      if (disease[key]){
        // Here, we check to see if the variable is a string (used for equations where parentheses are needed).
        if (originalType == "std::string" || originalType == "string"){
          std::string value = '(' + disease[key]["VALUE"].as<std::string>() + ')';
          // Finally, replace the variable reference with the actual value.
          originalValue = std::regex_replace(originalValue, std::regex("\\" + placeholder), value);
          replaced = true;
        // If it is not an equation, then numeric substitution should be without parentheses.
        } else {
          std::string value = disease[key]["VALUE"].as<std::string>();
          // Finally, replace the variable reference with the actual value.
          originalValue = std::regex_replace(originalValue, std::regex("\\" + placeholder), value);
          replaced = true;
        }
        
      }
      // Being sure to keep checking the rest of the string for placeholders.
      tempStr = matchResults.suffix();
    }
  }

  return originalValue;
}


void addPestParam(std::string paramName, YAML::Node valueNode, std::string groupName) {
  FlexibleIO* flexIO = FlexibleIO::getInstance();
  int sequenceIndex = 1;
  switch (valueNode["VALUE"].Type()) {
    case 1: // YAML::NodeType::Null:
      flexIO->setCharMemory(groupName, paramName, "-99");
      break;

    case 2: // YAML::NodeType::Scalar:
      if(paramName == "RATE" || paramName == "INTEGRATION") {
        // NOTE: Necessary to find the proper pathing here. 
        std::string filename = paramName + ".cpp";
        //Util::writeInjectedCode(valueNode["VALUE"].as<std::string>(), filename);
      } 
      flexIO->setCharMemory(groupName, paramName, valueNode["VALUE"].as<std::string>());
      break;

    case 3: // YAML::NodeType::Sequence:
      for (const auto& element : valueNode["VALUE"]) {
        flexIO->setCharIndexMemory(groupName, paramName, element.as<std::string>(), sequenceIndex);
        sequenceIndex++;
      }
      break;
    
    // In general, we should not be encountering this.
    case 4: // YAML::NodeType::Map:
      break;

    // This should cause a warning message, and should be considered NA/-99.
    case 5: // YAML::NodeType::Undefined:
      flexIO->setCharMemory(groupName, paramName, "-99");
      break;
  }
}

// Before entering into flexibleio, handle all of the preprocessing 
int readPestYaml(char *filePST, char *PESTID, int *FOUND) {
  // NOTE: This is a manual setting of the input file and should be removed.
  filePST = "WHGEN048.yaml";

  FlexibleIO* flexIOInstance = FlexibleIO::getInstance();
  std::vector<YAML::Node> diseases;
  std::string groupName;
  // diseaseNum must start at 1 because FlexibleIO is 1-indexed. 
  // NOTE: Why is Fortran 0-indexed and C++ 1-indexed?????
  int diseaseNum = 1;

  int maxDiseases = 5;
  std::vector<std::string> diseaseHashes;
  std::string storedHash;
  std::istringstream iss(flexIOInstance->getCharArray("PEST", "DISEASES", std::to_string(maxDiseases)));
  while (iss >> storedHash) {
    diseaseHashes.push_back(storedHash);
  }

  // Clear out any existing diseases in memory.
  for (std::string hash : diseaseHashes) {
    if (hash != "-99") {
      flexIOInstance->eraseGroupMemory(hash);
    } else {
      break;
    }
  }

  // Then clear out the overall disease list.
  flexIOInstance->eraseGroupMemory("PEST");
  flexIOInstance->setIntegerMemory("PEST", "MAXDISEASES", maxDiseases);

  // Try to read the input YAML file and throw an error if it doesn't work.
  // NOTE: How should we address errors in GDM/FlexibleIO?
  try {
    diseases = YAML::LoadAllFromFile(filePST);
  }
  catch (const std::exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
    *FOUND = -1;
    return 1; // NOTE: Check and make sure this is the right return.
  }

  // Iterate through the 'documents' in the YAML file which each enumerate 
  // one disease.
  // NOTE: Check to see if this '&' implies immutability.
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
      // NOTE: Is checking disease.IsMap() necessary?
      // Step 1 to loading the disease is to process the input for variable references.
      for (auto it=disease.begin(); it!=disease.end(); ++it) {
        std::string key = it->first.as<std::string>();
        // NOTE: name "value" here is a bit hard to understand because it refers to the entire submapping (value, desc, etc.)
        YAML::Node value = it->second;

        // Check if the node is a map (i.e. a parameter node)
        if (value.Type() == 4) {
          // Check if the value is a string or numeric value.
          if (value["VALUE"].Type() == 2) {
            std::string originalValue = value["VALUE"].as<std::string>();
            std::string originalType = value["TYPE"].as<std::string>();
            value["VALUE"] = replacePlaceholders(originalValue, originalType, disease);
          // Check if the value is a sequence, so each one can be checked for placeholders.
          // This is necessary because automatic sequence -> string conversion is not supported.
          } else if (value["VALUE"].Type() == 3) {
            for (int i=0; i<value["VALUE"].size(); i++) {
              std::string originalValue = value["VALUE"][i].as<std::string>();
              std::string originalType = value["TYPE"].as<std::string>();
              value["VALUE"][i] = replacePlaceholders(originalValue, originalType, disease);
            }
          }
        }
      }

      // Step 2 is to load the disease into flexibleio.
      // std::cout << "Loading disease: " << disease["DISEASE"].as<std::string>() << " With size: " << disease.size() << std::endl;

      /* Create the disease groupName for FlexibleIO. Hashing ensures uniqueness.
       * All of the diseases have their own group in memory. The "PST" group holds
       * the list of hashes that exist. It could also store the text names of the 
       * diseases, but unsure if that's necessary.
       */
      groupName = hashDisease(disease["DISEASE"].as<std::string>());
      // NOTE: The 'E' in 'PEST' is just for testing.
      flexIOInstance->setCharIndexMemory("PEST", "DISEASES", groupName, diseaseNum);

        for (auto it=disease.begin(); it!=disease.end(); ++it) {
          std::string key = it->first.as<std::string>();
          // The name "value" here is a bit hard to understand because it refers to the entire submapping (value, desc, etc.)
          YAML::Node value = it->second;

          switch (value.Type()) {
            // Maybe throw an exception or determine a fixed behavior for NA values.
            // NOTE: also alias -99 as NA.
            case 1: // YAML::NodeType::Null:
              break;

            // These will include all of the metadata for the disease.
            // NOTE: -99 conversions to NA should be done here and checked top-level.
            case 2: // YAML::NodeType::Scalar:
              // Add important metadata to FlexibleIO.
              if (key == "PESTID" || key == "DISEASE") {
                flexIOInstance->setCharMemory(groupName, key, value.as<std::string>());
              }
              break;

            // This is seen mostly inside parameter nodes. May not need to check here.
            case 3: // YAML::NodeType::Sequence:
              break;
            
            // Every functional disease parameter must fit into this category.
            case 4: // YAML::NodeType::Map:
              addPestParam(key, value, groupName);
              break;          

            // Unknown where this would come up, need to check documentation. 
            // May only be if user breaks yaml file
            case 5: // YAML::NodeType::Undefined:
              break;

            // BP would be to include for edge cases (I think?)
            default:
              std::cout << "Unexpected type encountered for: " << key << std::endl;
              break;
          }
        }
      }
    }
    // Move on to the next disease in the YAML file, if present.
    diseaseNum++;
  }
  // No more diseases found in the YAML file.
  return 1;
} 

int readPest(char *filePST, char *PESTID, int *FOUND) {
  readPestYaml(filePST, PESTID, FOUND);

    std::string file(filePST), PestID(PESTID), type;
    file = Util::trim(file);
    int readheader;
    PestID = PestID.substr(0,5);
    FlexibleIO *flexibleio = FlexibleIO::getInstance();

    pestInp.clear();
    // flexibleio->eraseGroupMemory("PST");

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
