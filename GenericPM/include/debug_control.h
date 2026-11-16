/**
 * @file debug_control.h
 * 
 * @author Virginia L. Covet (virginiacovert454@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * 
 * Centralized debug control for GenericPM disease simulation.
 * Set GENERICPM_DEBUG_ENABLED to 1 to enable all debug output.
 * Set GENERICPM_WARNINGS_ENABLED to 1 to enable runtime warnings.
 * Set GENERICPM_OUT_ENABLED to 1 to write the standard GDM_*.OUT files.
 * GENERICPM_OUT_INTERVAL_DAYS controls how often rows are written.
 * Set GENERICPM_INTERNAL_OUTPUT_ENABLED to 1 to write legacy Cpp_*.txt files.
 * 
 * Debug tags:
 *   [EXPR]   - Expression translation and evaluation
 *   [SIM]    - Simulator rate calculations
 *   [CLOUDF] - Field cloud operations
 *   [CLOUD]  - Generic cloud integration
 *   [MGR]    - Manager initialization
 *   [DIAG]   - Diagnostic traces (YEARDOY, removals, etc.)
 * 
 * @copyright Copyright (c) 2026, University of Florida
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEBUG_CONTROL_H
#define DEBUG_CONTROL_H

#define GENERICPM_DEBUG_ENABLED 0
#define GENERICPM_WARNINGS_ENABLED 0
#define GENERICPM_OUT_ENABLED 0
#define GENERICPM_OUT_INTERVAL_DAYS 1
#define GENERICPM_INTERNAL_OUTPUT_ENABLED 0

#if GENERICPM_OUT_INTERVAL_DAYS < 1
#error "GENERICPM_OUT_INTERVAL_DAYS must be at least 1"
#endif

#endif // DEBUG_CONTROL_H
