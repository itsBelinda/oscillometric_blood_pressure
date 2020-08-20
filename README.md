# Oscillometric Blood Pressure Measurement

The aim of this project is to  test the reliability of automatic oscillometric blood pressure measurements by implementing a real-time application in C++. The application automatically measures the user's pulse and blood pressure by using a manual blood pressure cuff equipped with a pressure sensor that is connected to a [USB-DUX-SIGMA](http://www.linux-usb-daq.co.uk/prod2_duxsigma/) converter, connected to a Linux computer.

<p align="center">
  <img src="https://itsbelinda.github.io/obp/doc/latex/figures/hw_overview.svg" alt="obp hardware overview." width="80%">     
</p>

If there is no USB-DUX devide connected to the computer, the application will not start up. If the USBDUX-D device is used, a warning will be written into the programs log file, but the application will still run. The USBDUX-D device has only 12-bit instead of 24, like the SIGMA device, which is not enough for the blood pressure detection to work.

## Demonstration

A demonstration of the application can be found on YouTube.

<p align="center" target="_blank">
  <a href="https://youtu.be/3zEBVUrJrbY">
  <img src="https://img.youtube.com/vi/3zEBVUrJrbY/sddefault.jpg" alt="obp hardware overview.">    
  </a>
</p>


# Installation (C++)
The following instructions concern the code located in the [C++ folder](https://github.com/itsBelinda/obp/tree/master/c%2B%2B).

The following libraries are required to compile and run the program:
 - [Comedi](http://www.comedi.org)
 - [Qt5 and Qwt](https://qwt.sourceforge.io/)
 - [iir1](https://github.com/berndporr/iir1)

## Cloning the Repository
**IMPORTANT:** This repository contains a submodule, the software will not build, if the submodule is not cloned. To clone both this repository and the submodule run the following commands:

    git clone https://github.com/itsBelinda/obp.git
    git submodule init
    git submodule update
    
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
The project is set-up as a cmake project (details are defined in [CMakeList.txt](https://github.com/itsBelinda/obp/tree/master/c%2B%2B/CMakeLists.txt)). 
Run `cmake .` from the console in the source foler ([c++](https://github.com/itsBelinda/obp/tree/master/c%2B%2B) to generate the Makefile and `make` to compile. 
Run `ctest` to run the test.


## Running the Application
Finally, run the application form the source folder with `./obp`.


# License

This piece of software is released under the GNU General Public License.
[http://www.gnu.org/licenses/licenses.html#GPL](http://www.gnu.org/licenses/licenses.html#GPL)
So please go ahead and modify/extend it.


# Credits
The first C++ setup of this project is based on a data aquisiton project by Bernd Porr, which can be found [here](https://github.com/berndporr/psth-vep).  This was initially developed by Tobi Delbrücks 
(http://[http://www.ini.uzh.ch/~tobi/friend/](http://www.ini.uzh.ch/~tobi/friend/)). 
