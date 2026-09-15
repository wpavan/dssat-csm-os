/**
 * @file basicinterface.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * @author Virginia L. Covert (virginiacovert454@gmail.com)
 * 
 * @copyright Copyright (c) 2017–2026, University of Florida
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef BASICINTERFACE_H
#define BASICINTERFACE_H

class BasicInterface {
public:
    virtual ~BasicInterface() {
    }
    
    virtual void rate() = 0;
    virtual void integration() = 0;
    virtual void output() = 0;
};

#endif // BASICINTERFACE_H
