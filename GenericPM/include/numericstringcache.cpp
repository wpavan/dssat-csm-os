/**
 * @file numericstringcache.cpp
 * 
 * @author Virginia Covert (virginiacovert454@gmail.com)
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2025–2026, University of Florida
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 * SPDX-License-Identifier: BSD-3-Clause
 */

 #include "numericstringcache.h"
// Static member definitions
FastStringDoubleConverter* FastStringDoubleConverter::instance = nullptr;

FastStringDoubleConverter* FastStringDoubleConverter::getInstance() {
    if (instance == nullptr) {
        instance = new FastStringDoubleConverter();
    }
    return instance;
}

FastStringDoubleConverter* FastStringDoubleConverter::newInstance() {
    instance = nullptr;
    clear();
    return getInstance();
}
