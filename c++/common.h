/**
 * This file contains common configurations and general definitions.
 * This is the only place where any defines should be, so they are never dubicated.
 */
#ifndef OBP_COMMON_H
#define OBP_COMMON_H

// will produce
//#define NDEBUG 1
#include <plog/Log.h>
#include <cassert>

#define SAMPLING_RATE       1000    //!< 1 kHz expected sampling rate from ADC
#define MAX_DATA_LENGTH     8000    //!< Data length per plot for memory allocation
#define AMBIENT_AV_TIME     1000    //!< The averaging time to detect ambient pressure
#define AMBIENT_DEVIATION   0.05     //!< Allowed deviation for average value
#define MAX_HEART_RATE      180     //!< The maximal assumes heart rate in Hz
#define MIN_HEART_TIME      (60000/MAX_HEART_RATE)


enum class Screen {
    startScreen,
    inflateScreen,
    deflateScreen,
    emptyCuffScreen,
    resultScreen
};


#endif //OBP_COMMON_H
