/**
 * @file basicinterface.h
 * 
 * @author Willingthon Pavan (wpavan.us@gmail.com)
 * @author Jose Mauricio Cunha Fernandes (jmauricio.fernandes@icloud.com)
 * 
 * @copyright Copyright (c) 2017–2025, DSSAT Foundation
 * @license BSD-3-Clause. See the LICENSE file in the root folder for details.
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
