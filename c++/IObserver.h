/**
 * @file        IObserver.h
 * @brief
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 * @details
 */
#ifndef OBP_IOBSERVER_H
#define OBP_IOBSERVER_H

#include "common.h"

class IObserver {
protected:
    IObserver() = default;
public:
    // Pure virtual functions would mean observer has to handle
    // every update. This way, they can ignore them if they do not
    // need them.
    virtual void eNewData(double pData, double oData) {};
    virtual void eSwitchScreen(Screen eScreen) {}; //TBD
    virtual void eResults(double map, double sbp, double dbp) {};
    virtual void eHeartRate(double heartRate) {};
    virtual void eReady() {};

};


#endif //OBP_IOBSERVER_H
