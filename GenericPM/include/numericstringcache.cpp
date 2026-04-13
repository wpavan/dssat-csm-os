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
