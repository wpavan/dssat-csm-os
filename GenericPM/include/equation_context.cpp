/**
 * @file equation_context.cpp
 * 
 * @author Virginia L. Covet (virginiacovert454@gmail.com)
 * 
 * @copyright Copyright (c) 2026, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#include "equation_context.h"

EquationContext *gEqContext = new EquationContext();

CustomFunctions& getCustomFunctions(){
    static CustomFunctions gCustomFunctions;
    return gCustomFunctions;
}
