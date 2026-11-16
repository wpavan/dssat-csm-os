/**
 * @file equation_context.cpp
 * 
 * @author Virginia L. Covet (virginiacovert454@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * 
 * @copyright Copyright (c) 2026, University of Florida
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "equation_context.h"

EquationContext *gEqContext = new EquationContext();

CustomFunctions& getCustomFunctions(){
    static CustomFunctions gCustomFunctions;
    return gCustomFunctions;
}
