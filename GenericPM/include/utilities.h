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

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

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

typedef float (*InjectionFunction)();

struct Injection {
    HMODULE dllHandle = nullptr;
    InjectionFunction injectedFunc = nullptr;

    int compile(const std::string& cpp_file, const std::string& dll){
        #ifdef _WIN32
            // Use the name of the file or some checksum to make sure
            // the DLL is not recompiled unnecessarily.
            // Use the linux server for testing to make sure it is compatible.
            // Potentially use the sed profiler.
            std::string cmd = "g++ -shared -o " + dll + " " + cpp_file;
            int result = std::system(cmd.c_str());
            return result;
        #else
            std::cerr << "Compilation is only supported on Windows." << std::endl;
            return -1;
        #endif
    }
    
    int load(const std::string& dll) {
        dllHandle = LoadLibraryA(dll.c_str());
        if (dllHandle == nullptr) {
            std::cerr << "Error loading DLL: " << dll << std::endl;
            std::cerr << "Error code: " << GetLastError() << std::endl;
            return -1;
        }

        injectedFunc = (InjectionFunction)GetProcAddress(dllHandle, "RATE");
        if (injectedFunc == nullptr) {
            std::cerr << "Error finding function in DLL: " << dll << std::endl;
            std::cerr << "Error code: " << GetLastError() << std::endl;
            FreeLibrary(dllHandle);
            dllHandle = nullptr;
            return -1;
        }
        return 0;
    }

    float exec() const {
        if (injectedFunc != nullptr) {
            return injectedFunc();
        } else {
            std::cerr << "Injection function is not loaded." << std::endl;
            return -99.0f;
        }
    }

    void unload() {
        if (dllHandle == nullptr) {
            std::cerr << "DLL is not loaded." << std::endl;
            return;
        } else {
            FreeLibrary(dllHandle);
            dllHandle = nullptr;
            injectedFunc = nullptr;
        }
    }

    ~Injection() {
        unload();
    }
};

#endif // UTILITIES_H
