/**
 * @file        ComediHandler.h
 * @brief       The header file of the ComdeiHandler class.
 * @author      Belinda Kneubühler belinda.kneubuehler@gmail.com
 * @date        2020-08-18
 * @author      Bernd Porr mail@berndporr.me.uk
 * @date        2005-2017
 * @author      Matthias H. Hennig hennig@cn.stir.ac.uk
 * @date        2003
 * @copyright   GNU General Public License v2.0
 *
 * @details
 * This class is based on a previous implementation by Bernd Porr and Matthias H. Henning.
 */
#ifndef OBP_COMEDIHANDLER_H
#define OBP_COMEDIHANDLER_H

#include <comedilib.h>

#define COMEDI_SUB_DEVICE   0   //!< using sub device 0
#define COMEDI_RANGE_ID     0   //!<  +/- 1.325V  for sigma device*/
#define COMEDI_NUM_CHANNEL  1   //!<  only one channel is used */
#define COMEDI_DEV_PATH     "/dev/comedi0" //!<  the path to access the comedi device


//! The ComediHandler class abstracts access to the hardware.
/*!
 * The class uses the comedi library (comedilib) to read data from the hardware device. The ComediHandler is
 * initialised when the Processing object is created. If the initialisation fails, e.g. because there is no device
 * connected, the application terminates, and an error message is printed. Upon a successful start-up, single samples
 * can be read from the device either as a raw integer value or as a voltage value. Optionally, the entire buffer
 * content can be read as raw values. The application is only reading voltage values.
 */
class ComediHandler
{
public:
    ComediHandler();

    double getSamplingRate();
    int getBufferContents();
    int getRawSample();
    double getVoltageSample();

private:

    comedi_cmd comediCommand;
    comedi_t *dev;
    size_t readSize;
    bool sigmaBoard;
    lsampl_t maxdata;
    comedi_range *crange;
    double sampling_rate;

    int numChannels;
    const int adChannel;
    unsigned *chanlist;

    int readRawSample();
};


#endif //OBP_COMEDIHANDLER_H
