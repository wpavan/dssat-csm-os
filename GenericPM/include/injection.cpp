#include "injection.h"
#include "manager.h"

double max_func(double a, double b) {
    return (a > b) ? a : b;
}

double min_func(double a, double b) {
    return (a < b) ? a : b;
}

// Define custom functions for tinyexpr for the user.
te_variable customFunctions[] = {
    {"max", (const void*)max_func, TE_FUNCTION2},
    {"min", (const void*)min_func, TE_FUNCTION2}
};

std::string Injection::parse() {
    std::regex fioPattern(R"(#\{([A-Za-z\s]+):([A-Za-z0-9_\s]+):?([A-Za-z0-9\s]+)?\})");
    std::regex simDatePattern(R"((?:(?:CURRENT|SIMULATION|SIM)(?: |_))?(?:YRDOY|DATE|TODAY|YYYYJJJ|YYYYDDD|YYYYDOY))");

    std::string result = rawExpression;
    
    // Process all matches from right to left to avoid position shifts
    std::vector<std::smatch> allMatches;
    std::sregex_iterator iter(rawExpression.begin(), rawExpression.end(), fioPattern);
    std::sregex_iterator end;
    
    // Collect all matches
    for (; iter != end; ++iter) {
        allMatches.push_back(*iter);
    }
    
    // Process matches in reverse order to maintain string positions
    for (auto it = allMatches.rbegin(); it != allMatches.rend(); ++it) {
        const std::smatch& match = *it;
        float value;
        
        std::string group = match[1].str();
        std::string second = match[2].str();
        std::string third = match[3].str();
        
        if (third.empty()) {
            // Two parts: #{GROUP:VARNAME}
            value = FlexibleIO::getInstance()->getReal(group, second);
        } else {
            // Three parts: #{GROUP:YRDOY:VARNAME} or #{GROUP:VARNAME:INDEX}
            // Check if second part is a year-day (length 7 and all digits)
            // Then check for specific keywords to indicate current simulation date.
            // Then check if it's an index (all digits).
            if (second.length() == 7 && std::all_of(second.begin(), second.end(), ::isdigit)) {
                value = FlexibleIO::getInstance()->getRealYrdoy(group, second, third);
            } else if (std::regex_match(second, simDatePattern)) {
                value = FlexibleIO::getInstance()->getRealYrdoy(group, std::to_string(Manager::getInstance()->getCurrentSimDate()), third);
            } else if (std::all_of(third.begin(), third.end(), ::isdigit)) {
                value = FlexibleIO::getInstance()->getRealIndex(group, third, std::stoi(second));
            } else {
                throw std::runtime_error("Error: Second part of FIO reference '" + second + "' is neither YRDOY nor is the third part of FIO reference: '" + third + "' an INDEX.");
            }
        }
        
        // Replace this specific match with its value
        result.replace(match.position(), match.length(), std::to_string(value));
    }
    
    return result;
}

float Injection::eval() {
    std::string parsedExpr = parse();
    int err;
    te_expr *n = te_compile(parsedExpr.c_str(), customFunctions, 2, &err);
    if (n) {
        float result = te_eval(n);
        te_free(n);
        return result;
    } else {
        throw std::runtime_error("Error: Failed to parse expression '" + parsedExpr + "' at position " + std::to_string(err));
    }
}

void Injection::apply(float& endpointValue) {
    switch (modification)
    {
    case ModificationType::ADD:
        endpointValue += eval();
        break;

    case ModificationType::SUBTRACT:
        endpointValue -= eval();
        break;

    case ModificationType::MULTIPLY:
        endpointValue *= eval();
        break;

    case ModificationType::DIVIDE:
        endpointValue /= eval();
        break;

    case ModificationType::ASSIGN:
        endpointValue = eval();
        break;

    default:
        throw std::runtime_error("Error: Unknown modification type in Injection::apply");
    }
}

// Function definitions
ModificationType parseModification(std::string modifStr) {
    // Define regex patterns for each modification type
    std::regex addPattern(R"((?:ADDITION|ADD|PLUS|\+=|\+))", std::regex::icase);
    std::regex subPattern(R"((?:SUBTRACTION|SUBTRACT|MINUS|-=|-))", std::regex::icase);
    std::regex multPattern(R"((?:MULTIPLICATION|MULTIPLY|MULT|TIMES|\*=|\*))", std::regex::icase);
    std::regex divPattern(R"((?:DIVISION|DIVIDE|DIV|\/=|\/))", std::regex::icase);
    std::regex asgnPattern(R"((?:ASSIGNMENT|REPLACEMENT|ASSIGN|REPLACE|EQUALS|=))", std::regex::icase);

    if (std::regex_match(modifStr, addPattern)) {
        return ModificationType::ADD;
    } else if (std::regex_match(modifStr, addPattern)) {
        return ModificationType::SUBTRACT;
    } else if (std::regex_match(modifStr, subPattern)) {
        return ModificationType::MULTIPLY;
    } else if (std::regex_match(modifStr, divPattern)) {
        return ModificationType::DIVIDE;
    } else if (std::regex_match(modifStr, asgnPattern)) {
        return ModificationType::ASSIGN;
    } else {
        throw std::runtime_error("Error: Unknown modification type '" + modifStr + "'");
    }
}

InjEndpoint parseEndpoint(std::string endpointStr) {
    if (endpointStr == "INOCULUM_GENERATION") {
        return InjEndpoint::INOCULUM_GEN;
    } else if (endpointStr == "INFECTION_BIOLOGICAL_FACTOR") {
        return InjEndpoint::INFECTION_BIOLOGICAL_FACTOR;
    } else if (endpointStr == "OUTPUT") {
        return InjEndpoint::OUTPUT;
    }
    // Add more endpoint parsing as needed.
    else {
        throw std::runtime_error("Error: Unknown injection endpoint '" + endpointStr + "'");
    }
}
