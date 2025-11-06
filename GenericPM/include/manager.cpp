/**
 * @file manager.cpp
 * 
 * @author Virginia Covert (virginiacovert454@gmail.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include <sstream>
#include <regex>
#include <variant>

#include <yaml-cpp/yaml.h>

#include "manager.h"
#include "simulator.h"
#include "injection.h"

#include "../../FlexibleIO/Data/FlexibleIO.hpp"

extern "C" int readPestYaml(char *filePST, int *FOUND);

// Define a struct for easily comparing parameters (clouds purpose)
struct CloudFParamHolder {
  std::unordered_map<std::string, std::string> params;

  bool operator==(const CloudFParamHolder& other) const {
    for (const auto& param : params) {
      auto it = other.params.find(param.first);
      if (it == other.params.end() || it->second != param.second) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const CloudFParamHolder& other) const {
    return !(*this == other);
  }
};

// Define a struct that holds the families and their parameters
struct UniqueFamilies {
  std::unordered_map<std::string, CloudFParamHolder> families;

  bool contains(const std::string& fam) const {
    return families.find(fam) != families.end();
  }
};

// Define all static members
Manager* Manager::instance = nullptr;
std::vector<std::unique_ptr<Simulator>> Manager::simulators;
int Manager::plantingDate = -99;
std::vector<std::string> Manager::families;
std::vector<CouplingPointID> Manager::couplingPointIDs;
std::vector<std::unique_ptr<CropInterface>> Manager::cropInterfaces;
std::vector<std::unique_ptr<CloudF>> Manager::cloudsF;

Manager::Manager() {}

Manager* Manager::getInstance() {
  if (instance == nullptr) {
      instance = new Manager();
  }
  return instance;
}

Manager* Manager::newInstance() {
  instance = nullptr;
  return getInstance();
}

Simulator* Manager::getSimulator(int index) {
  if (index < 0 || index >= simulators.size()) {
      return nullptr; // or throw an exception
  }
  return simulators[index].get();
}

void Manager::addSimulator(std::unordered_map<std::string, std::string> diseaseData, CropInterface *ci, InjectionHolder rateInjections, InjectionHolder integrationInjections) {
  if (ci == nullptr) {
    std::cerr << "Error: CropInterface pointer is null in Manager::addSimulator" << std::endl;
    return;
  }
  
  // This function should create a simulator for a disease and add it to the list of simulators.
  // it should also create the associated disease object and associate that with the simulator =.
  Disease *disease = new Disease();
  float arraysize3[3], arraysize4[4];

  disease->setDescription(diseaseData["PESTID"]);
  
  disease->setDailySporeProductionPerLesion(std::stof(diseaseData["DSPL"]));

  arraysize4[0] = std::stof(diseaseData["SPE:1"]);
  arraysize4[1] = std::stof(diseaseData["SPE:2"]);
  arraysize4[2] = std::stof(diseaseData["SPE:3"]);
  arraysize4[3] = std::stof(diseaseData["SPE:4"]);
  disease->setCohortAgeSet(arraysize4);
  
  arraysize3[0] = std::stof(diseaseData["SCF:1"]);
  arraysize3[1] = std::stof(diseaseData["SCF:2"]);
  arraysize3[2] = std::stof(diseaseData["SCF:3"]);
  disease->setSporulationCrowdingFactorsSet(arraysize3);
  
  disease->setMaxSporeCloudsDensity(std::stof(diseaseData["MSCD"]));
  
  disease->setProportionFromOrganToPlantCloud(std::stof(diseaseData["SPO2P"]));
  disease->setProportionFromPlantToFieldCloud(std::stof(diseaseData["SPP2F"]));
  
  disease->setVectorSizeCloudF(std::stof(diseaseData["CCFPO:1"]));
  disease->setVectorSizeCloudP(std::stof(diseaseData["CCFPO:2"]));
  disease->setVectorSizeCloudO(std::stof(diseaseData["CCFPO:3"]));

  disease->setMRRS(std::stof(diseaseData["MRRS"]));
  
  disease->setInitialInoculum(std::stof(diseaseData["II"]));
  
  disease->setAcumulateFavorability(std::stof(diseaseData["AFII"]));    
  
  arraysize3[0] = std::stof(diseaseData["TFS:1"]);
  arraysize3[1] = std::stof(diseaseData["TFS:2"]);
  arraysize3[2] = std::stof(diseaseData["TFS:3"]);
  disease->setTemperatureFavorabilitySet(arraysize3);
  
  disease->setInfectionEfficiency(std::stof(diseaseData["IE"]));
  
  disease->setInitialPustuleSize(std::stof(diseaseData["IPS"]));
  
  disease->setLatentPeriod(std::stoi(diseaseData["LP"]));
  
  disease->setInfectionPeriod(std::stoi(diseaseData["IP"]));
  
  disease->setWetnessThreshold(std::stof(diseaseData["WT"]));
  
  disease->setHostFactor(std::stof(diseaseData["HF"]));
  
  disease->setInvisibleGrowthFunction(diseaseData["IGF"]);
  
  disease->setVisibleGrowthFunction(diseaseData["VGF"]);

  disease->setWetnessFunction(diseaseData["WF"]);

  disease->setDispersionFrequency(diseaseData["DRE"]);

  disease->setRhFactor(diseaseData["RHFac"]);

  // Added new parameter called sporeModule to the disease class
  // - V. L. Covert 4/1/2025
  disease->setSporeModule(diseaseData["SPOREMODULE"]);

  // Added SWF parameter which was previously unused.
  // - V. L. Covert 4/1/2025
  disease->setSWF(diseaseData["SWF"]);

  // Added new parameters for input and output coupling points
  // - V. L. Covert 9/22/2025
  disease->setOrganCP(strToCPID(diseaseData["ORGAN_AREA_CP"]));
  disease->setDamageCP(strToCPID(diseaseData["ORGAN_DAMAGE_CP"]));

  // Added new parameter for sharing spore clouds between diseases
  // - V. L. Covert 9/22/2025
  disease->setFamily(diseaseData["FAMILY"]);
  // if the family is new, then create a new CloudF object and associate it with the disease.
  // if the family is not new, then find the existing CloudF object and associate it with the disease.
  
  for (auto& injection : rateInjections.injections) {
    disease->addRateInjection(Injection(std::get<0>(injection), std::get<1>(injection), std::get<2>(injection)));
  }

  for (auto& injection : integrationInjections.injections) {
    disease->addIntegrationInjection(Injection(std::get<0>(injection), std::get<1>(injection), std::get<2>(injection)));
  }

  #ifdef DEBUGX  
  disease->printDisease();

  std::cout << "Crop Interface size in Manager::addSimulator: " << ci->getOrgansQtd() << std::endl;
  #endif

  // Use the *disease to find a new slot in the simulators 
  // vector and then initialize a new Simulator inside it.
  simulators.emplace_back(std::make_unique<Simulator>(disease, ci));
}

void Manager::createCloudsF() {
  Disease* diseasePtr;

  for (const auto& fam : families) {
    addCloudF(fam);
    CloudF *cloudFPtr = getCloudF(fam);
    if (cloudFPtr == nullptr) {
      std::cout << "Error: CloudF pointer is null in Manager::createCloudsF for family " << fam << std::endl;
      continue;
    }
    for (const auto& sim : simulators) {
      std::cout << "Family for this simulator: " << sim->getDisease()->getFamily() << std::endl;
      if (sim->getDisease()->getFamily() == fam) {
          diseasePtr = sim->getDisease();
          if (diseasePtr == nullptr) {
            std::cout << "Error: Disease pointer is null in Manager::createCloudsF for family " << fam << std::endl;
            continue;
          }
          cloudFPtr->setDisease(diseasePtr);
          sim->getInitialCondition()->setCloud(cloudFPtr);
      }
    }
  }
}

void Manager::setCurrentSimDate(int yearDoy) {
    for (auto& simulator : simulators) {
        simulator->setCurrentYearDoy(yearDoy);
    }
}

int Manager::getCurrentSimDate() {
    if (simulators.size() > 0) {
        return simulators[0]->getCurrentYearDoy();
    } else {
        return -99;
    }
}

void Manager::rate() {
  for (auto& simulator : simulators) {
    simulator->rate();
  }
}

void Manager::integration() {
    for (auto& simulator : simulators) {
        simulator->integration();
    }
}

void Manager::output() {
    for (auto& simulator : simulators) {
        simulator->output();
    }
}

std::string hashDisease(std::string diseaseName) {
  return Utilities::base52Encode(std::hash<std::string>{}(diseaseName));
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


void addPestParam(std::string paramName, YAML::Node valueNode, std::unordered_map<std::string, std::string> &diseaseData) {
  int sequenceIndex = 1;
  std::string dataLabel;
  // Handle paramNames {RATE, INTEGRATION} differently to allow for 
  // code injection with a streamlined format.
  switch (valueNode["VALUE"].Type()) {
    case 1: // YAML::NodeType::Null:
      diseaseData[paramName] = "-99";
      break;

    case 2: // YAML::NodeType::Scalar:
      if(paramName == "RATE" || paramName == "INTEGRATION") {
        // NOTE: Necessary to find the proper pathing here. 
        // std::string filename = paramName + ".cpp";
        //Util::writeInjectedCode(valueNode["VALUE"].as<std::string>(), filename);
      }
      diseaseData[paramName] = valueNode["VALUE"].as<std::string>();
      break;

    case 3: // YAML::NodeType::Sequence:
      // This means that the three values of a sequence will be added like parameterX:1 = "val", parameterX:2 = "val", etc.
      for (const auto& element : valueNode["VALUE"]) {
        std::ostringstream temp;
        temp << paramName << ":" << sequenceIndex;
        dataLabel = temp.str();
        diseaseData[dataLabel] = element.as<std::string>();
        sequenceIndex++;
      }
      break;
    
    // In general, we should not be encountering this.
    case 4: // YAML::NodeType::Map:
      break;

    // This should cause a warning message, and should be considered NA/-99.
    case 5: // YAML::NodeType::Undefined:
      diseaseData[paramName] = "-99";
      break;
  }
}

int readPestYaml(char *filePST, int *FOUND) {
  // NOTE: This is a manual setting of the input file and should be removed.
  filePST = "WHGEN048.yaml";

  Manager* manager = Manager::newInstance();

  CouplingPointID tempCP;
  std::vector<CouplingPointID> uniqueCPs;
  UniqueFamilies uniqueFamilies;
  std::vector<YAML::Node> diseases;

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
          } // NOTE: Here we are effectively excluding the injection sections because of their alternate format.
        }
      }

      std::unordered_map<std::string, std::string> diseaseData;
      InjectionHolder rateInjections, integrationInjections;
      CloudFParamHolder cloudParams;

      // Step 2 is to load the disease into memory.
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
              // Add important metadata to DiseaseData map.
              if (key == "PESTID" || key == "DISEASE" || key == "FAMILY") {
                diseaseData[key] = value.as<std::string>();
              }
              break;

            // This is seen mostly inside parameter nodes. May not need to check here.
            case 3: // YAML::NodeType::Sequence:
              break;
            
            // Every functional disease parameter must fit into this category.
            case 4: // YAML::NodeType::Map:
              if (key == "RATE") {
                // Iterate through each injection endpoint in RATE
                for (auto injIt = value.begin(); injIt != value.end(); ++injIt) {
                  std::string endpointName = injIt->first.as<std::string>();
                  YAML::Node injectionData = injIt->second;
                  std::cout << injectionData << std::endl;
                  
                  // Validate that this injection has required fields
                  if (!injectionData["EXPRESSION"] || !injectionData["MODIFICATION"]) {
                    std::cout << "Warning: Rate injection " << endpointName 
                              << " missing EXPRESSION or MODIFICATION field" << std::endl;
                    continue;
                  }
                  
                  try {
                    std::string expression = injectionData["EXPRESSION"].as<std::string>();
                    std::string modification = injectionData["MODIFICATION"].as<std::string>();
                    
                    // Create the injection
                    rateInjections.add(endpointName, expression, modification);
                    
                  } catch (const YAML::Exception& e) {
                    std::cout << "Error parsing rate injection " << endpointName 
                              << ": " << e.what() << std::endl;
                  }
                }
                
              } else if (key == "INTEGRATION") {
                // Similar logic for INTEGRATION injections
                for (auto injIt = value.begin(); injIt != value.end(); ++injIt) {
                  std::string endpointName = injIt->first.as<std::string>();
                  YAML::Node injectionData = injIt->second;
                  
                  // Validate that this injection has required fields
                  if (!injectionData["EXPRESSION"] || !injectionData["MODIFICATION"]) {
                    std::cout << "Warning: Integration injection " << endpointName 
                              << " missing EXPRESSION or MODIFICATION field" << std::endl;
                    continue;
                  }
                  
                  try {
                    std::string expression = injectionData["EXPRESSION"].as<std::string>();
                    std::string modification = injectionData["MODIFICATION"].as<std::string>();
                    
                    // Create the injection
                    integrationInjections.add(endpointName, expression, modification);
                    
                  } catch (const YAML::Exception& e) {
                    std::cout << "Error parsing integration injection " << endpointName 
                              << ": " << e.what() << std::endl;
                  }
                }
              } else {
                addPestParam(key, value, diseaseData);
                // Handle if the node is a CLOUD_PARAM:
                if (value["CLOUD_PARAM"] && value["CLOUD_PARAM"].as<bool>() && typeid(diseaseData[key]) == typeid(std::string)) {
                  cloudParams.params[key] = diseaseData[key];
                }
              }
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

        // Now the diseaseData is fully populated with the disease parameters.
        // Step 3 is to add the disease to the manager.
        // Step 3a is to init the CropInterface if needed.
        CropInterface *ciPtr = nullptr;
        #ifdef DEBUGX
        std::cout << "ORGAN_AREA_CP: " << diseaseData.at("ORGAN_AREA_CP") << std::endl << "Running strToCPID..." << std::endl;
        #endif
        tempCP = strToCPID(diseaseData.at("ORGAN_AREA_CP"));
         
        // Only create a new CropInterface if we haven't seen this coupling point before
        if (tempCP == CouplingPointID::VALUE) {
          #ifdef DEBUGX
          std::cout << "Creating new CropInterface for CP: " << cpIDToStr(tempCP) << std::endl;
          #endif // DEBUG
          manager->addCropInterface(tempCP, std::stof(diseaseData.at("ORGAN_AREA_CP")));
        } else if (std::find(uniqueCPs.begin(), uniqueCPs.end(), tempCP) == uniqueCPs.end()) {
          #ifdef DEBUGX 
          std::cout << "Creating new CropInterface for CP: " << cpIDToStr(tempCP) << std::endl;
          #endif // DEBUG
          uniqueCPs.push_back(tempCP);
          manager->addCropInterface(tempCP);
        } else {
          #ifdef DEBUGX          
          std::cout << "Using existing CropInterface for CP: " << cpIDToStr(tempCP) << std::endl;
          #endif // DEBUG
        }
        
        ciPtr = manager->getCropInterface(tempCP);
        std::cout << "getCropInterface -> " << (ciPtr ? "valid " : "NULL ") << cpIDToStr(ciPtr->getOrganCP()) << std::endl;

        // Step 3b is to handle family grouping.
        std::string family = diseaseData["FAMILY"];

        CloudF *cloudFPtr = nullptr;
        
        if (!uniqueFamilies.contains(family)) {
          std::cout << "Will create new CloudF for disease family: " << family << std::endl;
          uniqueFamilies.families[family] = cloudParams;
          
          #ifdef DEBUGX          
          std::cout << "Cloud parameters for this family: " << family << std::endl;
          for (const auto& param : cloudParams.params) {
            std::cout << "  " << param.first << ": " << param.second << std::endl;
          }
          #endif // DEBUG

          manager->addUniqueFamily(family);
        } else {
          std::cout << "Will use existing CloudF for shared inoculum: " << cpIDToStr(tempCP) << std::endl;

          #ifdef DEBUGX          
          std::cout << "Cloud parameters for this family: " << family << std::endl;
          for (const auto& param : cloudParams.params) {
            std::cout << "  " << param.first << ": " << param.second << std::endl;
          }
          #endif // DEBUG

          if (cloudParams != uniqueFamilies.families[family]) {
            throw std::runtime_error("Error: Cloud parameters for family " + family + " do not match previous definition.");
          }
        }
        manager->addSimulator(diseaseData, ciPtr, rateInjections, integrationInjections);
      }
    }
  }

  manager->setCouplingPointIDs(uniqueCPs);
  manager->createCloudsF();
  // No more diseases found in the YAML file.
  return 1;
} 

/*
Execution workflow:
0. Coupling functions are the only ones that are called by fortran.
   These functions have the capability to create and manage the 
   (static) manager instance.
1. Singleton manager instance is created by couplingInit (can we 
   determine if this is the first time?).
2. The manager loads all of the diseases into fio ONCE (during an init 
   ideally).
3. The manager creates a list of simulators and holds onto them in an
   order defined by the user (in the YAML file).
4. The manager can loop through the simulators and propagate the rate,
   integration, and output functions down to lower objects.
NOTES: No objects will be passed back to DSSAT to maintain modularity.
       One simulator contains one initial condition/disease.
*/