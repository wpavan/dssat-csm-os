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
#include <algorithm>
#include <memory>

#include "utilities.h"
#include "manager.h"
#include "simulator.h"
#include "injection.h"
#include "expression.h"

#include "../../FlexibleIO/Data/FlexibleIO.hpp"

extern "C" int readPestYaml(char *filePST, int *TRTNUM, int *FOUND);

// Define a struct for easily comparing parameters (clouds purpose)
struct CloudFParamHolder {
  std::unordered_map<std::string, Expression> params;

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
YearDoy Manager::plantingDate = YearDoy(-99);
YearDoy Manager::currentGDMDate = YearDoy(-99);
std::vector<std::string> Manager::families;
std::vector<CouplingPointID> Manager::couplingPointIDs;
std::vector<std::unique_ptr<CropInterface>> Manager::cropInterfaces;
std::vector<std::shared_ptr<CloudF>> Manager::cloudsF;
std::string Manager::outfileName;

bool Manager::outputStatus = false;

Manager::Manager() {}

Manager* Manager::getInstance() {
  if (instance == nullptr) {
      instance = new Manager();
  }
  return instance;
}

Manager* Manager::newInstance() {
  // Reset the singleton instance and clear static collections so that
  // repeated initialization (e.g., multiple treatment runs) does not
  // accumulate simulators, clouds, or crop interfaces.
  instance = nullptr;
  simulators.clear();
  plantingDate = YearDoy(-99);
  families.clear();
  couplingPointIDs.clear();
  cropInterfaces.clear();
  cloudsF.clear();
  outputStatus = false;
  return getInstance();
}

Simulator* Manager::getSimulator(int index) {
  if (index < 0 || index >= simulators.size()) {
      return nullptr; // or throw an exception
  }
  return simulators[index].get();
}

float safe_assign_float(std::string valueStr) {
  if (valueStr.empty()) {
    std::cerr << "Warning: Missing parameter value. Defaulting to 0." << std::endl;
    return 0.0f;
  }
  try {
    return std::stof(valueStr);
  } catch (const std::invalid_argument& e) {
    std::cerr << "Warning: Invalid parameter value '" << valueStr << "'. Defaulting to 0." << std::endl;
    return 0.0f;
  } catch (const std::out_of_range& e) {
    std::cerr << "Warning: Parameter value '" << valueStr << "' out of range. Defaulting to 0." << std::endl;
    return 0.0f;
  }
}

int safe_assign_int(std::string valueStr) {
  if (valueStr.empty()) {
    std::cerr << "Warning: Missing parameter value. Defaulting to 0." << std::endl;
    return 0;
  }
  try {
    return std::stoi(valueStr);
  } catch (const std::invalid_argument& e) {
    std::cerr << "Warning: Invalid parameter value '" << valueStr << "'. Defaulting to 0." << std::endl;
    return 0;
  } catch (const std::out_of_range& e) {
    std::cerr << "Warning: Parameter value '" << valueStr << "' out of range. Defaulting to 0." << std::endl;
    return 0;
  }
}

void Manager::addSimulator(std::unordered_map<std::string, Expression> diseaseData, CropInterface *ci,
                           InjectionHolder rateInjections, InjectionHolder integrationInjections, InjectionHolder outputInjections) {
  if (ci == nullptr) {
    std::cerr << "Error: CropInterface pointer is null in Manager::addSimulator" << std::endl;
    return;
  }
  
  // This function should create a simulator for a disease and add it to the list of simulators.
  // it should also create the associated disease object and associate that with the simulator =.
  Disease *disease = new Disease();
  float arraysize3[3], arraysize4[4];

  disease->setDiseaseID(diseaseData["PESTID"].getOriginal());

  // Removed by V.L. Covert 04/02/2026:
  // - DSPL
  // - SCF
  // - SPE
  // - WT
  // - MRRS
  // - HF
  // - DRE
  // - TFS
  // - RHFactor
  // - SWF
  
  // Removed " 05/08/2026:
  // - MSCD

  // Modified " ":
  // - IPS -> INIT_LES
  
  disease->setProportionFromOrganToPlantCloud(safe_assign_float(diseaseData["INOC_ORG_PLANT"].getOriginal()));
  disease->setProportionFromPlantToFieldCloud(safe_assign_float(diseaseData["INOC_PLANT_FIELD"].getOriginal()));
  
  if (diseaseData["CCFPO"].getOriginal().empty() || diseaseData["CCFPO"].evaluate() == -99.0f) {
    // Do not track inoculum age in each cloud as a removal method.
    disease->setINOC_AGE(false);
    disease->setVectorSizeCloudF(1);
    disease->setVectorSizeCloudP(1);
    disease->setVectorSizeCloudO(1);
  } else {
    // Track inoculum age in each cloud as a removal method.
    disease->setINOC_AGE(true);
    disease->setVectorSizeCloudF(safe_assign_int(diseaseData["CCFPO:1"].getOriginal()));
    disease->setVectorSizeCloudP(safe_assign_int(diseaseData["CCFPO:2"].getOriginal()));
    disease->setVectorSizeCloudO(safe_assign_int(diseaseData["CCFPO:3"].getOriginal()));
  }

  // BEGIN GDM2 Parameters - 9/22/2025
  // Added new parameters for input and output coupling points
  // - V. L. Covert 9/22/2025
  disease->setOrganCP(strToCPID(diseaseData["ORGAN_VALUE_CP"].getOriginal()));
  disease->setDamageCP(strToCPID(diseaseData["ORGAN_DAMAGE_CP"].getOriginal()));

  // Added new parameter for sharing spore clouds between diseases
  // - V. L. Covert 9/22/2025
  disease->setFamily(diseaseData["FAMILY"].getOriginal());
  // if the family is new, then create a new CloudF object and associate it with the disease.
  // if the family is not new, then find the existing CloudF object and associate it with the disease.
  
  // Create injection support
  for (auto& injection : rateInjections.injections) {
    Expression* expr = new Expression(std::get<1>(injection));
    disease->addRateInjection(Injection(std::get<0>(injection), expr, std::get<2>(injection)));
  }

  for (auto& injection : integrationInjections.injections) {
    Expression* expr = new Expression(std::get<1>(injection));
    disease->addIntegrationInjection(Injection(std::get<0>(injection), expr, std::get<2>(injection)));
  }

  for (auto& injection : outputInjections.injections) {
    Expression* expr = new Expression(std::get<1>(injection));
    disease->addOutputInjection(Injection(std::get<0>(injection), expr, std::get<2>(injection)));
  }

  // Added parameter to determine organ mode (cohort vs singular)
  if (diseaseData["ORGAN_MODE"].getOriginal() == "COHORT") {
      disease->setOrganMode(OrganMode::COHORT);
  } else if (diseaseData["ORGAN_MODE"].getOriginal() == "SINGULAR") {
      disease->setOrganMode(OrganMode::SINGULAR);
  } else {
      std::cerr << "Warning: Invalid ORGAN_MODE value '" << diseaseData["ORGAN_MODE"].getOriginal()
                << "'; defaulting to COHORT." << std::endl;
      disease->setOrganMode(OrganMode::COHORT);
  }

  // Implement new inoculum parameters
  // - INOC_EXT for external inoculum production expression
  // - INOC_LES for lesion-based inoculum production expression
  // - INOC_REM for daily proportional removal of inoculum
  disease->setINOC_EXT(diseaseData["INOC_EXT"]);
  disease->setINOC_LES(diseaseData["INOC_LES"]);
  disease->setINOC_REM(diseaseData["INOC_REM"]);
  disease->setINOC_DEST(diseaseData["INOC_DEST"]);


  // Implement organ damage parameter
  // - DAMAGE is the expression evaluated and then sent back to the selected ORGAN_DAMAGE_CP
  disease->setDAMAGE(diseaseData["DAMAGE"]);

  // Implement initial inoculum parameters
  // - II_AGE is the expression that determines how quickly favorability accumulates for initial spore release
  disease->setII_AGE(diseaseData["II_AGE"]);

  // Implement lesion parameters
  // - LES_AGE is the expression that determines how quickly lesions age
  // - VGF is the expression that determines the visible growth of lesions
  // - IGF is the expression that determines the invisible growth of lesions
  disease->setLES_AGE(diseaseData["LES_AGE"]);
  disease->setIGF(diseaseData["IGF"]);
  disease->setVGF(diseaseData["VGF"]);

  // Implement debugging expression to test expression functionality without impacting evaluation.
  disease->setDEBUG(diseaseData["DEBUG"]);

  // Use the *disease to find a new slot in the simulators 
  // vector and then initialize a new Simulator inside it.
  std::cout << "NEW SIMULATOR\n";
  simulators.emplace_back(std::make_unique<Simulator>(disease, ci));
}

void Manager::createCloudsF() {
  Disease* diseasePtr;

  // New version
  for (const auto& fam : families) {
    addCloudF(fam);
  }

  for (const auto& sim : simulators) {
    std::string simFamily = sim->getDisease()->getFamily();
    std::cout << "Family for this simulator: " << simFamily << std::endl;

    if (std::find(families.begin(), families.end(), simFamily) != families.end()) {
      diseasePtr = sim->getDisease();
        if (diseasePtr == nullptr) {
          std::cout << "Error: Disease pointer is null in Manager::createCloudsF for family " << simFamily << std::endl;
          continue;
        }
        if (getCloudF(simFamily) == nullptr) {
          std::cout << "Error: CloudF pointer is null in Manager::createCloudsF for family " << simFamily << std::endl;
        }
        getCloudF(simFamily)->setDisease(diseasePtr);
        sim->getInitialCondition()->setCloud(getCloudF(simFamily));
    }
  }
}

void Manager::setCurrentSimDate(YearDoy yearDoy) {
  currentGDMDate = yearDoy;
}

YearDoy Manager::getCurrentSimDate() {
  return currentGDMDate;
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
  
  std::regex varPattern = GDM::RegexPatterns::VAR_PATTERN;
  std::smatch matchResults;

  bool replaced = true;

  while (replaced){
    replaced = false;
    std::string tempStr = originalValue;

    while (std::regex_search(tempStr, matchResults, varPattern)){
      std::string placeholder = matchResults.str();
      std::string key = placeholder.substr(1); // Remove the '$'
      // Then, look for the variable name in the disease YAML::Node.
      if (disease[key] && disease[key]["VALUE"]) {
        std::string valueStr;
        // If scalar, use the scalar string directly
        if (disease[key]["VALUE"].IsScalar()) {
          try {
            valueStr = disease[key]["VALUE"].as<std::string>();
          } catch (const std::exception &e) {
            valueStr = "";
          }
        // If sequence, join elements with commas
        } else if (disease[key]["VALUE"].IsSequence()) {
          std::ostringstream joined;
          for (std::size_t si = 0; si < disease[key]["VALUE"].size(); ++si) {
            if (si) joined << ",";
            if (disease[key]["VALUE"][si].IsScalar()) {
              try { joined << disease[key]["VALUE"][si].as<std::string>(); } catch (...) { /* ignore */ }
            } else {
              // Fallback to dumping non-scalars
              std::string dumped = YAML::Dump(disease[key]["VALUE"][si]);
              dumped.erase(std::remove(dumped.begin(), dumped.end(), '\n'), dumped.end());
              joined << dumped;
            }
          }
          valueStr = joined.str();
        } else {
          // For maps or other types, dump to string (remove newlines)
          std::string dumped = YAML::Dump(disease[key]["VALUE"]);
          dumped.erase(std::remove(dumped.begin(), dumped.end(), '\n'), dumped.end());
          valueStr = dumped;
        }

        if (originalType == "std::string" || originalType == "string"){
          valueStr = '(' + valueStr + ')';
        }

        // Finally, replace the variable reference with the actual value.
        originalValue = std::regex_replace(originalValue, std::regex("\\" + placeholder), valueStr);
        replaced = true;
      }
      // Being sure to keep checking the rest of the string for placeholders.
      tempStr = matchResults.suffix();
    }
  }

  return originalValue;
}


void addPestParam(std::string paramName, YAML::Node valueNode, std::unordered_map<std::string, Expression> &diseaseData, std::string trtKey) {
  int sequenceIndex = 1;
  std::string dataLabel;

  // Handle paramNames {RATE, INTEGRATION} differently to allow for 
  // code injection with a streamlined format.
  switch (valueNode["VALUE"].Type()) {
    case 1: // YAML::NodeType::Null:
      diseaseData[paramName] = Expression("-99");
      break;

    case 2: // YAML::NodeType::Scalar:
      // Put parsed value in diseaseData map
      diseaseData[paramName] = Expression(valueNode["VALUE"].as<std::string>());
      break;

    case 3: // YAML::NodeType::Sequence:
      // This means that the three values of a sequence will be added like parameterX:1 = "val", parameterX:2 = "val", etc.
      for (const auto& element : valueNode["VALUE"]) {
        std::ostringstream temp;
        temp << paramName << ":" << sequenceIndex;
        dataLabel = temp.str();
        diseaseData[dataLabel] = Expression(element.as<std::string>());
        sequenceIndex++;
      }
      break;
    
    // This represents a map of TRTNUM to value
    // We want to add the value that is associated with the key "TRNO" + TRTNUM, 
    // and if that is not found, then we want to add the value associated with the key "DEFAULT".
    case 4: // YAML::NodeType::Map:
      if (valueNode["VALUE"][trtKey]) {
        if (valueNode["VALUE"][trtKey].IsScalar()) {
          diseaseData[paramName] = Expression(valueNode["VALUE"][trtKey].as<std::string>());
        } else if (valueNode["VALUE"][trtKey].IsSequence()) {
          for (const auto& subElement : valueNode["VALUE"][trtKey]) {
            std::ostringstream temp;
            temp << paramName << ":" << sequenceIndex;
            dataLabel = temp.str();
            diseaseData[dataLabel] = Expression(subElement.as<std::string>());
            sequenceIndex++;
          }
        }
      } else if (valueNode["VALUE"]["DEFAULT"]) {
        if (valueNode["VALUE"]["DEFAULT"].IsScalar()) {
          diseaseData[paramName] = Expression(valueNode["VALUE"]["DEFAULT"].as<std::string>());
        } else if (valueNode["VALUE"]["DEFAULT"].IsSequence()) {
          for (const auto& subElement : valueNode["VALUE"]["DEFAULT"]) {
            std::ostringstream temp;
            temp << paramName << ":" << sequenceIndex;
            dataLabel = temp.str();
            diseaseData[dataLabel] = Expression(subElement.as<std::string>());
            sequenceIndex++;
          }
        }
      } else {
        throw std::invalid_argument("ERROR: No matching treatment key '" + trtKey + "' or 'DEFAULT' found for parameter '" + paramName + "'.");
      }
      break;

    // This should cause a warning message, and should be considered NA/-99.
    case 5: // YAML::NodeType::Undefined:
      diseaseData[paramName] = Expression("-99");
      break;
  }
}

int readPestYaml(char *filePST, int *TRTNUM, int *FOUND) {
  Manager* manager = Manager::newInstance();
  std::string outfileName = std::string(filePST, 8) + '_' + std::to_string(*TRTNUM);

  manager->setOutfileName(outfileName);

  CouplingPointID tempCP;
  std::vector<CouplingPointID> uniqueCPs;
  UniqueFamilies uniqueFamilies;
  std::vector<YAML::Node> diseases;

  std::string trtKey = "TRNO" + std::to_string(*TRTNUM);

  // Try to read the input YAML file and throw an error if it doesn't work.
  // NOTE: How should we address errors in GDM/FlexibleIO?
  try {
    diseases = YAML::LoadAllFromFile(filePST);
  }
  catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
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
            std::string originalType = "string";
            if (value["TYPE"] && value["TYPE"].IsScalar()) {
              originalType = value["TYPE"].as<std::string>();
            }
            value["VALUE"] = replacePlaceholders(originalValue, originalType, disease);
          // Check if the value is a sequence, so each one can be checked for placeholders.
          // This is necessary because automatic sequence -> string conversion is not supported.
          } else if (value["VALUE"].Type() == 3) {
            for (int i=0; i<value["VALUE"].size(); i++) {
              std::string originalValue = "";
              if (value["VALUE"][i].IsScalar()) {
                try { originalValue = value["VALUE"][i].as<std::string>(); } catch(...) { originalValue = ""; }
              } else {
                std::string dumped = YAML::Dump(value["VALUE"][i]);
                dumped.erase(std::remove(dumped.begin(), dumped.end(), '\n'), dumped.end());
                originalValue = dumped;
              }
              std::string originalType = "string";
              if (value["TYPE"] && value["TYPE"].IsScalar()) {
                originalType = value["TYPE"].as<std::string>();
              }
              value["VALUE"][i] = replacePlaceholders(originalValue, originalType, disease);
            }
          } else if (value["VALUE"].Type() == 4) {
            // Iterate through each treatment's key-value pair.
            // Accepted keys are TRNO01-99 and DEFAULT. 
            for (auto trtValueIt = value["VALUE"].begin(); trtValueIt != value["VALUE"].end(); ++trtValueIt) {
              std::string trtKey = trtValueIt->first.as<std::string>();
              YAML::Node trtValue = trtValueIt->second;
              if (trtKey.rfind("TRNO", 0) == 0 || trtKey == "DEFAULT") {
                if (trtValue.IsScalar()) {
                  std::string originalValue = trtValue.as<std::string>();
                  std::string originalType = "string";
                  if (value["TYPE"] && value["TYPE"].IsScalar()) {
                    originalType = value["TYPE"].as<std::string>();
                  }
                  value["VALUE"][trtKey] = replacePlaceholders(originalValue, originalType, disease);
                } else {
                  std::cout << "Warning: Treatment value for " << trtKey << " is not a scalar and will be skipped for placeholder replacement." << std::endl;
                }
              } else {
                std::cout << "Warning: Unrecognized treatment key '" << trtKey << "'; expected keys starting with 'TRNO' or 'DEFAULT'." << std::endl;
              }
            }
          } // NOTE: Here we are effectively excluding the injection sections because of their alternate format.
        }
      }

      std::unordered_map<std::string, Expression> diseaseData;
      // NOTE: Check out these InjectionHolders for validity after Expression changes.
      InjectionHolder rateInjections, integrationInjections, outputInjections;
      CloudFParamHolder cloudParams;
      std::string injectionExpression;

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
                diseaseData[key] = Expression(value.as<std::string>());
              }
              break;

            // This is seen mostly inside parameter nodes. May not need to check here.
            case 3: // YAML::NodeType::Sequence:
              break;
            
            // Every functional disease parameter must fit into this category.
            case 4: // YAML::NodeType::Map:
              if (key == "RATE" || key == "INTEGRATION" || key == "OUTPUT") {
                // Iterate through each injection endpoint in the node
                for (auto injIt = value.begin(); injIt != value.end(); ++injIt) {
                  std::string endpointName = injIt->first.as<std::string>();
                  YAML::Node injectionData = injIt->second;
                  
                  // Validate that this injection has required fields
                  if (!injectionData["EXPRESSION"] || !injectionData["MODIFICATION"]) {
                    std::cout << "Warning: Rate injection " << endpointName 
                              << " missing EXPRESSION or MODIFICATION field" << std::endl;
                    continue;
                  }
                  
                  try {
                    // First, check for the treatment logic for expressions only.
                    if (injectionData["EXPRESSION"].Type() == 4) {
                      if (injectionData["EXPRESSION"][trtKey]) {
                        injectionExpression = injectionData["EXPRESSION"][trtKey].as<std::string>();
                      } else if (injectionData["EXPRESSION"]["DEFAULT"]) {
                        injectionExpression = injectionData["EXPRESSION"]["DEFAULT"].as<std::string>();
                      } else {
                        throw std::invalid_argument("ERROR: No matching treatment key '" + trtKey + "' or 'DEFAULT' found for injection expression in endpoint '" + endpointName + "'.");
                      }
                    } else {
                      injectionExpression = injectionData["EXPRESSION"].as<std::string>();
                    }
                    
                    std::string modification = injectionData["MODIFICATION"].as<std::string>();
                    
                    // Create the injection
                    switch (StepParser::parse(key)) {
                      case -1:  // INVALID
                        throw std::invalid_argument("The following string is not recognized as one of the three steps: " + key + "\nThe step options are:\n\tRATE\n\tINTEGRATION\n\tOUTPUT");
                      case 0:   // RATE
                        rateInjections.add(endpointName, injectionExpression, modification);
                        break;
                      case 1:   // INTEGRATION
                        integrationInjections.add(endpointName, injectionExpression, modification);
                        break;
                      case 2:   // OUTPUT
                        outputInjections.add(endpointName, injectionExpression, modification);
                        break;
                    }
                    
                  } catch (const YAML::Exception& e) {
                    std::cout << "Error parsing rate injection " << endpointName 
                              << ": " << e.what() << std::endl;
                  }
                }
              } else {
                addPestParam(key, value, diseaseData, trtKey);
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
        // Safely obtain ORGAN_VALUE_CP from diseaseData
        auto organIt = diseaseData.find("ORGAN_VALUE_CP");
        if (organIt == diseaseData.end() || organIt->second.empty()) {
          std::cerr << "Error: missing required ORGAN_VALUE_CP for disease PESTID='"
                    << (diseaseData.count("PESTID") ? diseaseData["PESTID"].getOriginal() : "<unknown>")
                    << "' -- skipping this disease." << std::endl;
          continue;
        }

        const std::string cpStr = organIt->second.getOriginal();
        tempCP = strToCPID(cpStr);

        // Only create a new CropInterface if we haven't seen this coupling point before
        if (tempCP == CouplingPointID::VALUE) {
          try {
            manager->addCropInterface(tempCP, diseaseData["ORGAN_AGE"], safe_assign_float(cpStr));
          } catch (const std::exception &e) {
            std::cerr << "Error parsing ORGAN_VALUE_CP='" << cpStr << "': " << e.what() << std::endl;
            continue;
          }
        } else if (std::find(uniqueCPs.begin(), uniqueCPs.end(), tempCP) == uniqueCPs.end()) {
          uniqueCPs.push_back(tempCP);
          manager->addCropInterface(tempCP, diseaseData["ORGAN_AGE"]);
        }

        ciPtr = manager->getCropInterface(tempCP);
        if (!ciPtr) {
          std::cout << "getCropInterface -> NULL" << std::endl;
        }

        // Step 3b is to handle family grouping.
        std::string family = diseaseData["FAMILY"].getOriginal();

        CloudF *cloudFPtr = nullptr;
        
        if (!uniqueFamilies.contains(family)) {
          std::cout << "Will create new CloudF for disease family: " << family << std::endl;
          uniqueFamilies.families[family] = cloudParams;

          manager->addUniqueFamily(family);
        } else {
          std::cout << "Will use existing CloudF for shared inoculum: " << cpIDToStr(tempCP) << std::endl;

          if (cloudParams != uniqueFamilies.families[family]) {
            throw std::runtime_error("Error: Cloud parameters for family " + family + " do not match previous definition.");
          }
        }
        manager->addSimulator(diseaseData, ciPtr, rateInjections, integrationInjections, outputInjections);
      }
    }
  }

  manager->setCouplingPointIDs(uniqueCPs);
  manager->createCloudsF();
  // No more diseases found in the YAML file.
  return 1;
} 

void Manager::updateCurrentYearDoy(YearDoy yearDoy) {
  // Check if the currentGDMDate is less than the new yearDoy
  while(currentGDMDate.addOneDay() <= yearDoy) {
    // Simulate the currentGDMDate
    rate();
    integration();

    // Update the date by one day
    currentGDMDate += 1;  
  }
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