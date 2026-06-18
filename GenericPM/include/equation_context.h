/**
 * @file equation_context.h
 * @brief Global equation evaluation context for TinyExpr++ expressions
 * 
 * @copyright Copyright (c) 2026, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 */

#ifndef EQUATION_CONTEXT_H
#define EQUATION_CONTEXT_H

#include "../TinyExpr++/tinyexpr.h"
#include <memory>

// Forward declarations to avoid circular dependencies
class Organ;
class LesionCohort;
class Plant;
class Cloud;
class Disease;

/**
 * @brief Context object passed to equation evaluators
 * 
 * Holds pointers to the current Organ and LesionCohort during
 * expression evaluation. Allows custom functions to access state
 * without additional parameters.
 */
struct EquationContext {
    Plant* plant;
    Organ* organ;
    LesionCohort* lesionCohort;
    std::shared_ptr<Cloud> cloud; 
    std::shared_ptr<Disease> disease;
    
    EquationContext() : plant(nullptr), organ(nullptr), lesionCohort(nullptr), cloud(nullptr), disease(nullptr) {}
};

/**
 * @brief Global equation context
 * 
 * Set and managed by Organ::rate() and LesionCohort::rate().
 * Accessed by equation evaluation functions via custom TinyExpr++ callbacks.
 * 
 * THREAD-SAFETY NOTE: Not thread-safe. If parallelizing, use thread-local storage.
 */
extern EquationContext* gEqContext;

struct CustomFunctions {
    std::set<te_variable> functionSet;

    void register_context_function(te_variable func){
        functionSet.insert(func);
    }

    CustomFunctions() = default;
};

// Declare dereferenced function set
CustomFunctions& getCustomFunctions();

#endif // EQUATION_CONTEXT_H
