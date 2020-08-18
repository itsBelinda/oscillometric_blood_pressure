/**
 * @file        ComediHandler.h
 * @brief
 * @author      Belinda Kneubühler belinda.kneubuehler@gmail.com
 * @date        2020-08-18
 * @author      Bernd Porr mail@berndporr.me.uk
 * @date        2005-2017
 * @author      Matthias H. Hennig hennig@cn.stir.ac.uk
 * @date        2003
 * @copyright   GNU General Public License v2.0
 *
 * @details
 */
#ifndef OBP_COMEDIHANDLER_H
#define OBP_COMEDIHANDLER_H


#include <comedilib.h>


#define COMEDI_SUB_DEVICE   0   /* using sub device 0 */
#define COMEDI_RANGE_ID     0   /* +/- 1.325V  for sigma device*/
#define COMEDI_NUM_CHANNEL  1   /* only one channel is used */
#define COMEDI_DEV_PATH     "/dev/comedi0" /* the path to access the comedi device */

class ComediHandler {
public:
    ComediHandler();
    ~ComediHandler();

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
