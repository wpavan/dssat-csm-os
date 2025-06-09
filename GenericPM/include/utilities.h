/**
 * @file utilities.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#ifndef UTILITIES_H
#define UTILITIES_H

#include "../TinyExpr/tinyexpr.h"

#include <string>
#include <cstring>
#include <iostream>

class Utilities {
public:
    static float trapezoidalFunction(float value, float v[]);
    static float temperatureFavorability(float temp, float cardinalTemperatures[]);
    static float wetnessFavorability(float wetDuration, std::string wetnessFunction);
    static float growthFunction(float value, std::string expression_string);
    static float runExpressionFunction(float value, std::string expressionString);
    static float getHealthAreaProportion(float diseaseArea, float totalArea, float senescedArea);
    static std::string formatfloat(float value);
    static std::string formatfloat(float value, int decimals);
    static int addOneDay(int yearDoy);
    static bool isLeapYear(int year);

    static float runExpression(std::string expression_string, float value) {
        double x = value;
        te_variable vars[] = {{"x", &x}};

        int err;
        te_expr *expr = te_compile(expression_string.c_str(), vars, 1, &err);

        
        if (err != 0 || expr == nullptr) {
            std::cout << "-----" << std::endl << "Expression: " << expression_string << std::endl 
            << "Error: " << err << std::endl << "-----" << std::endl;
        }

        float result = te_eval(expr);

        te_free(expr);
        return result;
    }
};

#endif // UTILITIES_H
