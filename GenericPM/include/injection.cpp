#include "injection.h"

std::string Injection::parse() {
    std::regex fioPattern(R"(#\{([A-Za-z]+):([A-Za-z0-9_]+):?([A-Za-z0-9]+)?\})");
    std::string result = rawExpression;
    
    // Process all matches from right to left to avoid position shifts
    std::vector<std::smatch> allMatches;
    std::sregex_iterator iter(rawExpression.begin(), rawExpression.end(), fioPattern);
    std::sregex_iterator end;
    
    // Collect all matches
    for (; iter != end; ++iter) {
        allMatches.push_back(*iter);
    }
    
    // Process matches in reverse order to maintain string positions
    for (auto it = allMatches.rbegin(); it != allMatches.rend(); ++it) {
        const std::smatch& match = *it;
        float value;
        
        std::string group = match[1].str();
        std::string second = match[2].str();
        std::string third = match[3].str();
        
        if (third.empty()) {
            // Two parts: #{GROUP:VARNAME}
            value = FlexibleIO::getInstance()->getReal(group, second);
        } else {
            // Three parts: #{GROUP:YRDOY:VARNAME} or #{GROUP:VARNAME:INDEX}
            // Check if second part is a year-day (length 7 and all digits)
            if (second.length() == 7 && std::all_of(second.begin(), second.end(), ::isdigit)) {
                value = FlexibleIO::getInstance()->getRealYrdoy(group, second, third);
            } else if (std::all_of(second.begin(), second.end(), ::isdigit)) {
                value = FlexibleIO::getInstance()->getRealIndex(group, third, std::stoi(second));
            } else {
                throw std::runtime_error("Error: Second part of FIO reference '" + second + "' is neither YRDOY nor INDEX.");
            }
        }
        
        // Replace this specific match with its value
        result.replace(match.position(), match.length(), std::to_string(value));
    }
    
    return result;
}

double Injection::eval() {
    std::string parsedExpr = parse();
    int err;
    te_expr *n = te_compile(parsedExpr.c_str(), nullptr, 0, &err);
    if (n) {
        double result = te_eval(n);
        te_free(n);
        return result;
    } else {
        throw std::runtime_error("Error: Failed to parse expression '" + parsedExpr + "' at position " + std::to_string(err));
    }
}