/**
 * @file        test_OBPDetection.cpp
 * @brief       OBPDetection test implementation.
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 * @details
 * Very basic testing of the OBPDetection class.
 * A set of sample data is stored in the same folder as this test 'p.dat' contains pressure values and 'o.dat'
 * contains oscillation values. The values are passed to the OBPDetection object. If the OBPDetection object
 * successfully calculates all values as not equal to 0.0 the test passes.
 */

#include <iostream>
#include <fstream>
#include "../OBPDetection.cpp"

int main()
{
    OBPDetection *obpDetect = new OBPDetection(1000.0);
    obpDetect->resetConfigValues();
    //string line;
    std::ifstream pFile("p.dat");
    std::ifstream oFile("o.dat");
    double tP, vP;
    double tO, vO;
    while (pFile >> tP >> vP)
    {
        if (!(oFile >> tO >> vO))
        { break; } // error

        if (obpDetect->processSample(vP, vO))
        {
            if (obpDetect->getIsEnoughData())
            {
                std::cout << obpDetect->getMAP() << " " << obpDetect->getSBP() << " " << obpDetect->getDBP()
                          << std::endl;
                break;
            }
        }
    }

    int ret = 0;
    //Processing procThread;
    if (obpDetect->getMAP() != 0.0 && obpDetect->getSBP() != 0.0 && obpDetect->getDBP() != 0.0 &&
    obpDetect->getAverageHeartRate() != 0.0)
    {
        std::cout << "Test passed";
    } else
    {
        std::cout << "Test failed";
        ret = 1;
    }

    delete obpDetect;
    return ret;
}
