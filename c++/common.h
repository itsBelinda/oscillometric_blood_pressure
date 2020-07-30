/**
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
 * Limits for the configuraable variables in Processing and OBPDetection
 */
#define RATIO_MIN           0.3         //!< The minimal value for the SBP and DBP ratios.
#define RATIO_MAX           0.9         //!< The maximal value for the SBP and DBP ratios.
#define NBR_PEAKS_MIN       9           //!<
#define NBR_PEAKS_MAX       25          //!<
#define PUMP_UP_VALUE_MIN   120         //!<
#define PUMP_UP_VALUE_MAX   230         //!<

/**
 * Enum to describe the current state of the UI.
*/
enum class Screen {
    startScreen,
    inflateScreen,
    deflateScreen,
    emptyCuffScreen,
    resultScreen
};


#endif //OBP_COMMON_H
