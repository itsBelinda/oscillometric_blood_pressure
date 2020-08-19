/**
 * @file        ComediHandler.cpp
 * @brief
 * @version     1.0.0
 * @author      Belinda Kneubühler belinda.kneubuehler@gmail.com
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 * @details
 * This file contains common configurations and general definitions.
 * This is the only place where any defines should be, so they are never duplicated.
 */
#ifndef OBP_COMMON_H
#define OBP_COMMON_H

// will produce
//#define NDEBUG 1 //TODO: uncomment to remove asserts
#include <plog/Log.h>
#include <cassert>

#define SAMPLING_RATE       1000        //!< 1 kHz expected sampling rate from ADC
#define MAX_DATA_LENGTH     8*SAMPLING_RATE
//!< Data length per plot for memory allocation
#define AMBIENT_AV_TIME     250         //!< The averaging time to detect ambient pressure in ms
#define AMBIENT_DEVIATION   0.001       //!< Allowed deviation for average value in V
#define DEFAULT_MINUTES     5           //!< Maximum allowed minutes for
#define DEFAULT_DATA_SIZE   SAMPLING_RATE*60*DEFAULT_MINUTES
//!< Maximum allowed data size
/**
 * Limits for the configurable variables in Processing and OBPDetection
 */
#define RATIO_MIN           0.4         //!< The minimal value for the SBP and DBP ratios.
#define RATIO_MAX           0.9         //!< The maximal value for the SBP and DBP ratios.
#define NBR_PEAKS_MIN       9           //!< The minimal number of peaks minimally used for detection.
#define NBR_PEAKS_MAX       25          //!< The maximal number of peaks minimally used for detection.
#define PUMP_UP_VALUE_MIN   120         //!< The minimal pump-up value that can be set.
#define PUMP_UP_VALUE_MAX   230         //!< The minimal pump-up value that can be set.

/**
 * Enum to describe the current state of the UI.
*/
enum class Screen
{
    startScreen,      //!< The start-up screen.
    inflateScreen,    //!< The screen that instructs the user to inflate the cuff.
    deflateScreen,    //!< The screen that instructs the user to deflate the cuff slowly.
    emptyCuffScreen,  //!< The screen that instructs the user to empty the cuff completely.
    resultScreen,     //!< The screen showing the results.
};


#endif //OBP_COMMON_H
