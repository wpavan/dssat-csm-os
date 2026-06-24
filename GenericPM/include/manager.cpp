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
#include <cctype>
#include <memory>

#include "utilities.h"
#include "manager.h"
#include "simulator.h"
#include "injection.h"
#include "expression.h"
#include "debug_control.h"

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
std::string Manager::outfileName;

bool Manager::outputStatus = false;

Manager::Manager() {}

Manager* Manager::getInstance() {
  if (instance == nullptr) {
#if GENERICPM_DEBUG_ENABLED
      std::cerr << "[MGR] Creating new Manager instance" << std::endl << std::flush;
#endif
      instance = new Manager();
  }
  return instance;
}

Manager* Manager::newInstance() {
  // Reset the singleton instance and clear static collections so that
  // repeated initialization (e.g., multiple treatment runs) does not
  // accumulate simulators, clouds, or crop interfaces.
  delete instance;
  instance = nullptr;
  simulators.clear();
  plantingDate = YearDoy(-99);
  families.clear();
  couplingPointIDs.clear();
  cropInterfaces.clear();
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
                           InjectionHolder rateInjections, InjectionHolder integrationInjections, InjectionHolder outputInjections,
                           CustomOutputHolder customOutputs) {
#if GENERICPM_DEBUG_ENABLED
  std::cerr << "[MGR] addSimulator() called for disease: " << diseaseData["PESTID"].getOriginal() << std::endl << std::flush;
#endif
  if (ci == nullptr) {
    std::cerr << "Error: CropInterface pointer is null in Manager::addSimulator" << std::endl;
    return;
  }
  
  std::unordered_map<std::string, std::string> diseaseContext;
  for (const auto& pair : diseaseData) {
    diseaseContext[pair.first] = pair.second.getOriginal();
  } 

  // This function should create a simulator for a disease and add it to the list of simulators.
  // it should also create the associated disease object and associate that with the simulator =.
  std::shared_ptr<Disease> disease = std::make_shared<Disease>();
  float arraysize3[3], arraysize4[4];

  disease->setDiseaseID(diseaseData["PESTID"].getOriginal());
  if (!customOutputs.fileName.empty()) {
    disease->setCustomOutputFileName(customOutputs.fileName);
  }
  disease->setCustomOutputFormat(customOutputs.format);

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

  for (auto& output : customOutputs.outputs) {
    Expression expr(output.expression);
    expr.setContext(diseaseContext);
    disease->addCustomOutput(output.name, expr, output.desc);
  }

  // Added parameter to determine organ mode (cohort vs singular)
  // NOTE: local inoculum seems to occasionally return some garbage huge value when running in COHORT mode. Unsure if happens in SINGULAR.
  if (diseaseData["ORGAN_MODE"].getOriginal() == "COHORT") {
      disease->setOrganMode(OrganMode::COHORT);
  } else if (diseaseData["ORGAN_MODE"].getOriginal() == "SINGULAR") {
      disease->setOrganMode(OrganMode::SINGULAR);
  } else {
      std::cerr << "Warning: Invalid ORGAN_MODE value '" << diseaseData["ORGAN_MODE"].getOriginal()
                << "'; defaulting to COHORT." << std::endl;
      disease->setOrganMode(OrganMode::COHORT);
  }

  // Use Existing parameters for initial inoculum and favorability 
  disease->setInitialInoculum(safe_assign_float(diseaseData["II"].getOriginal()));
  disease->setAcumulateFavorability(safe_assign_float(diseaseData["AFII"].getOriginal()));    

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
  // - INIT_LES is the expression that determines the initial lesion size in unit of ORGAN_CP
  disease->setLES_AGE(diseaseData["LES_AGE"]);
  disease->setIGF(diseaseData["IGF"]);
  disease->setVGF(diseaseData["VGF"]);
  disease->setInitialLesionSize(diseaseData["INIT_LES"]);
  disease->setNEW_LES(diseaseData["NEW_LES"]);
  disease->setP1_DUR(safe_assign_int(diseaseData["P1"].getOriginal()));
  disease->setP2_DUR(safe_assign_int(diseaseData["P2"].getOriginal()));

  // Implement debugging expression to test expression functionality without impacting evaluation.
  disease->setDEBUG(diseaseData["DEBUG"]);

  // Use the *disease to find a new slot in the simulators 
  // vector and then initialize a new Simulator inside it.
  simulators.emplace_back(std::make_unique<Simulator>(disease, ci));
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

  /** Call the rate function for the Plant */
  Plant::getInstance()->rate();
}

void Manager::integration() {
  /** Call the integration function for the Plant */
  Plant::getInstance()->integration();
  
  for (auto& simulator : simulators) {
    simulator->integration();
  }
}

void Manager::output() {
  for (auto& simulator : simulators) {
    simulator->output();
  }

  /** Call the integration function for the Plant */
  Plant::getInstance()->output();
}

