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
    
    // Static cache for all translated expressions (keyed by original expression)
    static std::unordered_map<std::string, std::string> translationCache;
    
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
    virtual float evaluate();
    
    // Determination if expression is empty
    const bool empty();

    // For debugging
    const std::string& getOriginal() const { return originalExpr; }
};

class BoundExpression : public Expression {
private:
    std::vector<float> allowedOutputs; // Vector of allowed output values enforceed by evaluate().
    float bounds[2]; // Optional bounds (min, max) for the expression output. (-99.0f,-99.0f) implies no bounds.
    
    enum class BoundType {
        UNSET,          // Bounds not set yet. Evaluation results in error. 
        ALLOWED_VALUES, // Evaluation must be one of the values in allowedOutputs.
        MIN_MAX         // Evaluation must be >= bounds[0] and <= bounds[1].
    };

    BoundType boundType;
public:
    BoundExpression() : Expression(), allowedOutputs(), bounds{-99.0f, -99.0f}, boundType(BoundType::UNSET) {}
    explicit BoundExpression(const std::string& raw) : Expression(raw), allowedOutputs(), bounds{-99.0f, -99.0f}, boundType(BoundType::UNSET) {}
    explicit BoundExpression(const Expression& expr) : Expression(expr), allowedOutputs(), bounds{-99.0f, -99.0f}, boundType(BoundType::UNSET) {}
    explicit BoundExpression(const std::string& raw, const std::vector<float>& allowed) : Expression(raw), allowedOutputs(allowed), bounds{-99.0f, -99.0f}, boundType(BoundType::ALLOWED_VALUES) {}
    explicit BoundExpression(const Expression& expr, const std::vector<float>& allowed) : Expression(expr), allowedOutputs(allowed), bounds{-99.0f, -99.0f}, boundType(BoundType::ALLOWED_VALUES) {}
    explicit BoundExpression(const std::string& raw, float minBound, float maxBound) : Expression(raw), allowedOutputs(), bounds{minBound, maxBound}, boundType(BoundType::MIN_MAX) {}
    explicit BoundExpression(const Expression& expr, float minBound, float maxBound) : Expression(expr), allowedOutputs(), bounds{minBound, maxBound}, boundType(BoundType::MIN_MAX) {}

    void setRange(float bounds[2]);
    void setAllowedOutputs(std::vector<float>& allowed);

    float evaluate() override;
};

// NOTE: we should add a way to explicitly check for if the expression is empty.
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

        te_parser* getParser(const std::string& expression);
};

// newLesions = CLOUD_VALUE_BY_FAMILY(WB) * gaussian(#{PEST:ZSTAGE}, 65, 2.5, 1)

#endif // EXPRESSION_H