#ifndef BASICINTERFACES_H
#define BASICINTERFACES_H

class BasicInterfaceS {
public:

    virtual ~BasicInterfaceS() {
    }
    virtual void rateS() = 0;
    virtual void integrationS() = 0;
    virtual void outputS() = 0;

};

#endif // BASICINTERFACE_H
