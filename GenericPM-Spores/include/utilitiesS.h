#ifndef UTILITIESS_H
#define UTILITIESS_H

#include<string>
#include "../TinyExpr/tinyexpr.h"

class UtilitiesS {
public:

    static double trapezoidalFunctionS(double value, double v[]);
    static double temperatureFavorabilityS(double temp, double cardinalTemperatures[]);
    static double wetnessFavorabilityS(double wetDuration, std::string wetnessFunction);
    static double dispersalRainFunctionS(double rain, std::string dispersionFreequency);
    static double runExpressionFunctionS(double value, std::string expressionString);
    static double getHealthAreaProportionS(double diseaseArea, double totalArea, double senescedArea);
    static std::string formatDoubleS(double value);
    static std::string formatDoubleS(double value, int decimals);
    static int addOneDayS(int yearDoy);
    static bool isLeapYearS(int year);

    static double growthFunction(std::string expression_string, double value) {
        te_variableS vars[] = {{"x", &value}};
        int err;
        te_exprS *expr = te_compileS(expression_string.c_str(), vars, 1, &err);
        return te_evalS(expr);
    }

    static double dispersalRainFunctionS(std::string expression_string, double value) {
        te_variableS vars[] = {{"x", &value}};
        int err;
        te_exprS *expr = te_compileS(expression_string.c_str(), vars, 1, &err);
        return te_evalS(expr);
    }

       static double runExpression(std::string expression_string, double value) {
        te_variableS vars[] = {{"x", &value}};
        int err;
        te_exprS *expr = te_compileS(expression_string.c_str(), vars, 1, &err);
        return te_evalS(expr);
    }
};

#endif // UTILITIES_H
