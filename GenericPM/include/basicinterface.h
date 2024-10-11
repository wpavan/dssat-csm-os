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
