/**
 * @file project_config.h
 * 
 * @author Virginia Covert (virginiacovert454@gmail.com)
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2025–2026, University of Florida
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_H
#define CONFIG_H

// Removal method for spores in the clouds (F,P,O)
// 0: proportional removal based on k_g and k_d
// 1: compartmentalization and max density with rain effect
#define REMOVAL_METHOD 0

// These are the removal constants used in their exact form from in 
// Thiago's code. A portion (alpha) of the spores that are lost due to
// the gravitational constant are actually deposited on the plant. These
// spores remain in the system for infection. That way, when infection
// takes place, the removal will take care of the rest of the spores.
#define K_G 0.05f
#define K_D 0.1f

#define K_INF 0.0025f

#define LAG_SLOPE 1.0f
#define T_LAG 5.0f
#define R_MAX 0.3f
#define Y 0.4f
// Diagnostic flag for spores movement tracing. Set to 1 to enable verbose
// per-cloud diagnostic prints showing spores created, forwarded, queued,
// and removed. Disabled by default.
#ifndef DIAG_SPORES
#define DIAG_SPORES 0
#endif

// Enable ODE-based GDM simulation (0 = disabled, 1 = enabled)
#ifndef ENABLE_ODE_GDM
#define ENABLE_ODE_GDM 0
#endif

#endif // CONFIG_H