#ifndef OBP_IOBSERVER_H
#define OBP_IOBSERVER_H

#include "common.h"

class IObserver {
public:
    // Pure virtual functions would mean observer has to handle
    // every update. This way, they can ignore them if they do not
    // need them.
    virtual void eNewData(double pData, double oData) {};
    virtual void eSwitchScreen(Screen eScreen) {}; //TBD
    virtual void eResults(double map, double sbp, double dbp) {};

};


#endif //OBP_IOBSERVER_H
