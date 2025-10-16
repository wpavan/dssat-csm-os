#include "../TinyExpr/tinyexpr.h"
#include "../../FlexibleIO/Data/FlexibleIO.hpp"

#include <string>
#include <regex>
#include <iostream>

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
 * Example inputs to be handled:
 * #{WTH:2024135:TMAX}
 * #{GROUP:SPORE_VALUE}
 * #{GROUP:VARNAME:INDEX}
 */

class Injection {
    protected:
        std::string rawExpression;

    public:
        Injection() : rawExpression("") {};
        Injection(std::string expr) : rawExpression(expr) {};
        
        std::string parse();
        double eval();
};