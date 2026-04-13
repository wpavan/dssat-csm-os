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