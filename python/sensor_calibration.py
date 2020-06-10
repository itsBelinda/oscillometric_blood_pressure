#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Jun  5 16:15:29 2020

To calibrate, record about 20 s at ambient pressure, 
then pump up to 150 mmHg and keep it stable.Stop recording at 150 mmHg with
the signal stable for at least 5 s.
@author: belinda
"""

#%% Imports

import matplotlib.pyplot as plt
import numpy as np
from scipy import signal
plt.rcParams['axes.grid'] = True


#%% Get the data

data = np.loadtxt('../data/sample_07_01.dat')
fs= 1000 # Hz
resolution = 24 # bits
N = len(data)
T = 1/fs
bin_size = fs/N 
y1 = data[:,1]
y2 = data[:,2]
y3 = data[:,3]
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


#%% Filter 
f1 = 0.5
bLP, aLP = signal.butter(2, f1/fs*2, 'lowpass')
yfLP = signal.lfilter(bLP, aLP, y)

#%% Calculations
# take seconds 10-20 and average over filtered values to get 0 mmHg
y0Hg = np.mean(yfLP[10*fs:20*fs])

# take the last 5s to average over filtered values to get 150 mmHg
y150Hg = np.mean(yfLP[-10*fs:])

factor = 150 / ((y150Hg - y0Hg) * 50 * 7.5006157584566)
print("voltage at ambient pressure: ", np.around(y0Hg, decimals=5) )
print("correction factor for voltage divider: ", np.around(factor, decimals=5) )

#%% Plot to check

fig_timeSignal, (time_filt) = plt.subplots(1,1,
                sharex=True,sharey=False,num='')
fig_timeSignal.subplots_adjust(hspace=0)

time_filt.plot(t,y, 'b', label='raw')
time_filt.plot(t,yfLP, 'r', label='filtered')

time_filt.legend()

fig_timeSignal.suptitle('Blood Pressure', fontsize=20) 
time_filt.set_ylabel('mmHg', fontsize=16)
time_filt.set_ylabel('mmHg', fontsize=16)
time_filt.set_xlabel('Time (s)', fontsize=16)
time_filt.set_xlim(0, max(t))
plt.get_current_fig_manager().window.showMaximized()

##%% Plot frequency spectrum
#
#y_F = np.fft.fft(y)
#y_F_show = abs(y_F)#20 * np.log10(abs(y_F))
#yLP_F = np.fft.fft(yfLP)
#yLP_F_show = 20 * np.log10(abs(yLP_F))
#yHP_F = np.fft.fft(yfHP)
#yHP_F_show = 20 * np.log10(abs(yHP_F))
#
#fig_specSignal, (spec_orig,
#                 spec_filtered) = plt.subplots(2,
#                                           1,
#                                           sharex=True,
#                                           sharey=True,
#                                           num='Frequency Spectrum')
#fig_specSignal.subplots_adjust(hspace=0)
#fig_specSignal.suptitle(
#    'Single-sided Magnitude Spectrum '
#    'Frequency Domain (fs=1 kHz)',
#    fontsize=16)
#plt.get_current_fig_manager().window.showMaximized()
#
#spec_orig.plot(faxis[0:(int(len(y_F) / 2)) + 1],
#               y_F_show[0:(int(len(y_F) / 2)) + 1],
#               'b',
#               label='original')
#
#spec_filtered.plot(faxis[0:(int(len(y_F) / 2)) + 1],
#               yLP_F_show[0:(int(len(y_F) / 2)) + 1],
#               'r',
#               label='filtered')
#
#spec_orig.legend()
#spec_filtered.legend()
#
#spec_orig.set_ylabel('Magnitude (dB)', fontsize=16)
#spec_filtered.set_ylabel('Magnitude (dB)', fontsize=16)
#spec_filtered.set_xlabel('Frequency [Hz]', fontsize=16)
#spec_orig.set_ylim(-1, 800)
#spec_orig.set_xlim(-1, 12)