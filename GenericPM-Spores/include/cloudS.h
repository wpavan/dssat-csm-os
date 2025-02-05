#ifndef CLOUDS_H
#define CLOUDS_H

#include "basicS.h"
#include "basicinterfaceS.h"
#include "diseaseS.h"
#include<vector>

class CloudS : public BasicS, virtual public BasicInterfaceS {
protected:
    std::vector<float> values;
    DiseaseS *disease;
    int sporesCreated = 0;
    int sporesToBeRemoved = 0;
    
public:

    DiseaseS* getDisease() {
        return disease;
    }
    virtual void addSporesCreatedS(float sporesCreated) = 0;

    void rateS() {
    }
    void integrationS();

    void outputS() {
    }
    float getValueS();
    void removeSporesCloudS(float toBeRemove);
    void removeSporesCloudByRainS(float percent);
    void removeSporesCloudFByAgeS(void);
    void removeSporesCloudPByAgeS(void);
    void removeSporesCloudOByAgeS(void);


    //void removeSporesCloudFByAgeUvS(void);

    int getSporesToBeRemoved() {
        return sporesToBeRemoved;
    }

    void setSporesToBeRemoved(int sporesToBeRemoved) {
        this->sporesToBeRemoved = sporesToBeRemoved;
    }

    void addSporesToBeRemoved(int sporesToBeRemoved) {
        this->sporesToBeRemoved += sporesToBeRemoved;
    }
};

#endif // CLOUD_H
