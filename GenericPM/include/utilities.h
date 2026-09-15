/**
 * @file utilities.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * @author Virginia Covert (virginiacovert454@gmail.com)
 * 
 * @copyright Copyright (c) 2017–2026, University of Florida
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef UTILITIES_H
#define UTILITIES_H

#include "errors.h"
#include "../../FlexibleIO/Data/FlexibleIO.hpp"
#include "../TinyExpr++/tinyexpr.h"

#include <string>
#include <regex>
#include <fstream>
#include <cstring>
#include <iostream>
#include <filesystem>

namespace GDM::RegexPatterns {
    inline const std::regex FIO_PATTERN(R"(#\{\s*([^:{}\r\n]+?)\s*:\s*([^:{}\r\n]+?)\s*(?::\s*([^{}\r\n]+?)\s*)?\})", std::regex::icase);
    inline const std::regex SIM_DATE_PATTERN(R"((?:(?:CURRENT|SIMULATION|SIM)(?: |_))?(?:YRDOY|DATE|TODAY|YYYYJJJ|YYYYDDD|YYYYDOY))", std::regex::icase);
    inline const std::regex VAR_PATTERN(R"(\$\w+(?::(?:TRNO\d+|DEFAULT))?)", std::regex::icase);
    inline const std::regex INVALID_SELECTOR_PATTERN(R"(\$\w+:(?!(?:TRNO\d+|DEFAULT)))", std::regex::icase); 
    inline const std::regex ADD_PATTERN(R"((?:ADDITION|ADD|PLUS|A|\+=|\+))", std::regex::icase);
    inline const std::regex SUBTRACT_PATTERN(R"((?:SUBTRACTION|SUBTRACT|MINUS|S|-=|-))", std::regex::icase);
    inline const std::regex MULTIPLY_PATTERN(R"((?:MULTIPLICATION|MULTIPLY|MULT|TIMES|M|\*=|\*))", std::regex::icase);
    inline const std::regex DIVIDE_PATTERN(R"((?:DIVISION|DIVIDE|DIV|D|\/=|\/))", std::regex::icase);
    inline const std::regex ASSIGN_PATTERN(R"((?:ASSIGNMENT|REPLACEMENT|ASSIGN|REPLACE|EQUALS|R|=))", std::regex::icase);
} // namespace GDM::RegexPatterns

enum class OrganMode {
    COHORT,
    SINGULAR
};

enum class Steps {
    RATE,
    INTEGRATION,
    OUTPUT
};

class StepParser {
    public:
        static int parse(std::string stepName) {
            if (stepName == "RATE") return (int)Steps::RATE;
            if (stepName == "INTEGRATION") return (int)Steps::INTEGRATION;
            if (stepName == "OUTPUT") return (int)Steps::OUTPUT;
            return -1;
        }
};

struct YearDoy {
    int year;
    int doy;

    YearDoy(int yearDoy) {
        year = yearDoy / 1000;
        doy = yearDoy % 1000;
    }

    operator int() const {
        return (year * 1000) + doy;
    }

    operator std::string() const {
        return std::to_string((year * 1000) + doy);
    }

    bool operator>(const YearDoy& other) const {
        return (year > other.year) || (year == other.year && doy > other.doy);
    }

    bool operator<(const YearDoy& other) const {
        return (year < other.year) || (year == other.year && doy < other.doy);
    }

    bool operator==(const YearDoy& other) const {
        return (year == other.year) && (doy == other.doy);
    }

    bool operator!=(const YearDoy& other) const {
        return !(*this == other);
    }

    bool operator>=(const YearDoy& other) const {
        return (*this > other) || (*this == other);
    }

    bool operator<=(const YearDoy& other) const {
        return (*this < other) || (*this == other);
    }

    bool operator>(const int& yearDoy) const {
        return *this > YearDoy(yearDoy);
    }

    bool operator<(const int& yearDoy) const {
        return *this < YearDoy(yearDoy);
    }

    bool operator==(const int& yearDoy) const {
        return *this == YearDoy(yearDoy);
    }

    bool operator!=(const int& yearDoy) const {
        return !(*this == YearDoy(yearDoy));
    }

    YearDoy addOneDay() {
        int newYear = year;
        int newDoy = doy + 1;

        int daysInYear = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 366 : 365;
        if (newDoy > daysInYear) {
            newDoy = 1;
            newYear += 1;
        }
        return YearDoy(newYear * 1000 + newDoy);
    }

    YearDoy& operator+=(int days) {
        for (int i = 0; i < days; i++) {
            *this = this->addOneDay();
        }
        return *this;
    }
};

class Utilities {
    private:
        static const std::string BASE52_CODING;
    public:
        static float growthFunction(float value, std::string expression_string);
        static float runExpressionFunction(float value, std::string expressionString);
        static float getHealthAreaProportion(float diseaseArea, float totalArea, float senescedArea);
        static std::string formatfloat(float value);
        static std::string formatfloat(float value, int decimals);
        static std::string base52Encode(size_t hashValue);

        // A method to safely rename files to a version that includes the suffix "_(X)" to avoid 
        // appending to files unintentionally.
        static std::filesystem::path safeRenameFile(const std::filesystem::path& oldPath);

        static void log_value(std::string varname, float value, int yrdoy, int decimals = 2) {
            std::ofstream logFile("value_log", std::ios::app);
            if (logFile.is_open()) {
                logFile << "Day: " << yrdoy 
                        << " Value: " << Utilities::formatfloat(value, decimals) 
                        << " Variable: " << varname << std::endl;
                logFile.close();
            } else {
                std::vector<std::string> messages;
                messages.push_back("Unable to open log file.");

                throwWarning(messages.size(), messages);
            }
        }

        static void log_str(std::string varname, std::string value, int yrdoy) {
            std::ofstream logFile("value_log", std::ios::app);
            if (logFile.is_open()) {
                logFile << "Day: " << yrdoy 
                        << " Value: " << value 
                        << " Variable: " << varname << std::endl;
                logFile.close();
            } else {
                std::vector<std::string> messages;
                messages.push_back("Unable to open log file.");

                throwWarning(messages.size(), messages);
            }
        }

        static float runExpression(std::string expression_string, float value) {
            te_parser parser;
            double x = value;
            parser.add_variable_or_function({"x", &x});

            bool compile_success = parser.compile(expression_string.c_str());
            
            if (!compile_success) {
                // Throw a could not compile error and fill warning.OUT with more information
                std::vector<std::string> messages;
                messages.push_back("Could not compile expression: ");
                messages.push_back(expression_string);
                messages.push_back("Error position: " + std::to_string(parser.get_last_error_position()));
                messages.push_back("Error message: " + parser.get_last_error_message());

                throwWarning(messages.size(), messages);

                // Access FIO for GDM file name + CSM TRTNUM
                FlexibleIO *fio = FlexibleIO::getInstance();
                std::string filePST = fio->getChar("PEST", "FILEPST");
                int TRTNUM = fio->getInteger("CONTROL", "TRTNUM");
                // NOTE: need to add some way to point the user to the proper line number
                throwError(25, filePST + " - " + std::to_string(TRTNUM), 0);
            }

            float result = parser.evaluate();

            return result;
        }
};

#endif // UTILITIES_H
