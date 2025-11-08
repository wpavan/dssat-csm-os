#include "../TinyExpr/tinyexpr.h"
#include "../../FlexibleIO/Data/FlexibleIO.hpp"

#include <string>
#include <regex>
#include <iostream>

#ifndef INJECTION_H
#define INJECTION_H

/* Define a flexible flexibleIO getter for all flexibleIO variables
 * 
 * The user types something like this in the YAML file:
 * RATE:
 *     DEST_VALUE: "#{WTH:YYYYJJJ:TMAX}*15"
 * 
 * Then we need to parse to make it interpretable by tinyexpr.
 * NOTE: do we need to clean the XXXX= bit and link it to a destination
 * variable on our own? We could also potentially setup a hierarchy of 
 * information in the yaml file that more clearly defines this.
 * 
 * Anything #{inside here} should be converted to the appropriate 
 * flexibleIO call to get the value. Then this should be bound to a 
 * variable inside the tinyexpr context. 
 * 
 * string replace_fio_call(string) {
 *     remove # and {} ;
 *     split by : ;
 *     first part is GROUP;
 * 
 *     if 2 parts -> getReal | getInteger
 *     (second part is VARNAME);
 * 
 *     if 3 parts -> {
 *         if middle part is YYYYJJJ -> getRealYrdoy | getIntegerYrdoy
 *         (second part is YRDOY, third part is VARNAME);
 * 
 *         else -> getRealIndex | getIntegerIndex
 *         (second part is VARNAME, third part is INDEX);
 *     }
 * 
 *     int result = FlexibleIO::getInstance()->getAppropriateType(args);
 *     return string_of(func_call)
 * }
 * 
 * Reserved words for current simulation date:
 * - CURRENT_YRDOY
 * - CURRENT_DATE
 * - SIMULATION_DATE
 * - SIM_DATE
 * - TODAY
 * - YRDOY
 * - YYYYJJJ 
 * 
 * The form of these reserved words should be: {FIRSTWORD{'_' | ' '}SECONDWORD}
 * These should be replaced with the current simulation date.
 * 
 * ## Example inputs to be handled:
 * - `#{WTH:2024135:TMAX}`
 * - `#{WTH:CURRENT_YRDOY:TMAX}`
 * - `#{PEST:SPORE_VALUE}`
 * - `#{GROUP:VARNAME:INDEX}`
 * 
 * # Reading & Processing
 * 
 * Because the user could input a set of size n of these expressions, 
 * we need to ensure that each one can be run dynamically and be used 
 * to replace or modify a value. 
 * 
 * Each entry (taking the form: `DEST_VAR: "EXPRESSION"`) should be 
 * parsed into an injection object that not only holds the raw 
 * expression, but also a pointer to the destination variable. 
 * 
 * An `update` function could be implemented in the injection class 
 * that results in the value being updated appropriately. 
 */

// Define the modification types for injections.
enum class ModificationType {
    ADD,        // Effect will be the same as +=
    SUBTRACT,   // Effect will be the same as -=
    MULTIPLY,   // Effect will be the same as *=
    DIVIDE,     // Effect will be the same as /=
    ASSIGN      // Effect will be the same as =
};

// Define regex patterns for each modification type
struct ModificationRegex { 
    static const std::regex addPattern;
    static const std::regex subPattern;
    static const std::regex multPattern;
    static const std::regex divPattern;
    static const std::regex asgnPattern;
};

enum class InjEndpoint {
    INOCULUM_GEN,
    INFECTION_BIOLOGICAL_FACTOR,
    OUTPUT
};

// Function declarations
ModificationType parseModification(std::string modifStr);
InjEndpoint parseEndpoint(std::string endpointStr);

/* Define a flexible flexibleIO getter for all flexibleIO variables
 * [... your comment block ...]
 */
class Injection {
    private:
        static const std::regex fioPattern_;
        static const std::regex simDatePattern_;
    protected:
        std::string rawExpression;
        std::string rawEndpoint;
        InjEndpoint endpoint;
        ModificationType modification;
    public:
        Injection() : rawExpression("") {};
        Injection(std::string endpt, std::string expr, std::string modif) : rawExpression(expr),  rawEndpoint(endpt){
            endpoint = parseEndpoint(endpt);
            modification = parseModification(modif);
        };

        InjEndpoint getEndpoint() const {
            return endpoint;
        }
        std::string parse(bool& missingVal);
        double eval();

        void apply(float& endpointValue);
        void apply(std::string& endpointVarName);
        void apply(float& endpointValue, std::string& endpointVarName);
};

/* NOTE: Create a bunch of handlers that will read in this information 
 * from the yaml file in the first place. This will allow us to create 
 * a unified modify function that points to the proper operation to 
 * conduct.
 */

#endif // INJECTION_H