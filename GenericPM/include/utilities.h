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
        te_variable vars[] = {{"x", &value}};
        int err;
        te_expr *expr = te_compile(expression_string.c_str(), vars, 1, &err);
        return te_eval(expr);
    }
};

#endif // UTILITIES_H
