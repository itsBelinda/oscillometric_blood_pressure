/**
 * @file        IObserver.h
 * @brief       The header file of the IObserver class.
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 * @details
 * Defines and implements the ISubject class and contains the general class description.
 */
#ifndef OBP_IOBSERVER_H
#define OBP_IOBSERVER_H

#include "common.h"

//! The IObserver Class provides the functionality to receive events from an observable object.
/*!
 * The IObserver class defines the functions that the observable class (the subject) uses to notify the observer. All
 * methods are virtual but implemented as empty functions. This way, the observing class can choose to implement, and
 * therefore listen to, the notifications that it wants to and ignore the ones that it does not. Just like the
 * ISubject class, the IObserver class can not be instantiated directly, but a child class has to inherit from it.
 */
class IObserver {
protected:
    /**
     * Protected constructor, cannot be instantiated directly.
     */
    IObserver() = default;
public:

    /**
     * The virtual function to handle new data events.
     * @param pData The new pressure data.
     * @param oData The new oscillation data.
     */
    virtual void eNewData(double pData, double oData) {};

    /**
     * The virtual function to handle screen events.
     * @param eScreen The new screen.
     */
    virtual void eSwitchScreen(Screen eScreen) {};

    /**
     * The virtual function to handle result events.
     * @param map The MAP value.
     * @param sbp The SBP value.
     * @param obp The OBP value.
     */
    virtual void eResults(double map, double sbp, double dbp) {};

    /**
     * The virtual function to handle heart rate events.
     * @param heartRate The new heart rate value.
     */
    virtual void eHeartRate(double heartRate) {};

    /**
     * The virtual function to handle ready events.
     */
    virtual void eReady() {};

};


#endif //OBP_IOBSERVER_H
