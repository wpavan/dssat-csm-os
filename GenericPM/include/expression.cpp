#include "../TinyExpr++/tinyexpr.h"

#include "expression.h"
#include "numericstringcache.h"
#include "utilities.h"

// Propagated includes:
// - #include "equation_context.h"

ParserCache* ParserCache::instance = nullptr;

// Initialize the static translation cache
std::unordered_map<std::string, std::string> Expression::translationCache;

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
            FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();

            // Encode the symbol to a unique double ID for tinyexpr parsing
            return converter->encode(std::string(symbol));
            }
        );
        // Attempt to compile the new parser
        cacheEntry.compile();

        // Return a reference to the compiled parser for this expression
        return &(cacheEntry.parser);
    }
}

const std::string& Expression::getTranslated() {
    if (!translated) {
        // Check if this expression has already been translated globally
        auto cacheIt = translationCache.find(originalExpr);
        if (cacheIt != translationCache.end()) {
            // Found in global cache, use it
            translatedExpr = cacheIt->second;
            translated = true;
            return translatedExpr;
        }

        // Not in cache, perform translation
        translatedExpr = originalExpr; // Start with the original expression
        FastStringDoubleConverter* converter = FastStringDoubleConverter::getInstance();

        // Process all matches from right to left to avoid position shifts
        std::vector<std::smatch> allMatches;
        std::sregex_iterator iter(translatedExpr.begin(), translatedExpr.end(), GDM::RegexPatterns::FIO_PATTERN);
        std::sregex_iterator end;

        // Collect all matches
        for (; iter != end; ++iter) {
            allMatches.push_back(*iter);
        }

        // Process matches in reverse order to maintain string positions
        for (auto it = allMatches.rbegin(); it != allMatches.rend(); ++it) {
            const std::smatch& match = *it;

            // The constructed string that replaces the flexibleIO reference.
            std::string fnCall;
            
            std::string group = match[1].str();
            std::string second = match[2].str();
            std::string third = match[3].str();
            
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
                if (second.length() == 7 && std::all_of(second.begin(), second.end(), ::isdigit)) {
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
                } else if (std::all_of(third.begin(), third.end(), ::isdigit)) {
                    // Encode group and varname (second) to unique double IDs for tinyexpr parsing
                    group = std::to_string(converter->encode(group));
                    second = std::to_string(converter->encode(second));

                    // Skip encoding third bc it is numeric index
                    // Create a string that represents the fio function call.
                    fnCall = "FIO_REAL_INDEX(" + group + "," + second + "," + third + ")";
                } else {
                    throw std::runtime_error("Error: Second part of FIO reference '" + second + "' is neither YRDOY nor is the third part of FIO reference: '" + third + "' an INDEX.");
                }
                // Show result.
                printf("Constructed function call for FIO fn: %s\n", fnCall.c_str());
            }

            // Replace this specific match with its value
            translatedExpr.replace(match.position(), match.length(), fnCall);
        }

        // Store in global cache for future use
        translationCache[originalExpr] = translatedExpr;
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
        std::string errorMsg = "Evaluation resulted in NaN for expression:\n\tOriginal:   " + originalExpr + "\n\tTranslated: " + this->getTranslated();
        throw std::runtime_error(errorMsg);
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