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
#include "../tinycc/libtcc.h"

#include <string>
#include <cstring>
#include <iostream>

char testCode[] = 
"float rate(){\n"
"    printf(\"Hello from TCC!\");\n"
"    return 1.0f;\n"
"}\n";

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

class TCCUtilities {
public:
    static float evalExternalCode(const char *code, const char *phase) {
        TCCState *s = tcc_new();
        if (s == nullptr) {
            std::cerr << "Failed to create TCC state." << std::endl;
            return -1;
        }

        tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

        if (tcc_set_options(s, "-nostdlib") < 0) {
            std::cerr << "Failed to set TCC options." << std::endl;
            tcc_delete(s);
            return -1;
        }

        if (tcc_compile_string(s, code) < 0) {
            std::cerr << "Failed to compile code." << std::endl;
            tcc_delete(s);
            return -1;
        }

        if (tcc_relocate(s) < 0) {
            std::cerr << "Failed to relocate code." << std::endl;
            tcc_delete(s);
            return -1;
        }
        
        void *handle = tcc_get_symbol(s, phase);
        if (handle == nullptr) {
            std::cerr << "Failed to get symbol '" << phase << "'." << std::endl;
            tcc_delete(s);
            return -1;
        }
        // Cast the symbol to the correct function pointer type
        auto func = reinterpret_cast<float(*)()>(handle);
        float result = func();

        tcc_delete(s);
        return result;
    }
};

#endif // UTILITIES_H
