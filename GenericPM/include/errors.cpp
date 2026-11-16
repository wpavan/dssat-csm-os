/**
 * @file errors.cpp
 * 
 * @author Virginia Covert (virginiacovert454@gmail.com)
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2026, University of Florida
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <algorithm>
#include <cstring>

#include "errors.h"

void throwError(int errnum, std::string file, int line) {
    const char *ERRKEY = "GDM";
    const int ERRNUM = errnum;
    const char *ERRFILE = file.c_str();
    const int ERRLINE = line;

    errorGDM(ERRKEY, &ERRNUM, ERRFILE, &ERRLINE);
}

void throwWarning(int messageCount, std::vector<std::string> messages) {
    const char *ERRKEY = "GDM";
    // Pass the real number of lines (max 10, the size of the Fortran buffer);
    // previously fixed at 1, which dropped every line after the first.
    const int ICOUNT = std::max(1, std::min(messageCount, 10));

    char MESSAGE[10][78];
    memset(MESSAGE, ' ', sizeof(MESSAGE));

    for (int i = 0; i < ICOUNT && i < (int)messages.size(); ++i) {
        const char* text = messages[i].c_str();
        size_t len = std::min(strlen(text), (size_t)78);
        memcpy(MESSAGE[i], text, len);
    }

    warningGDM(&ICOUNT, ERRKEY, reinterpret_cast<char*>(MESSAGE));
}
