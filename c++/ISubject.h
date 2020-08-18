/**
 * @file        ISubject.h
 * @brief
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 * @details
 */

#ifndef OBP_ISUBJECT_H
#define OBP_ISUBJECT_H

#include <list>
#include <algorithm>

#include "common.h"
#include "IObserver.h"


class ISubject {

public:
    virtual void attach(IObserver *observer) {
        observerList.push_back(observer);
    }

    virtual void detach(IObserver *observer) {
        observerList.remove(observer);
    }

protected:
    ISubject() = default;
    virtual void notifyReady() {
        std::for_each(observerList.begin(), observerList.end(),
                      [](IObserver *observer) {
                          observer->eReady();
                      });
    }

    virtual void notifyNewData(double pData, double oData) {
        std::for_each(observerList.begin(), observerList.end(),
                      [pData, oData](IObserver *observer) {
                          observer->eNewData(pData, oData);
                      });
    }


    virtual void notifySwitchScreen(Screen eScreen) {
        std::for_each(observerList.begin(), observerList.end(),
                      [eScreen](IObserver *observer) {
                          observer->eSwitchScreen(eScreen);
                      });
    }

    virtual void notifyResults(double map, double sbp, double obp) {
        std::for_each(observerList.begin(), observerList.end(),
                      [map, sbp, obp](IObserver *observer) {
                          observer->eResults(map, sbp, obp);
                      });
    }

    virtual void notifyHeartRate(double heartRate) {
        std::for_each(observerList.begin(), observerList.end(),
                      [heartRate](IObserver *observer) {
                          observer->eHeartRate(heartRate);
                      });
    }
private:
    std::list<IObserver *> observerList;

};

#endif //OBP_ISUBJECT_H
