/**
 * @file numericstringcache.h
 * 
 * @author Virginia Covert (virginiacovert454@gmail.com)
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2025–2026, University of Florida
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NUMERIC_STRING_CACHE_H
#define NUMERIC_STRING_CACHE_H

#include <unordered_map>
#include <vector>
#include <string>

class FastStringDoubleConverter {
private:
    FastStringDoubleConverter() {};
    static FastStringDoubleConverter* instance;
    static inline std::unordered_map<std::string, double> stringToDouble;
    static inline std::unordered_map<double, std::string> doubleToString;
    static inline double nextId = 1.0;
    
public:
    static FastStringDoubleConverter* getInstance();
    static FastStringDoubleConverter* newInstance();
    
    // Convert string to unique double ID (creates if doesn't exist)
    static double encode(const std::string& str) {
        auto it = stringToDouble.find(str);
        if (it != stringToDouble.end()) {
            return it->second;
        }
        
        double id = nextId++;
        stringToDouble[str] = id;
        doubleToString[id] = str;
        return id;
    }
    
    // Convert double ID back to string
    static const std::string& decode(double id) {
        return doubleToString[id];
    }
    
    // Pre-populate with known strings for maximum speed
    static void preload(const std::vector<std::string>& strings) {
        for (const auto& str : strings) {
            encode(str);
        }
    }
    
    // Clear all mappings
    static void clear() {
        stringToDouble.clear();
        doubleToString.clear();
        nextId = 1.0;
    }
};
#endif