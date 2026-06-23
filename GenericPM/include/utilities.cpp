/**
 * @file utilities.cpp
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "utilities.h"
#include "disease.h"

#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

const std::string Utilities::BASE52_CODING = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

float Utilities::growthFunction(float value, std::string expressionString) {
    return Utilities::runExpression(expressionString, value);
}

float Utilities::runExpressionFunction(float value, std::string expressionString) {
    return Utilities::runExpression(expressionString, value);
}

float Utilities::getHealthAreaProportion(float diseaseArea, float totalArea, float senescedArea) {
    float hap = 1 - ((diseaseArea+senescedArea) / totalArea);
    return (hap > 0 ? hap : 0);
}

std::string Utilities::formatfloat(float value) {
    return formatfloat(value, 2);
}

std::string Utilities::formatfloat(float value, int decimals) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(decimals) << value;
    return ss.str();
}

std::string Utilities::base52Encode(size_t hashValue) {
    if (hashValue == 0) {
        return "0";
    }

    std::string encodedResult;
    while (hashValue > 0) {
        encodedResult = Utilities::BASE52_CODING[hashValue % 52] + encodedResult;
        hashValue /= 52;
    }
    return encodedResult;
}

 std::filesystem::path Utilities::safeRenameFile(const std::filesystem::path& oldPath) {
    // Assume going into the function, there is already the base file existing. Now we need 
    // to return the new path with the appropriate suffix.

    std::filesystem::path newPath;
    int counter = 1;
    while (true) {
        newPath = oldPath.stem().string() + "_(" + std::to_string(counter) + ")" + oldPath.extension().string();
        if (!std::filesystem::exists(newPath)) {
            return newPath;
        }
        counter++;
    }
}