std::string hashDisease(std::string diseaseName) {
  return Utilities::base52Encode(std::hash<std::string>{}(diseaseName));
}

// Helper function to extract the variable key from a placeholder like "$KEY" or "$KEY:TRNO5"
std::string extractPlaceholderKey(const std::string& placeholder) {
  size_t colonPos = placeholder.find(':');
  if (colonPos != std::string::npos) {
    return placeholder.substr(1, colonPos - 1); // Between '$' and ':'
  }
  return placeholder.substr(1); // Just remove leading '$'
}

// Helper function to validate treatment selector format
// Valid formats: empty (no selector), "TRNOx" where x is digits, or "DEFAULT"
// Invalid: just a number like "5" or "123"
void validateTreatmentSelector(const std::string& selector) {
  if (selector.empty()) {
    return; // Empty is valid (bare placeholder like $SOURCE)
  }
  
  if (selector == "DEFAULT") {
    return; // DEFAULT is valid
  }
  
  // Check if it starts with TRNO
  if (selector.rfind("TRNO", 0) == 0) {
    std::string numberPart = selector.substr(4);
    // Verify the rest is all digits
    if (!numberPart.empty() && std::all_of(numberPart.begin(), numberPart.end(), ::isdigit)) {
      return; // Valid TRNOx format
    }
  }
  
  // If we get here, the format is invalid
  throw std::invalid_argument("Invalid treatment selector format '" + selector + "' in placeholder. "
                              "Expected format: ':TRNOx' (where x is a number), ':DEFAULT', or no selector. "
                              "Number-only selectors like ':5' are not allowed.");
}

// Helper function to extract the treatment selector from a placeholder like "$KEY:TRNO5" or "$KEY:DEFAULT"
std::string extractTreatmentSelector(const std::string& placeholder) {
  size_t colonPos = placeholder.find(':');
  if (colonPos == std::string::npos) {
    return "";
  }
  
  std::string selector = placeholder.substr(colonPos + 1);
  validateTreatmentSelector(selector);
  return selector;
}

// Helper function to get a source value from a YAML node, handling treatment-keyed maps
std::string getSourceValue(const YAML::Node& sourceNode, const std::string& treatmentKey) {
  if (!sourceNode) {
    return "";
  }

  std::string valueStr;
  
  if (sourceNode.IsScalar()) {
    try {
      valueStr = sourceNode.as<std::string>();
    } catch (const std::exception &e) {
      valueStr = "";
    }
  } else if (sourceNode.IsSequence()) {
    // Join sequence elements with commas
    std::ostringstream joined;
    for (std::size_t si = 0; si < sourceNode.size(); ++si) {
      if (si) joined << ",";
      if (sourceNode[si].IsScalar()) {
        try { joined << sourceNode[si].as<std::string>(); } catch (...) { /* ignore */ }
      } else {
        std::string dumped = YAML::Dump(sourceNode[si]);
        dumped.erase(std::remove(dumped.begin(), dumped.end(), '\n'), dumped.end());
        joined << dumped;
      }
    }
    valueStr = joined.str();
  } else if (sourceNode.IsMap()) {
    // Treatment-keyed map: look for the specific treatment or DEFAULT
    if (sourceNode[treatmentKey]) {
      valueStr = getSourceValue(sourceNode[treatmentKey], "");
    } else if (sourceNode["DEFAULT"]) {
      valueStr = getSourceValue(sourceNode["DEFAULT"], "");
    }
  } else {
    // For other types, dump to string
    std::string dumped = YAML::Dump(sourceNode);
    dumped.erase(std::remove(dumped.begin(), dumped.end(), '\n'), dumped.end());
    valueStr = dumped;
  }
  
  return valueStr;
}

