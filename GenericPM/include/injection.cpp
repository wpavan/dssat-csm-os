#include "injection.h"
#include "manager.h"
#include "utilities.h"
#include "equation_context.h"
#include "numericstringcache.h"

static std::mt19937 rng(std::random_device{}());

static const double inverse_sqrt_2pi = 0.3989422804014337;

// NOTE: We should add a function for automatic gdd calculation.
// Define custom functions for tinyexpr for the user.
static double TE_max(double a, double b) {
    return (a > b) ? a : b;
}

static double TE_min(double a, double b) {
    return (a < b) ? a : b;
}

static double TE_amp_gaussian(double x, double mean, double stddev, double amplitude) {
    double a = (x - mean) / stddev;
    return amplitude * std::exp(-0.5 * a * a);
}

static double TE_norm_gaussian(double x, double mean, double stddev) {
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

static double TE_bounded_beta(double x, double max, double opt, double min) {
    if (x == -99 || max == -99.0 || opt == -99.0 || min == -99.0) {
        return 0.0;
    } else if (x < min || x > max) {
        return 0.0;
    } else if (min >= opt || opt >= max) {
        return 1.0;
    } else if (!std::isfinite(x) || !std::isfinite(min) || !std::isfinite(opt) || !std::isfinite(max)) {
        return 0.0;
    } else {
        return std::fmin(std::fmax(std::pow((x - min) / (opt - min), (opt - min) / (max - min)) * std::pow((max - x) / (max - opt), (max - opt) / (max - min)), 0.0), 1.0);
    }
}

static double TE_unit_beta(double x, double max, double opt, double min) {
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
    return fio->getReal(groupStr, varnameStr);
}

static double TE_fio_real_yrdoy(double group, double yrdoy, double varname) {
    FlexibleIO* fio = FlexibleIO::getInstance();
    FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();
    std::string groupStr = converter->decode(group);
    std::string varnameStr = converter->decode(varname);

    if (yrdoy == -1) {
        // This is the case where the second part of the FIO reference was a sim date keyword like CURRENT_YRDOY.
        int currentYrdoy = fio->getInteger("CONTROL", "CURRENT_YRDOY");
        return fio->getRealYrdoy(groupStr, std::to_string(currentYrdoy), varnameStr);

    }
    return fio->getRealYrdoy(groupStr, std::to_string((int)yrdoy), varnameStr);
}

static double TE_fio_real_index(double group, double varname, double index) {
    FlexibleIO* fio = FlexibleIO::getInstance();
    FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();
    std::string groupStr = converter->decode(group);
    std::string varnameStr = converter->decode(varname);
    return fio->getRealIndex(groupStr, varnameStr, (int)index);
}


namespace {
    struct FunctionRegistrar {
        FunctionRegistrar() {
            getCustomFunctions().register_context_function({"max", TE_max});
            getCustomFunctions().register_context_function({"min", TE_min});
            getCustomFunctions().register_context_function({"norm_gaussian", TE_norm_gaussian});
            getCustomFunctions().register_context_function({"gaussian", TE_norm_gaussian});
            getCustomFunctions().register_context_function({"amp_gaussian", TE_amp_gaussian});
            getCustomFunctions().register_context_function({"rand_unif", TE_rand_unif});
            getCustomFunctions().register_context_function({"rand_norm", TE_rand_norm});
            getCustomFunctions().register_context_function({"bounded_beta", TE_bounded_beta});
            getCustomFunctions().register_context_function({"unit_beta", TE_unit_beta});
            getCustomFunctions().register_context_function({"trapezoidal", TE_trapezoidal});
            getCustomFunctions().register_context_function({"triangular", TE_triangular});
            getCustomFunctions().register_context_function({"linear", TE_linear});
            getCustomFunctions().register_context_function({"FIO_REAL", TE_fio_real});
            getCustomFunctions().register_context_function({"FIO_REAL_YRDOY", TE_fio_real_yrdoy});
            getCustomFunctions().register_context_function({"FIO_REAL_INDEX", TE_fio_real_index});
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
