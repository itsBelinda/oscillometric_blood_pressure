/**
 * This file contains common configurations and general definitions.
 * This is the only place where any defines should be, so they are never dubicated.
 */
#ifndef OBP_COMMON_H
#define OBP_COMMON_H

#include <plog/Log.h>

#define SAMPLING_RATE       1000 // 1kHz
#define MAX_DATA_LENGTH     8000     //!< Data length per plot for memory allocation

enum class Screen {
    startScreen,
    inflateScreen,
    releaseScreen,
    deflateScreen,
    resultScreen
};


#endif //OBP_COMMON_H
