#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "equation_context.h"

#include <string>
#include <set>
#include <regex>

class Expression {
private:
    std::string originalExpr;            // #{GROUP:VAR} form
    mutable std::string translatedExpr;  // FIO_REAL(...) form
    mutable bool translated = false;     // Flag to indicate if translation occurred
    
public:
    // Default constructor
    Expression() : originalExpr("") {}

    // Constructor
    explicit Expression(const std::string& raw) : originalExpr(raw) {}

    // Equivalence operator
    bool operator==(const Expression& other) const {
        return originalExpr == other.originalExpr;
    }

    // Inequality operator
    bool operator!=(const Expression& other) const {
        return !(*this == other);
    }

    // Modify the original expression to replace FIO references with function calls
    const std::string& getTranslated();
    
    // Evaluate with globally provided context
    float evaluate();
    
    // Determination if expression is empty
    const bool empty();

    // For debugging
    const std::string& getOriginal() const { return originalExpr; }
};

class ParserCache {
    private:
        ParserCache() {};
        struct CachedParser {
            // Expression post fio translation 
            std::string expression;

            // TinyExpr++ parser object
            te_parser parser;

            // Compilation status
            bool compiled;

            CachedParser(std::string expr) : expression(expr), compiled(false) {};

            void compile() {
                if (!compiled) {
                    parser.set_variables_and_functions(getCustomFunctions().functionSet);
                    compiled = parser.compile(expression.c_str());  
                }
                if (!parser.success()) {
                    int error_pos = parser.get_last_error_position();
                    printf("-----\nExpression: %s\n", expression.c_str());

                    // Create visual indicator line with spaces and marker at error position
                    std::string indicator(expression.length(), ' ');
                    if (error_pos >= 0 && error_pos < expression.length()) {
                        indicator[error_pos] = '^';  // Use caret to point to error position
                    }
                    printf("            %s\n", indicator.c_str());  // "Expression: " is 12 chars, so pad with 12 spaces

                    printf("Error position: %d\nError msg: %s\n-----\n",
                        error_pos,
                        parser.get_last_error_message().c_str());

                    int length = expression.length();
                    std::string what = "Parser compilation failed for expression: " + expression;
                    throw std::runtime_error(what);
                }
            }
        };

        std::unordered_map<std::string, CachedParser> cache;
        static ParserCache* instance;
    public:
        static ParserCache* newInstance() {
            instance = nullptr;
            return getInstance();
        }

        static ParserCache* getInstance() {
            if (instance == nullptr) {
                instance = new ParserCache();
            }
            return instance;
        }

        te_parser* getParser(const std::string& expression) {
            // Find the parser based on the expression (using map)
            auto it = cache.find(expression);

            if (it != cache.end()) {
                // If found, return a reference to the existing parser
                return &(it->second.parser);
            } else {
                // If not found, create a new parser
                auto& cacheEntry = cache.emplace(expression, CachedParser(expression)).first->second;
                
                // Attempt to compile the new parser
                cacheEntry.compile();

                // Return a reference to the compiled parser for this expression
                return &(cacheEntry.parser);
            }
        }
};

#endif // EXPRESSION_H