#ifndef UTILITIESS_H
#define UTILITIESS_H

#include<string>
#include "../TinyExpr/tinyexpr.h"

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

    static float growthFunction(std::string expression_string, float value) {
        te_variableS vars[] = {{"x", &value}};
        int err;
        te_exprS *expr = te_compileS(expression_string.c_str(), vars, 1, &err);
        return te_evalS(expr);
    }

    static float dispersalRainFunctionS(std::string expression_string, float value) {
        te_variableS vars[] = {{"x", &value}};
        int err;
        te_exprS *expr = te_compileS(expression_string.c_str(), vars, 1, &err);
        return te_evalS(expr);
    }

       static float runExpression(std::string expression_string, float value) {
        te_variableS vars[] = {{"x", &value}};
        int err;
        te_exprS *expr = te_compileS(expression_string.c_str(), vars, 1, &err);
        return te_evalS(expr);
    }
};

#endif // UTILITIES_H
