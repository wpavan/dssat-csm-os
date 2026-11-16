/**
 * @file errors.h
 * 
 * @author Virginia Covert (virginiacovert454@gmail.com)
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2026, University of Florida
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ERRORS_H
#define ERRORS_H

#include <string>
#include <vector>

extern "C" {
    void errorGDM(const char *ERRKEY, const int *ERRNUM, const char *ERRFILE, const int *ERRLINE);
    void warningGDM(const int *ICOUNT, const char *ERRKEY, const char *MESSAGE);
    void infoGDM(const int *ICOUNT, const char *ERRKEY, const char *MESSAGE);
}

void throwError(int errnum, std::string file, int line);
void throwWarning(int messageCount, std::vector<std::string> messages);
void throwInfo(int messageCount, std::vector<std::string> messages);

#endif // ERRORS_H