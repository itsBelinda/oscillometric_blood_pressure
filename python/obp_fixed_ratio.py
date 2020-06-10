# -*- coding: utf-8 -*-
"""
Created on Wed Jun 10 10:29:51 2020

@author: bekn
"""


# -*- coding: utf-8 -*-
"""
obp_tests.py
"""
#%% Imports

import matplotlib.pyplot as plt
import numpy as np
from scipy import signal
plt.rcParams['axes.grid'] = True


#%% Get the data

data = np.loadtxt('../data/sample_07_11.dat')
fs= 1000 # Hz
resolution = 24 # bits
N = len(data)
T = 1/fs
bin_size = fs/N 
y1 = data[:,1]
t = data[:,0] 

# convert data
vmin = -1.325
vmax = +1.325
xmax = (2**24 -1)

# Choose data set to work with and convert to voltage
if np.max(y1) > vmax:
    #for raw data: 
    y = ((y1 * (vmax-vmin) / xmax ) + vmin) 
    t = t/1000
else:
    y = y1 #for raw data: ((y1 * (vmax-vmin) / xmax ) + vmin) 



#%% Convert voltage to mmHg

#
ambientV = 0.710#0.675 # from calibration
mmHg_per_kPa = 7.5006157584566 # from literature
kPa_per_V = 50 # 20mV per 1kPa / 0.02 or * 50 - from sensor datasheet
corrFact = 2.50 # from calibration

ymmHg = (y - ambientV)  * mmHg_per_kPa * kPa_per_V * corrFact 


#%% Filter design
#
## 50 Hz notch filter
#f0 = 48.0
#f1 = 52.0
#sos50 = signal.butter(6, [f0/fs*2,f1/fs*2], 'bandstop', output='sos')
#filt50 = iir_filter.IIR_filter(sos50)

# 5 Hz LP filter
f5 = 5
bLP, aLP = signal.butter(6, f5/fs*2, 'lowpass')
yfLP = signal.lfilter(bLP, aLP, ymmHg)

# 0.5 Hz HP filter
f05 = 0.5
bHP, aHP = signal.butter(2, f05/fs*2, 'highpass')
yfHP = signal.lfilter(bHP, aHP, yfLP)



#%% Find peaks in derrivative (HP filtered)

localMax, _ = signal.find_peaks(yfHP, prominence = 0.1 )#distance = 500)

# get values of local maximas in pressure and time
yMaximas = yfLP[localMax]
tMaximas = t[localMax]
# the local max values of the oscillation
oscMax = yfHP[localMax]
# get indice of overal max pressure 
xPumpedUp = np.argmax(yMaximas)
# the pressure that was pumped up to
yPumpedUP = yMaximas[xPumpedUp]
# the start time of the deflation 
tPumpedUP = tMaximas[xPumpedUp]


deltaT = np.zeros(len(tMaximas))
delta2T = np.zeros(len(tMaximas))
oscValid = np.zeros(len(tMaximas))
validCnt = 0
oscStart = 0
oscEnd = 0
for i in range(1,len(tMaximas)-1):
    deltaT[i] = tMaximas[i]-tMaximas[i-1]
    delta2T[i] = deltaT[i]-deltaT[i-1]
    
    if oscStart == 0 :
        # check for start of oscillogram: 
        if np.abs(delta2T[i]) < 0.1 and i > xPumpedUp :
            validCnt += 1
            if validCnt == 5 :
                oscStart = tMaximas[i - (validCnt-1)]
        else:
            validCnt = 0
    elif oscEnd == 0:
        #check for end of oscillogram
        if np.abs(delta2T[i]) > 0.1:
            oscEnd = tMaximas[i-1]
        
        
    
# data for processing:
iStart = int(oscStart*1000)
iEnd = int(oscEnd*1000)
tP = t[iStart:iEnd]
yhpP = yfHP[iStart:iEnd]
ylpP = yfLP[iStart:iEnd]

# plot imporant part of singal:
fig_processSignal, (proc_Pressure, proc_delta) = plt.subplots(2,1,
                sharex=True,sharey=False,num='proces Signal')
fig_processSignal.subplots_adjust(hspace=0)

proc_Pressure.plot(tP,ylpP, 'r', label='pressure')
proc_delta.plot(tP, yhpP, 'g', label='oscillogram')

proc_Pressure.legend()
proc_delta.legend()

fig_processSignal.suptitle('Blood Pressure', fontsize=20) 
proc_Pressure.set_ylabel('mmHg', fontsize=16)
proc_delta.set_ylabel('mmHg', fontsize=16)
proc_delta.set_ylabel('detlat/mmHg', fontsize=16)
proc_delta.set_xlabel('Time (s)', fontsize=16)
proc_Pressure.set_xlim(min(tP), max(tP))
proc_delta.set_ylim(-3, 3)
plt.get_current_fig_manager().window.showMaximized()


#%% Plot full signals

fig_timeSignal, (time_filt, time_LP, time_HP) = plt.subplots(3,1,
                sharex=True,sharey=False,num='mmHg Signal')
fig_timeSignal.subplots_adjust(hspace=0)

time_filt.plot(t,ymmHg, 'b', label='v')
time_LP.plot(t, yfLP, 'r')
time_LP.plot(t[localMax], yfLP[localMax], "x")
time_HP.plot(t,yfHP, 'g', label='HP')
time_HP.plot(tMaximas, oscMax, 'm', label='local maximas')
time_HP.plot(tMaximas, deltaT, 'c', label='time since last maxima')
time_HP.plot(tMaximas, delta2T, 'k', label='2nd derrivative in time')
time_HP.axvline(x = oscStart)
time_HP.axvline(x = oscEnd)

time_filt.legend()
time_LP.legend()
time_HP.legend()

fig_timeSignal.suptitle('Blood Pressure', fontsize=20) 
time_filt.set_ylabel('mmHg', fontsize=16)
time_LP.set_ylabel('mmHg', fontsize=16)
time_HP.set_ylabel('detlat/mmHg', fontsize=16)
time_HP.set_xlabel('Time (s)', fontsize=16)
time_LP.set_xlim(0, max(t))
time_HP.set_ylim(-3, 3)
plt.get_current_fig_manager().window.showMaximized()
 



#%% Print out data

plt.show()
