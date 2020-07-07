
#include "ISubject.h"




void ISubject::attach(IObserver * observer) {
    observerList.push_back(observer);
}



void ISubject::notifyNewData(double pData, double oData)


void ISubject::notifySwitchScreen(Screen eScreen) {
    std::for_each(observerList.begin(), observerList.end(),
                  [eScreen](IObserver * observer)
                  {
                      observer->eSwitchScreen(eScreen);
                  });
}