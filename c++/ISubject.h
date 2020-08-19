/**
 * @file        ISubject.h
 * @brief       The header file of the ISubject class.
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 * @details
 * Defines and implements the ISubject class and contains the general class description.
 */

#ifndef OBP_ISUBJECT_H
#define OBP_ISUBJECT_H

#include <list>
#include <algorithm>

#include "common.h"
#include "IObserver.h"

//! The ISubject Class provides the functionality to register observers and send events to an observing object.
/*!
 * ISubject is a simple interface defined in a header file that lets the implementing class notify its observers
 * about specific events. Each notification is realised as a protected notify-X method. The public methods are
 * 'attach' and 'detach'. Through these, a class that implements the IObserver can be attached to the subject. When
 * an observer is attached to the subject, a reference to the object is stored in a list. If one of the notify
 * methods is called, the notification is sent to all the observers in the list. An observer can be removed through
 * the detach method.
 *
 * The ISubject class can not be instantiated directly. A child class has to inherit from it. A protected constructor
 * ensures this.
 */
class ISubject {

public:
    /**
     * Add an observer to the observerList.
     * @param observer The observer to add.
     */
    virtual void attach(IObserver *observer) {
        observerList.push_back(observer);
    }

    /**
     * Remove an observer from the observerList.
     * @param observer The observer to remove.
     */
    virtual void detach(IObserver *observer) {
        observerList.remove(observer);
    }

protected:
    /**
     * Protected constructor, cannot be instantiated directly.
     */
    ISubject() = default;

    /**
     * Notify observers that the subject is ready to start a measurement.
     */
    virtual void notifyReady() {
        std::for_each(observerList.begin(), observerList.end(),
                      [](IObserver *observer) {
                          observer->eReady();
                      });
    }

    /**
     * Notify observers about a new data pair.
     * @param pData The new pressure data sample.
     * @param oData The new oscillation data sample.
     */
    virtual void notifyNewData(double pData, double oData) {
        std::for_each(observerList.begin(), observerList.end(),
                      [pData, oData](IObserver *observer) {
                          observer->eNewData(pData, oData);
                      });
    }

    /**
     * Notify observers about a change in the screen to display.
     * @param eScreen The new screen to display.
     */
    virtual void notifySwitchScreen(Screen eScreen) {
        std::for_each(observerList.begin(), observerList.end(),
                      [eScreen](IObserver *observer) {
                          observer->eSwitchScreen(eScreen);
                      });
    }

    /**
     * Notify observers about a newly available result. If any of the calculations failed, the correponding value
     * will be 0.0.
     * @param map The MAP value.
     * @param sbp The SBP value.
     * @param obp The OBP value.
     */
    virtual void notifyResults(double map, double sbp, double obp) {
        std::for_each(observerList.begin(), observerList.end(),
                      [map, sbp, obp](IObserver *observer) {
                          observer->eResults(map, sbp, obp);
                      });
    }

    /**
     * Notify observers about a new heartRate value.
     * @param heartRate The new value.
     */
    virtual void notifyHeartRate(double heartRate) {
        std::for_each(observerList.begin(), observerList.end(),
                      [heartRate](IObserver *observer) {
                          observer->eHeartRate(heartRate);
                      });
    }
private:
    std::list<IObserver *> observerList;  //!< The list of attached observers, used to iterate over when sending notifications.

};

#endif //OBP_ISUBJECT_H
