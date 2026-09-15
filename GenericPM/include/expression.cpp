/**
 * @file expression.cpp
 * 
 * @author Virginia L. Covet (virginiacovert454@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * 
 * @copyright Copyright (c) 2026, University of Florida
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <regex>
#include <algorithm>
#include <cctype>
#include <sstream>

#include "../TinyExpr++/tinyexpr.h"

#include "errors.h"
#include "expression.h"
#include "debug_control.h"
#include "numericstringcache.h"
#include "utilities.h"

// Propagated includes:
// - #include "equation_context.h"

ParserCache* ParserCache::instance = nullptr;

// Initialize the static translation cache
std::unordered_map<std::string, std::string> Expression::translationCache;

namespace {
    struct FioReference {
        size_t position;
        size_t length;
        std::string raw;
        std::vector<std::string> parts;
    };

    std::string trimWhitespace(const std::string& value) {
        auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
            return std::isspace(ch);
        });

        auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
            return std::isspace(ch);
        }).base();

        if (first >= last) {
            return "";
        }

        return std::string(first, last);
    }

    std::string escapeForLog(const std::string& value) {
        std::string escaped;
        for (char ch : value) {
            switch (ch) {
                case '\n':
                    escaped += "\\n";
                    break;
                case '\r':
                    escaped += "\\r";
                    break;
                case '\t':
                    escaped += "\\t";
                    break;
                default:
                    escaped += ch;
                    break;
            }
        }
        return escaped;
    }

    std::vector<std::string> splitFioParts(const std::string& content) {
        std::vector<std::string> parts;
        std::stringstream stream(content);
        std::string part;

        while (std::getline(stream, part, ':')) {
            parts.push_back(trimWhitespace(part));
        }

        return parts;
    }

    std::vector<FioReference> collectFioReferences(const std::string& expression) {
        std::vector<FioReference> references;
        size_t searchFrom = 0;

        while (true) {
            size_t start = expression.find("#{", searchFrom);
            if (start == std::string::npos) {
                break;
            }

            size_t end = expression.find('}', start + 2);
            if (end == std::string::npos) {
                throw std::runtime_error(
                    "Error: Unterminated FIO reference starting at '" +
                    escapeForLog(expression.substr(start)) + "'."
                );
            }

            std::string raw = expression.substr(start, end - start + 1);
            std::string content = expression.substr(start + 2, end - start - 2);
            references.push_back({start, raw.length(), raw, splitFioParts(content)});
            searchFrom = end + 1;
        }

        return references;
    }
}

const bool Expression::empty() {
    return originalExpr.empty();
}

te_parser* ParserCache::getParser(const std::string& expression) {
    // Find the parser based on the expression (using map)
    auto it = cache.find(expression);

    if (it != cache.end()) {
        // If found, return a reference to the existing parser
        return &(it->second.parser);
    } else {
        // If not found, create a new parser
        auto& cacheEntry = cache.emplace(expression, CachedParser(expression)).first->second;
        
        // Prepare the parser to handle functions with string arguments
        cacheEntry.parser.set_unknown_symbol_resolver(
            [](std::string_view symbol) -> te_type 
            {
            // Use the method of encoding *every* symbol and let the functions 
            // handle errors on their own
            // First, check if the symbol is referring to the current simulation date.
            //   This is a special case used in many functions and should be encoded 
            //   to a value of -1. Each function expecting a YRDOY can check for this
            //   special value and replace it dynamically.
            if (std::regex_match(symbol.begin(), symbol.end(), GDM::RegexPatterns::SIM_DATE_PATTERN)) {
                return -1.0;  // Special value for current simulation date
            } else {
                // For all other symbols, encode them to unique double IDs for tinyexpr parsing
                FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();

                // Encode the symbol to a unique double ID for tinyexpr parsing
                return converter->encode(std::string(symbol));
            }
            }
        );
        // Attempt to compile the new parser
        cacheEntry.compile();

        // Return a reference to the compiled parser for this expression
        return &(cacheEntry.parser);
    }
}

// Helper method to create context signature for caching
std::string Expression::createContextSignature() const {
    std::vector<std::string> pairs;
    for (const auto& pair : instanceContext) {
        pairs.push_back(pair.first + ":" + pair.second);
    }
    std::sort(pairs.begin(), pairs.end());
    
    std::string signature;
    for (const auto& pair : pairs) {
        signature += pair + ";";
    }
    return std::to_string(std::hash<std::string>{}(signature));
}

const std::string& Expression::getTranslated() {
    if (!translated) {
        std::string contextSignature = createContextSignature();
        std::string cacheKey = originalExpr + "|CTX:" + contextSignature;

        // Check if this expression has already been translated globally
        auto cacheIt = translationCache.find(cacheKey);
        if (cacheIt != translationCache.end()) {
            // Found in global cache, use it
            translatedExpr = cacheIt->second;
            translated = true;
            return translatedExpr;
        }
        
        // Not in cache, perform translation
        translatedExpr = originalExpr; // Start with the original expression
       
        std::regex varPattern(R"(\$([A-Za-z_][A-Za-z0-9_]*))");
        std::smatch match;
        std::string working = translatedExpr;
        
        while (std::regex_search(working, match, varPattern)) {
            std::string varName = match[1].str();

            
            // Look up this variable in the context
            auto varIt = instanceContext.find(varName);
            if (varIt != instanceContext.end()) {
                // Found the variable, substitute it (wrapped in parentheses for safety)
                std::string replacement = "(" + varIt->second + ")";

                working.replace(match.position(0), match.length(0), replacement);

                // Continue searching from the end of the replacement
            } else {
                // Variable not found - throw an error
                throw std::runtime_error("Error: Variable reference '$" + varName + "' not found in expression context.");
            }
        }
        translatedExpr = working;
        
        // ===== STEP 2: FIO Reference Translation =====
        FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();
        // Process all references from right to left to avoid position shifts
        std::vector<FioReference> allMatches = collectFioReferences(translatedExpr);

        // Process matches in reverse order to maintain string positions
        int matchNum = 0;
        for (auto it = allMatches.rbegin(); it != allMatches.rend(); ++it) {
            const FioReference& match = *it;
            matchNum++;

            // The constructed string that replaces the flexibleIO reference.
            std::string fnCall;

            if (match.parts.size() < 2 || match.parts.size() > 3) {
                throw std::runtime_error(
                    "Error: Invalid FIO reference '" + escapeForLog(match.raw) +
                    "' in expression '" + escapeForLog(originalExpr) +
                    "'. Expected #{GROUP:VARNAME}, #{GROUP:YRDOY:VARNAME}, or #{GROUP:VARNAME:INDEX}."
                );
            }

            std::string group = match.parts[0];
            std::string second = match.parts[1];
            std::string third = match.parts.size() == 3 ? match.parts[2] : "";
            
            // Instead of directly returning the values from flexibleIO, we will construct a string which holds function calls that are parseable by tinyexpr.
            if (third.empty()) {
                // Two parts: #{GROUP:VARNAME}
                // Encode group and varname into unique double IDs
                group = std::to_string(converter->encode(group));
                second = std::to_string(converter->encode(second));

                fnCall = "FIO_REAL(" + group + "," + second + ")";
            } else {
                // Three parts: #{GROUP:YRDOY:VARNAME} or #{GROUP:VARNAME:INDEX}
                // Check if second part is a year-day (length 7 and all digits)
                // Then check for specific keywords to indicate current simulation date.
                // Then check if it's an index (all digits).
                if (second.length() == 7 && std::all_of(second.begin(), second.end(), [](unsigned char ch) { return std::isdigit(ch); })) {
                    // Encode group and varname (third) to unique double IDs for tinyexpr parsing
                    group = std::to_string(converter->encode(group));
                    third = std::to_string(converter->encode(third));

                    // Do not encode second since it's already a string of digits representing a YRDOY, which is what the function expects.
                    // Create a string that represents the fio function call.
                    fnCall = "FIO_REAL_YRDOY(" + group + "," + second + "," + third + ")";
                } else if (std::regex_match(second, GDM::RegexPatterns::SIM_DATE_PATTERN)) {
                    // Encode group and varname (third) to unique double IDs for tinyexpr parsing
                    group = std::to_string(converter->encode(group));
                    third = std::to_string(converter->encode(third));
                    
                    // Do not encode second since it will be replaced by a string of digits representing a YRDOY, which is what the function expects.
                    // Create a string that represents the fio function call.
                    fnCall = "FIO_REAL_YRDOY(" + group + ",-1," + third + ")";
                } else if (std::all_of(third.begin(), third.end(), [](unsigned char ch) { return std::isdigit(ch); })) {
                    // Encode group and varname (second) to unique double IDs for tinyexpr parsing
                    group = std::to_string(converter->encode(group));
                    second = std::to_string(converter->encode(second));

                    // Skip encoding third bc it is numeric index
                    // Create a string that represents the fio function call.
                    fnCall = "FIO_REAL_INDEX(" + group + "," + second + "," + third + ")";
                } else {
                    throw std::runtime_error(
                        "Error: Invalid FIO reference '" + escapeForLog(match.raw) +
                        "' in expression '" + escapeForLog(originalExpr) +
                        "' after variable substitution '" + escapeForLog(translatedExpr) +
                        "'. The second part '" + second +
                        "' is not a YRDOY/SIM_DATE, and the third part '" +
                        third + "' is not an INDEX."
                    );
                }
            }
            // Replace this specific match with its value
            translatedExpr.replace(match.position, match.length, fnCall);
        }

        // Store in cache with context aware signature
        translationCache[cacheKey] = translatedExpr;
        translated = true;
    }
    return translatedExpr;
}

float Expression::evaluate() {
    // Get the parser referring to the expression supplied *after translation + compilation* 
    // Translation performed if not already done,
    // Compilation performed if not already done for this expression.
    // Evaluate and return value
    te_type result = ParserCache::getInstance()->getParser(this->getTranslated())->evaluate();
    if (std::isnan(result)) {
        const char *ERRKEY = "GDM";
        const int ERRNUM = 21;
        const char *FILE = "";
        const int LINE = 0;

        errorGDM(ERRKEY, &ERRNUM, FILE, &LINE);
        std::string errorMsg = "Evaluation resulted in NaN for expression:\n\tOriginal:   " + originalExpr + "\n\tTranslated: " + this->getTranslated() + "\n\tError Message: '" + ParserCache::getInstance()->getParser(this->getTranslated())->get_last_error_message() + "'";
        throw std::runtime_error(errorMsg);
    }
    return result;
}

float BoundExpression::evaluate() {
    float result = Expression::evaluate();
    switch(this->boundType) {
        case BoundType::UNSET:
            throw std::runtime_error("Bounds not set for this expression. Evaluation is not valid.");
        case BoundType::ALLOWED_VALUES:
            if (std::find(allowedOutputs.begin(), allowedOutputs.end(), result) == allowedOutputs.end()) {
                throw std::runtime_error("Evaluation result " + std::to_string(result) + " is not in the list of allowed output values.");
            }
        case BoundType::MIN_MAX:
            if (result < bounds[0] || result > bounds[1]) {
                throw std::runtime_error("Evaluation result " + std::to_string(result) + " is out of bounds. Allowed range: [" + std::to_string(bounds[0]) + ", " + std::to_string(bounds[1]) + "]");
            }
    }
    return result;
}

/*
// Compile expression from string and with provided context (variables & functions)
    bool compile_success = parser.compile(expression_string.c_str());

    // Detect errors and report
    if (!compile_success) {
        std::cout << "-----" << std::endl << "Expression: " << expression_string << std::endl 
        << "Error position: " << parser.get_last_error_position() << std::endl
        << "Error msg: " << parser.get_last_error_message() << std::endl 
        << "-----" << std::endl;
    }

*/
