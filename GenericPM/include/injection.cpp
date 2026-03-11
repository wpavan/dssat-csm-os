#include <random>

#include "injection.h"
#include "manager.h"

static std::mt19937 rng(std::random_device{}());

static const double inverse_sqrt_2pi = 0.3989422804014337;

const std::regex Injection::fioPattern_(R"(#\{([A-Za-z\s]+):([A-Za-z0-9_\s]+):?([A-Za-z0-9\s]+)?\})");
const std::regex Injection::simDatePattern_(R"((?:(?:CURRENT|SIMULATION|SIM)(?: |_))?(?:YRDOY|DATE|TODAY|YYYYJJJ|YYYYDDD|YYYYDOY))");
 
const std::regex ModificationRegex::addPattern(R"((?:ADDITION|ADD|PLUS|\+=|\+))", std::regex::icase);
const std::regex ModificationRegex::subPattern(R"((?:SUBTRACTION|SUBTRACT|MINUS|-=|-))", std::regex::icase);
const std::regex ModificationRegex::multPattern(R"((?:MULTIPLICATION|MULTIPLY|MULT|TIMES|\*=|\*))", std::regex::icase);
const std::regex ModificationRegex::divPattern(R"((?:DIVISION|DIVIDE|DIV|\/=|\/))", std::regex::icase);
const std::regex ModificationRegex::asgnPattern(R"((?:ASSIGNMENT|REPLACEMENT|ASSIGN|REPLACE|EQUALS|=))", std::regex::icase);

double max_func(double a, double b) {
    return (a > b) ? a : b;
}

double min_func(double a, double b) {
    return (a < b) ? a : b;
}

double amp_gaussian_func(double x, double mean, double stddev, double amplitude) {
    double a = (x - mean) / stddev;
    return amplitude * std::exp(-0.5 * a * a);
}

double norm_gaussian_func(double x, double mean, double stddev) {
    double a = (x - mean) / stddev;
    return (inverse_sqrt_2pi * stddev) * std::exp(-0.5 * a * a);
}

double rand_unif_func(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng);
}

double rand_norm_func(double mean, double stddev) {
    std::normal_distribution<double> dist(mean, stddev);
    return dist(rng);
}

double bounded_beta_func(double x, double max, double opt, double min) {
    if (x == -99 || max == -99.0 || opt == -99.0 || min == -99.0) {
        return 0.0;
    } else if (x < min || x > max) {
        return 0.0;
    } else if (min >= opt || opt >= max) {
        return 1.0;
    } else if (!std::isfinite(x) || !std::isfinite(min) || !std::isfinite(opt) || !std::isfinite(max)) {
        return 0.0;
    } else {
        return fmin(fmax(std::pow((x - min) / (opt - min), (opt - min) / (max - min)) * std::pow((max - x) / (max - opt), (max - opt) / (max - min)), 0.0), 1.0);
    }
}

double unit_beta_func(double x, double max, double opt, double min) {
    double tf, a, b;

    b = ((max - opt) / (opt - min));
    a = (1 / ((opt - min) * pow(max - opt, b)));
    if(x > max) {
        x = max;
    } else if(x < min) {
        x = min;
    }
    tf = (a * (x - min) * pow(max - x, b));
    return (fmax(0,tf));
}

double trapezoidal_func(double x, double max, double opt_max, double opt_min, double min) {
    if (x <= min || x >= max) {
        return 0.0;
    } else if (x >= opt_min && x <= opt_max) {
        return 1.0;
    } else if (x > min && x < opt_min) {
        return (x - min) / (opt_min - min);
    } else { // x > c && x < d
        return (max - x) / (max - opt_max);
    }
}

double triangular_func(double x, double max, double opt, double min) {
    if (x <= min || x >= max) {
        return 0.0;
    } else if (x == opt) {
        return 1.0;
    } else if (x > min && x < opt) {
        return (x - min) / (opt - min);
    } else { // x > b && x < c
        return (max - x) / (max - opt);
    }
}

double linear_func(double x, double x_at_max, double x_at_min) {
    if (x_at_max > x_at_min) {
        if (x <= x_at_min) {
            return 0.0;
        } else if (x >= x_at_max) {
            return 1.0;
        } else {
            return (x - x_at_min) / (x_at_max - x_at_min);
        }
    } else {
        if (x <= x_at_max) {
            return 1.0;
        } else if (x >= x_at_min) {
            return 0.0;
        } else {
            return (x_at_min - x) / (x_at_min - x_at_max);
        }
    }
}

// Define custom functions for tinyexpr for the user.
te_variable customFunctions[] = {
    {"max", (const void*)max_func, TE_FUNCTION2},
    {"min", (const void*)min_func, TE_FUNCTION2},
    {"amp_gaussian", (const void*)amp_gaussian_func, TE_FUNCTION4},
    {"gaussian", (const void*)amp_gaussian_func, TE_FUNCTION4},
    {"gaussian", (const void*)norm_gaussian_func, TE_FUNCTION3},
    {"norm_gaussian", (const void*)norm_gaussian_func, TE_FUNCTION3},
    {"beta", (const void*)unit_beta_func, TE_FUNCTION4},
    {"trapezoidal", (const void*)trapezoidal_func, TE_FUNCTION4},
    {"triangular", (const void*)triangular_func, TE_FUNCTION3},
    {"linear", (const void*)linear_func, TE_FUNCTION3},
    {"rand_unif", (const void*)rand_unif_func, TE_FUNCTION2},
    {"rand_norm", (const void*)rand_norm_func, TE_FUNCTION2}
};