std::string replacePlaceholders(std::string originalValue, std::string originalType, YAML::Node disease, const std::string& currentTrtKey = ""){
  std::string valueStr = originalValue;
  
  // First, check for invalid selector formats like $VAR:5 (number-only) or $VAR:INVALID
  std::smatch invalidMatch;
  
  if (std::regex_search(originalValue, invalidMatch, GDM::RegexPatterns::INVALID_SELECTOR_PATTERN)) {
    std::string invalidPlaceholder = invalidMatch.str(); // This is "$VAR:" without the selector
    
    // Find where the match ends in the original string to extract the actual invalid selector
    size_t matchPos = originalValue.find(invalidPlaceholder);
    size_t selectorStartPos = matchPos + invalidPlaceholder.length();
    
    // Extract everything after the colon until we hit a space, ), operator, or end of string
    std::string invalidSelector;
    size_t i = selectorStartPos;
    while (i < originalValue.length() && !std::isspace(originalValue[i]) && 
           originalValue[i] != ')' && originalValue[i] != '+' && originalValue[i] != '-' && 
           originalValue[i] != '*' && originalValue[i] != '/' && originalValue[i] != ':') {
      invalidSelector += originalValue[i];
      i++;
    }
    
    std::string key = extractPlaceholderKey(invalidPlaceholder);
    
    throw std::invalid_argument(
      "Invalid treatment selector format in placeholder '" + invalidPlaceholder + invalidSelector + "'. "
      "The selector '" + invalidSelector + "' is not recognized. "
      "Expected format: ':TRNOx' (where x is a number), ':DEFAULT', or no selector. "
      "Number-only selectors like ':5' are not allowed."
    );
  }
  
  // Look for the special character '$' which indicates a variable reference.
  // If the variable reference is not found, then return the original value.
  // If the variable reference is found, then look for the variable in the rest of the file.

  std::smatch matchResults;

  bool replaced = true;

  while (replaced){
    replaced = false;
    std::string tempStr = originalValue;

    while (std::regex_search(tempStr, matchResults, GDM::RegexPatterns::VAR_PATTERN)){
      std::string placeholder = matchResults.str();
      std::string key = extractPlaceholderKey(placeholder);
      std::string treatmentSelector = extractTreatmentSelector(placeholder);
      
      // Then, look for the variable name in the disease YAML::Node.
      if (disease[key] && disease[key]["VALUE"]) {
        std::string valueStr;
        
        // Determine which treatment to use for source lookup
        std::string effectiveTrtKey = treatmentSelector.empty() ? currentTrtKey : treatmentSelector;
        // std::cout << "Looking up value for placeholder '" << placeholder << "' with key '" << key << "' and treatment selector '" << treatmentSelector << "' (effective treatment key: '" << effectiveTrtKey << "')" << std::endl;
        // Get the source value, handling treatment-keyed maps appropriately
        valueStr = getSourceValue(disease[key]["VALUE"], effectiveTrtKey);

        // OPTIMIZATION: Skip inlining if the value contains FIO references (#{...})
        // This allows Expression class to handle variable resolution more efficiently
        // and prevents issues with nested FIO reference translation
        if (std::regex_search(valueStr, GDM::RegexPatterns::FIO_PATTERN)) {
        //  std::cerr << "Skipping inline substitution for '" << placeholder << "' because source contains FIO references - will be resolved at expression evaluation time." << std::endl << std::flush;
        //  std::cerr << "[REPL] Value preserved: " << valueStr << std::endl << std::flush;
          tempStr = matchResults.suffix(); // Move past this match without replacing
          continue;
        }

        if (originalType == "std::string" || originalType == "string"){
          valueStr = '(' + valueStr + ')';
        }

        // Finally, replace the variable reference with the actual value.
        // Use direct string replacement instead of regex_replace to avoid recompiling regex
        size_t matchPos = originalValue.find(placeholder);
        if (matchPos != std::string::npos) {
          originalValue.replace(matchPos, placeholder.length(), valueStr);
          replaced = true;
        }
      }
      // Being sure to keep checking the rest of the string for placeholders.
      tempStr = matchResults.suffix();
    }
  }

  // std::cout << "Final replaced value for '" << valueStr << "': " << originalValue << std::endl;
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
            value["VALUE"] = replacePlaceholders(originalValue, originalType, disease, trtKey);
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
              value["VALUE"][i] = replacePlaceholders(originalValue, originalType, disease, trtKey);
            }
          } else if (value["VALUE"].Type() == 4) {
            // Iterate through each treatment's key-value pair.
            // Accepted keys are TRNO01-99 and DEFAULT. 
            for (auto trtValueIt = value["VALUE"].begin(); trtValueIt != value["VALUE"].end(); ++trtValueIt) {
              std::string nodeTrtKey = trtValueIt->first.as<std::string>();
              YAML::Node trtValue = trtValueIt->second;
              if (nodeTrtKey.rfind("TRNO", 0) == 0 || nodeTrtKey == "DEFAULT") {
                if (trtValue.IsScalar()) {
                  std::string originalValue = trtValue.as<std::string>();
                  std::string originalType = "string";
                  if (value["TYPE"] && value["TYPE"].IsScalar()) {
                    originalType = value["TYPE"].as<std::string>();
                  }
                  value["VALUE"][nodeTrtKey] = replacePlaceholders(originalValue, originalType, disease, trtKey);
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
      CustomOutputHolder customOutputs;
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
              } else if (key == "CUSTOM_OUTPUT") {
                for (auto outIt = value.begin(); outIt != value.end(); ++outIt) {
                  std::string outputName = outIt->first.as<std::string>();
                  YAML::Node outputData = outIt->second;

                  if (outputName == "FILE_NAME" || outputName == "OUTPUT_FILE") {
                    try {
                      std::string outputFileName;
                      if (outputData.IsMap()) {
                        if (outputData[trtKey]) {
                          outputFileName = outputData[trtKey].as<std::string>();
                        } else if (outputData["DEFAULT"]) {
                          outputFileName = outputData["DEFAULT"].as<std::string>();
                        } else if (outputData["VALUE"]) {
                          outputFileName = outputData["VALUE"].as<std::string>();
                        } else {
                          throw std::invalid_argument("ERROR: No matching treatment key '" + trtKey + "', 'DEFAULT', or 'VALUE' found for CUSTOM_OUTPUT FILE_NAME.");
                        }
                      } else {
                        outputFileName = outputData.as<std::string>();
                      }
                      customOutputs.setFileName(outputFileName);
                    } catch (const std::exception& e) {
                      std::cout << "Error parsing custom output file name: "
                                << e.what() << std::endl;
                    }
                    continue;
                  }

                  if (outputName == "FORMAT") {
                    try {
                      customOutputs.setFormat(outputData.as<std::string>());
                    } catch (const std::exception& e) {
                      std::cout << "Error parsing custom output format: "
                                << e.what() << std::endl;
                    }
                    continue;
                  }

                  if (!outputData["VALUE"]) {
                    std::cout << "Warning: Custom output " << outputName
                              << " missing VALUE field" << std::endl;
                    continue;
                  }

                  try {
                    std::string outputExpression;
                    if (outputData["VALUE"].Type() == 4) {
                      if (outputData["VALUE"][trtKey]) {
                        outputExpression = outputData["VALUE"][trtKey].as<std::string>();
                      } else if (outputData["VALUE"]["DEFAULT"]) {
                        outputExpression = outputData["VALUE"]["DEFAULT"].as<std::string>();
                      } else {
                        throw std::invalid_argument("ERROR: No matching treatment key '" + trtKey + "' or 'DEFAULT' found for custom output '" + outputName + "'.");
                      }
                    } else {
                      outputExpression = outputData["VALUE"].as<std::string>();
                    }

                    std::string outputType = "string";
                    if (outputData["TYPE"] && outputData["TYPE"].IsScalar()) {
                      outputType = outputData["TYPE"].as<std::string>();
                    }

                    outputExpression = replacePlaceholders(outputExpression, outputType, disease, trtKey);

                    std::string outputDesc = "";
                    if (outputData["DESC"] && outputData["DESC"].IsScalar()) {
                      outputDesc = outputData["DESC"].as<std::string>();
                    }

                    customOutputs.add(outputName, outputExpression, outputDesc);
                  } catch (const std::exception& e) {
                    std::cout << "Error parsing custom output " << outputName
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

        // ============================================================================
        // Step 3b is to handle family grouping.
        // std::string family = diseaseData["FAMILY"].getOriginal();

        // std::shared_ptr<CloudF> cloudFPtr = nullptr;
        
        // if (!uniqueFamilies.contains(family)) {
        //   std::cout << "Will create new CloudF for disease family: " << family << std::endl;
        //   uniqueFamilies.families[family] = cloudParams;

        //   manager->addUniqueFamily(family);
        // } else {
        //   std::cout << "Will use existing CloudF for shared inoculum: " << cpIDToStr(tempCP) << std::endl;

        //   if (cloudParams != uniqueFamilies.families[family]) {
        //     throw std::runtime_error("Error: Cloud parameters for family " + family + " do not match previous definition.");
        //   }
        // }
        
        // try 3b again but considering 1 cloud per disease instead
        // family grouping will be tracked by the getInoculum function that uses family context
        
        // Can we just let the simulator creation create a new CloudF for each one initialized?

        // ============================================================================


        // ===== STEP 3C: Set up Expression variable context =====
        // Before expressions are evaluated, provide the variable context so that
        // $VARIABLE_NAME references can be resolved. This is more efficient than
        // inlining variables during YAML parsing.

        std::unordered_map<std::string, std::string> variableContext;
        for (const auto& pair : diseaseData) {
          variableContext[pair.first] = pair.second.getOriginal();
        }

        for (auto& pair : diseaseData) {
          pair.second.setContext(variableContext);
        }

        
        manager->addSimulator(diseaseData, ciPtr, rateInjections, integrationInjections, outputInjections, customOutputs);
      }
    }
  }

  manager->setCouplingPointIDs(uniqueCPs);
  // No more diseases found in the YAML file.
  return 1;
} 

void Manager::updateCurrentYearDoy(YearDoy yearDoy) {
  // Check if the currentGDMDate is less than the new yearDoy
  FlexibleIO* fio = FlexibleIO::getInstance();
  while(currentGDMDate.addOneDay() <= yearDoy) {
    fio->setIntegerMemory("CONTROL", "YEARDOY", currentGDMDate); 

    // Simulate the currentGDMDate
    rate();
    integration();
    output();

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
