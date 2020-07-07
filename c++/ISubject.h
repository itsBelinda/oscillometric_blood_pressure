#ifndef OBP_ISUBJECT_H
#define OBP_ISUBJECT_H

#include <list>
#include <algorithm>

#include "common.h"
#include "IObserver.h"


class ISubject {
public:
    // Pure virtual functions would mean observer has to implement
    // every method.
    virtual void attach(IObserver *observer) {
        observerList.push_back(observer);
    }

    virtual void detach(IObserver *observer) {
        observerList.remove(observer);
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

private:
    std::list<IObserver *> observerList;

};

#endif //OBP_ISUBJECT_H