std::string Injection::parse(bool& missingVal) {
    std::string result = rawExpression;
    missingVal = false;
    
    // Process all matches from right to left to avoid position shifts
    std::vector<std::smatch> allMatches;
    std::sregex_iterator iter(rawExpression.begin(), rawExpression.end(), Injection::fioPattern_);
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
            } else if (std::regex_match(second, Injection::simDatePattern_)) {
                value = FlexibleIO::getInstance()->getRealYrdoy(group, std::to_string(Manager::getInstance()->getCurrentSimDate()), third);
            } else if (std::all_of(third.begin(), third.end(), ::isdigit)) {
                value = FlexibleIO::getInstance()->getRealIndex(group, third, std::stoi(second));
            } else {
                throw std::runtime_error("Error: Second part of FIO reference '" + second + "' is neither YRDOY nor is the third part of FIO reference: '" + third + "' an INDEX.");
            }
        }
        
        // Check if value was missing in FlexibleIO
        if (value == -99) {
            missingVal = true;
        }

        // Replace this specific match with its value
        result.replace(match.position(), match.length(), std::to_string(value));
    }
    
    return result;
}

std::string Injection::parse() {
    bool missingVal;
    return parse(missingVal);
}

double Injection::eval() {
    bool missingVal; 
    try {
        std::string parsedExpr = parse(missingVal);
        if (!missingVal) {
            int err = 0;
            te_expr *n = te_compile(parsedExpr.c_str(), customFunctions, sizeof(customFunctions) / sizeof(te_variable), &err);
            
            if (!n) {
                throw std::runtime_error("Expression compilation failed at position " + 
                                    std::to_string(err) + " in: " + parsedExpr);
            }
            
            double result = te_eval(n);
            te_free(n);
            
            if (!std::isfinite(result)) {
                throw std::runtime_error("Expression evaluation resulted in non-finite value");
            }
            // std::cout << "Evaluated expression: " << parsedExpr << " = " << result << std::endl;
            return result;
        } else {
            return -99.0f;
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Evaluation error: " + std::string(e.what()));
    }
}

void Injection::apply(float& endpointValue) {
    double evalResult = eval();  
    if (evalResult == -99.0f) {
        // Do not apply if the evaluated result indicates a missing value
        return;
    } else {
        switch (modification)
        {
        case ModificationType::ADD:
            endpointValue += evalResult;
            break;

        case ModificationType::SUBTRACT:
            endpointValue -= evalResult;
            break;

        case ModificationType::MULTIPLY:
            endpointValue *= evalResult;
            break;

        case ModificationType::DIVIDE:
            if (std::abs(evalResult) < std::numeric_limits<float>::epsilon()) {
                throw std::runtime_error("Error: Division by zero in Injection::apply");
            }
            endpointValue /= evalResult;
            break;

        case ModificationType::ASSIGN:
            endpointValue = evalResult;
            break;

        default:
            throw std::runtime_error("Error: Unknown modification type in Injection::apply");
        }
    }
}

void Injection::apply(std::string& endpointVarName) {
    endpointVarName = rawEndpoint;
}

void Injection::apply(float& endpointValue, std::string& endpointVarName) {
    Injection::apply(endpointValue);
    Injection::apply(endpointVarName);
}

// Function definitions
ModificationType parseModification(std::string modifStr) {
    if (std::regex_match(modifStr, ModificationRegex::addPattern)) {
        return ModificationType::ADD;
    } else if (std::regex_match(modifStr, ModificationRegex::subPattern)) {
        return ModificationType::SUBTRACT;
    } else if (std::regex_match(modifStr, ModificationRegex::multPattern)) {
        return ModificationType::MULTIPLY;
    } else if (std::regex_match(modifStr, ModificationRegex::divPattern)) {
        return ModificationType::DIVIDE;
    } else if (std::regex_match(modifStr, ModificationRegex::asgnPattern)) {
        return ModificationType::ASSIGN;
    } else {
        throw std::runtime_error("Error: Unknown modification type '" + modifStr + "'");
    }
}

InjEndpoint parseEndpoint(std::string endpointStr) {
    try {
        CouplingPointID cpID = strToCPID(endpointStr);
        return InjEndpoint::PCP;
    } catch (const std::invalid_argument&) {
        // The user has supplied an endpoint that is not a coupling point.
    }
    if (endpointStr == "INOCULUM") {
        return InjEndpoint::INOCULUM;
    } else if (endpointStr == "INFECTION_BIOLOGICAL_FACTOR") {
        return InjEndpoint::INFECTION_BIOLOGICAL_FACTOR;
    } else {
        return InjEndpoint::FIO;
    }
}
