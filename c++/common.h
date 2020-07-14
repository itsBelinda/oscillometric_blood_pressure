/**
 * This file contains common configurations and general definitions.
 * This is the only place where any defines should be, so they are never dubicated.
 */
#ifndef OBP_COMMON_H
#define OBP_COMMON_H

#include <plog/Log.h>

#define SAMPLING_RATE       1000    //!< 1 kHz expected sampling rate from ADC
#define MAX_DATA_LENGTH     8000    //!< Data length per plot for memory allocation
#define AMBIENT_AV_TIME     1000    //!< The averaging time to detect ambient pressure
#define AMBIENT_DEVIATION   0.05     //!< Allowed deviation for average value

enum class Screen {
    startScreen,
    inflateScreen,
    deflateScreen,
    emptyCuffScreen,
    resultScreen
};


#endif //OBP_COMMON_H
