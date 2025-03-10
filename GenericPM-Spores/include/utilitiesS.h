#ifndef UTILITIESS_H
#define UTILITIESS_H

#include <string>
#include <cstring>
#include <iostream>
#include "../../GenericPM/TinyExpr/tinyexpr.h"

class UtilitiesS {
public:

    static float trapezoidalFunctionS(float value, float v[]);
    static float temperatureFavorabilityS(float temp, float cardinalTemperatures[]);
    static float wetnessFavorabilityS(float wetDuration, std::string wetnessFunction);
    static float dispersalRainFunctionS(float rain, std::string dispersionFreequency);
    static float runExpressionFunctionS(float value, std::string expressionString);
    static float getHealthAreaProportionS(float diseaseArea, float totalArea, float senescedArea);
    static std::string formatfloatS(float value);
    static std::string formatfloatS(float value, int decimals);
    static int addOneDayS(int yearDoy);
    static bool isLeapYearS(int year);
    static float growthFunction(std::string expression_string, float value);

    static float runExpression(std::string expression_string, float value) {
        double x = value;
        te_variable vars[] = {{"x", &x}};

        int err;
        te_expr *expr = te_compile(expression_string.c_str(), vars, 1, &err);
        
        if (err != 0 || expr == nullptr) {
            std::cout << "Error: " << err << std::endl;
        }
        
        float result = te_eval(expr);
        /*if (strchr(expression_string.c_str(), '^') != nullptr) {
            std::cout << "Expression: " << expression_string << " Value: " << value << " Result: " << result << std::endl;
        }*/

        te_free(expr);
        return result;
    }
};

#endif // UTILITIES_H
