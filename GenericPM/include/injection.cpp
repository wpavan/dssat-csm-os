#include "injection.h"
#include "manager.h"
#include "utilities.h"
#include "equation_context.h"
#include "numericstringcache.h"

static std::mt19937 rng(std::random_device{}());

static const double inverse_sqrt_2pi = 0.3989422804014337;

// NOTE: We should add a function for automatic gdd calculation.
// Define custom functions for tinyexpr for the user.
static double TE_amp_gaussian(double x, double mean, double stddev, double amplitude) {
    if (stddev <= 0.0) {
        throw std::invalid_argument("Gaussian function: standard deviation (sigma) must be positive (> 0)");
    }
    if (amplitude < 0.0) {
        throw std::invalid_argument("Gaussian function: amplitude must be non-negative (>= 0)");
    }
    double a = (x - mean) / stddev;
    return amplitude * std::exp(-0.5 * a * a);
}

static double TE_norm_gaussian(double x, double mean, double stddev) {
    if (stddev <= 0.0) {
        throw std::invalid_argument("Gaussian function: standard deviation (sigma) must be positive (> 0)");
    }
    double a = (x - mean) / stddev;
    return (inverse_sqrt_2pi * stddev) * std::exp(-0.5 * a * a);
}

static double TE_rand_unif(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(rng);
}

static double TE_rand_norm(double mean, double stddev) {
    std::normal_distribution<double> dist(mean, stddev);
    return dist(rng);
}

static double TE_weibull_pdf(double x, double shape, double scale) {
    if (shape <= 0.0) {
        throw std::invalid_argument("Weibull PDF: shape parameter must be positive (> 0)");
    }
    if (scale <= 0.0) {
        throw std::invalid_argument("Weibull PDF: scale parameter must be positive (> 0)");
    }
    if (x < 0.0) {
        return 0.0;
    }
    double a = std::pow(x / scale, shape - 1);
    double b = std::exp(-std::pow(x / scale, shape));
    return (shape / scale) * a * b;
}

static double TE_weibull_cdf(double x, double shape, double scale) {
    if (shape <= 0.0) {
        throw std::invalid_argument("Weibull CDF: shape parameter must be positive (> 0)");
    }
    if (scale <= 0.0) {
        throw std::invalid_argument("Weibull CDF: scale parameter must be positive (> 0)");
    }
    if (x < 0.0) {
        return 0.0;
    }
    return 1.0 - std::exp(-std::pow(x / scale, shape));
}

static double TE_beta(double x, double max, double opt, double min) {
    if (x == -99 || max == -99.0 || opt == -99.0 || min == -99.0) {
        return 0.0;
    } else if (x < min || x > max) {
        return 0.0;
    } else if (min >= opt || opt >= max) {
        throw std::invalid_argument("Beta function: parameters do not follow the order min < opt < max");
    } else if (!std::isfinite(x) || !std::isfinite(min) || !std::isfinite(opt) || !std::isfinite(max)) {
        return 0.0;
    } else {
        return std::fmin(std::fmax(std::pow((x - min) / (opt - min), (opt - min) / (max - min)) * std::pow((max - x) / (max - opt), (max - opt) / (max - min)), 0.0), 1.0);
    }
}

static double TE_logistic(double x, double a, double h) {
    return 1.0 / (1.0 + std::exp(-a * (x - h)));
}

