# Oscillometric Blood Pressure Measurement

**This project is under development.**

The aim of this project is to  test the reliability of the automatic oscillometric blood pressure measurement process by implementing a real time application in C++. The application will automatically measure the user's pulse and blood pressure by using a manual blood pressure cuff equipped with a pressure sensor that is connected to a [USBDUX-SIGMA](http://www.linux-usb-daq.co.uk/prod2_duxsigma/) converter, connected to a Linux computer.

If there is no USBDUX devide connected to the computer, the application will not start up. If the USBDUX-D device is used, a warning will be written into the programs log file, but the application will still run. The USBDUX-D device has only 12-bit instead of 24, like the SIGMA device, which is not enough for the blood pressure detection to work.

# License

This piece of software is released under the GNU General Public License.
[http://www.gnu.org/licenses/licenses.(html#GPL](http://www.gnu.org/licenses/licenses.html#GPL)
So please go ahead and modify/extend it.


# Installation (C++)
The following instructions concern the code located in the [C++ foler](c++).

The following libraries are required to compile and run the program:
 - [Comedi](http://www.comedi.org)
 - [Qt5 and Qwt](https://qwt.sourceforge.io/)
 - [iir1](https://github.com/berndporr/iir1)

## Installing dependencies (for Ubuntu)
This quick guide assumes g++ and cmake are installed with a g++ verstion that supports C++20.

### Install Comedi Development Librairy
    sudo apt-get install libcomedi-dev
### Install Qt and Qwt Development Librairy
    sudo apt-get install qt-default
    sudo apt-get install libqwt-qt5-dev
### Install the IIR Filter Librairy (iir1) by Bernd Porr
Link the repository to the package manager:

    sudo add-apt-repository ppa:berndporr/dsp
Then install as usual:

    sudo apt-get install iir1-dev

## Building the Projcet
The project is now also set-up as a cmake project (details are defined in [CMakeList.txt](/c%2B%2B/CMakeLists.txt)). 
Run `cmake .` from the console in the source foler to generate the Makefile and `make` to compile. 

## Running the Application
Finally, run the application form the source folder with `./obp`.

# Credits
The first C++ setup of this project is based on a data aquisiton project by Bernd Porr, which can be found [here](https://github.com/berndporr/psth-vep).  This was initially developed by Tobi Delbrücks 
(http://[http://www.ini.uzh.ch/~tobi/friend/](http://www.ini.uzh.ch/~tobi/friend/)). 