static double TE_trapezoidal(double x, double max, double opt_max, double opt_min, double min) {
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

static double TE_triangular(double x, double max, double opt, double min) {
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

static double TE_linear(double x, double x_at_max, double x_at_min) {
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

static double TE_fio_real(double group, double varname) {
    FlexibleIO* fio = FlexibleIO::getInstance();
    FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();
    std::string groupStr = converter->decode(group);
    std::string varnameStr = converter->decode(varname);
    float fioValue = fio->getReal(groupStr, varnameStr);
    if (fioValue == -99.0f) {
        std::cerr << "Warning: FIO_REAL returned -99.0f for group '" << groupStr << "', variable '" << varnameStr << "'. This may indicate a missing value or invalid index." << std::endl;
    }
    return fioValue;
}

static double TE_fio_real_yrdoy(double group, double yrdoy, double varname) {
    FlexibleIO* fio = FlexibleIO::getInstance();
    FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();
    std::string groupStr = converter->decode(group);
    std::string varnameStr = converter->decode(varname);

    float fioValue;
    
    yrdoy = yrdoy == -1 ? fio->getInteger("CONTROL", "YEARDOY") : yrdoy;
    fioValue = fio->getRealYrdoy(groupStr, std::to_string((int)yrdoy), varnameStr);

    if (fioValue == -99.0f) {
        std::cerr << "Warning: FIO_REAL_YRDOY returned -99.0f for group '" << groupStr << "', variable '" << varnameStr << "', YEARDOY " << (int)yrdoy << ". This may indicate a missing value or invalid index." << std::endl;
    }
    return fioValue;
}

static double TE_fio_real_index(double group, double varname, double index) {
    FlexibleIO* fio = FlexibleIO::getInstance();
    FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();
    std::string groupStr = converter->decode(group);
    std::string varnameStr = converter->decode(varname);
    float fioValue = fio->getRealIndex(groupStr, varnameStr, (int)index);
    if (fioValue == -99.0f) {
        std::cerr << "Warning: FIO_REAL_INDEX returned -99.0f for group '" << groupStr << "', variable '" << varnameStr << "', index " << (int)index << ". This may indicate a missing value or invalid index." << std::endl;
    }
    return fioValue;
}

// NOTE: I should combine the backend of the ABOVE, BELOW, and BETWEEN functions to help with readability.
static double TE_hours_VAR_above(double var, double yrdoy, double threshold) {
    FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();
    FlexibleIO* fio = FlexibleIO::getInstance();
    std::string varStr = converter->decode(var);

    // Convert a date from the "TODAY" placeholder to the actual current date.
    if (yrdoy == -1) {
        yrdoy = fio->getInteger("CONTROL", "YEARDOY");
    }

    std::string yrdoyStr = std::to_string((int)yrdoy);

    std::vector<float> hourlyValues;
    hourlyValues.reserve(24);

    std::string hourVarName = varStr;
    size_t baseLen = varStr.length();

    // Collect all of the hourly values for the variable on the specified date.
    for (int i = 1; i <= 24; i++) {
        hourVarName.resize(baseLen);
        if (i < 10) {
            hourVarName += char('0');
            hourVarName += char('0' + i);
        } else {
            hourVarName += std::to_string(i);
        }
        hourlyValues.push_back(fio->getRealYrdoy("WTH", yrdoyStr, hourVarName));
    }

    int hoursAbove = 0;
    // Iterate through the 24 hours of the day to count hours of the variable over the threshold.
    for (const float val : hourlyValues) {
        if (val > threshold && val != -99.0f) {
            hoursAbove++; 
        }
    }
    return hoursAbove;
}

static double TE_hours_VAR_below(double var, double yrdoy, double threshold) {
    FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();
    FlexibleIO* fio = FlexibleIO::getInstance();
    std::string varStr = converter->decode(var);

    // Convert a date from the "TODAY" placeholder to the actual current date.
    if (yrdoy == -1) {
        yrdoy = fio->getInteger("CONTROL", "YEARDOY");
    }

    std::string yrdoyStr = std::to_string((int)yrdoy);

    std::vector<float> hourlyValues;
    hourlyValues.reserve(24);

    std::string hourVarName = varStr;
    size_t baseLen = varStr.length();

    // Collect all of the hourly values for the variable on the specified date.
    for (int i = 1; i <= 24; i++) {
        hourVarName.resize(baseLen);
        if (i < 10) {
            hourVarName += char('0');
            hourVarName += char('0' + i);
        } else {
            hourVarName += std::to_string(i);
        }
        hourlyValues.push_back(fio->getRealYrdoy("WTH", yrdoyStr, hourVarName));
    }

    int hoursBelow = 0;
    // Iterate through the 24 hours of the day to count hours of the variable under the threshold.
    for (const float val : hourlyValues) {
        if (val < threshold && val != -99.0f) {
            hoursBelow++; 
        }
    }
    return hoursBelow;
}

static double TE_hours_VAR_between(double var, double yrdoy, double upperThreshold, double lowerThreshold) {
    FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();
    FlexibleIO* fio = FlexibleIO::getInstance();
    std::string varStr = converter->decode(var);

    // Convert a date from the "TODAY" placeholder to the actual current date.
    if (yrdoy == -1) {
        yrdoy = fio->getInteger("CONTROL", "YEARDOY");
    }

    std::string yrdoyStr = std::to_string((int)yrdoy);

    std::vector<float> hourlyValues;
    hourlyValues.reserve(24);

    std::string hourVarName = varStr;
    size_t baseLen = varStr.length();

    // Collect all of the hourly values for the variable on the specified date.
    for (int i = 1; i <= 24; i++) {
        hourVarName.resize(baseLen);
        if (i < 10) {
            hourVarName += char('0');
            hourVarName += char('0' + i);
        } else {
            hourVarName += std::to_string(i);
        }
        hourlyValues.push_back(fio->getRealYrdoy("WTH", yrdoyStr, hourVarName));
    }

    int hoursBetween = 0;
    // Iterate through the 24 hours of the day to count hours of the variable between the thresholds.
    for (const float val : hourlyValues) {
        if (val > lowerThreshold && val < upperThreshold && val != -99.0f) {
            hoursBetween++;
        }
    }
    return hoursBetween;
}

// NOTE: Implement some sort of high temperature kill option
static double TE_growing_degree_days(double baseTemp) {
    FlexibleIO* fio = FlexibleIO::getInstance();
    int currentYrdoy = fio->getInteger("CONTROL", "YEARDOY");
    double dailyAvgTemp = fio->getRealYrdoy("WTH", std::to_string(currentYrdoy), "TAVG");
    return std::max(0.0, dailyAvgTemp - baseTemp);
}

static double TE_growing_degree_days_max(double baseTemp, double maxTemp) {
    FlexibleIO* fio = FlexibleIO::getInstance();
    int currentYrdoy = fio->getInteger("CONTROL", "YEARDOY");
    double dailyAvgTemp = fio->getRealYrdoy("WTH", std::to_string(currentYrdoy), "TAVG");
    std::cerr << "Debug: In TE_growing_degree_days_max, dailyAvgTemp = " << dailyAvgTemp << ", baseTemp = " << baseTemp << ", maxTemp = " << maxTemp << std::endl;
    return std::max(0.0, std::min(dailyAvgTemp, maxTemp) - baseTemp);
}

namespace {
    struct FunctionRegistrar {
        FunctionRegistrar() {
            getCustomFunctions().register_context_function({"logistic", TE_logistic});
            getCustomFunctions().register_context_function({"norm_gaussian", TE_norm_gaussian});
            getCustomFunctions().register_context_function({"gaussian", TE_norm_gaussian});
            getCustomFunctions().register_context_function({"amp_gaussian", TE_amp_gaussian});
            getCustomFunctions().register_context_function({"weibull_pdf", TE_weibull_pdf});
            getCustomFunctions().register_context_function({"weibull_cdf", TE_weibull_cdf});
            getCustomFunctions().register_context_function({"rand_unif", TE_rand_unif});
            getCustomFunctions().register_context_function({"rand_norm", TE_rand_norm});
            getCustomFunctions().register_context_function({"beta", TE_beta});
            getCustomFunctions().register_context_function({"trapezoidal", TE_trapezoidal});
            getCustomFunctions().register_context_function({"triangular", TE_triangular});
            getCustomFunctions().register_context_function({"linear", TE_linear});
            getCustomFunctions().register_context_function({"FIO_REAL", TE_fio_real});
            getCustomFunctions().register_context_function({"FIO_REAL_YRDOY", TE_fio_real_yrdoy});
            getCustomFunctions().register_context_function({"FIO_REAL_INDEX", TE_fio_real_index});
            getCustomFunctions().register_context_function({"HOURS_VAR_ABOVE", TE_hours_VAR_above});
            getCustomFunctions().register_context_function({"HOURS_VAR_BELOW", TE_hours_VAR_below});
            getCustomFunctions().register_context_function({"HOURS_VAR_BETWEEN", TE_hours_VAR_between});
            getCustomFunctions().register_context_function({"gdd", TE_growing_degree_days});
            getCustomFunctions().register_context_function({"gdd_max", TE_growing_degree_days_max});
            getCustomFunctions().register_context_function({"GDD", TE_growing_degree_days});
        }
    };

    static FunctionRegistrar registrar;
}

double Injection::eval() {
    try {
        return expression->evaluate();
    } catch (const std::runtime_error& e) {
        std::cerr << "Error evaluating expression for Injection with raw endpoint '" << rawEndpoint << "': " << e.what() << std::endl;
        throw e;
    }
}

void Injection::apply(float& endpointValue) {
    double evalResult;
    try {
        evalResult = expression->evaluate();
    } catch (const std::runtime_error& e) {
        throw e; // Rethrow for the caller in
    }

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
    if (std::regex_match(modifStr, GDM::RegexPatterns::ADD_PATTERN)) {
        return ModificationType::ADD;
    } else if (std::regex_match(modifStr, GDM::RegexPatterns::SUBTRACT_PATTERN)) {
        return ModificationType::SUBTRACT;
    } else if (std::regex_match(modifStr, GDM::RegexPatterns::MULTIPLY_PATTERN)) {
        return ModificationType::MULTIPLY;
    } else if (std::regex_match(modifStr, GDM::RegexPatterns::DIVIDE_PATTERN)) {
        return ModificationType::DIVIDE;
    } else if (std::regex_match(modifStr, GDM::RegexPatterns::ASSIGN_PATTERN)) {
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
    if (endpointStr == "INFECTION_BIOLOGICAL_FACTOR") {
        return InjEndpoint::INFECTION_BIOLOGICAL_FACTOR;
    } else {
        return InjEndpoint::FIO;
    }
}
