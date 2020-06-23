#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Jun 12 14:33:31 2020

@author: belinda
"""
#%% Imports

import matplotlib.pyplot as plt
import numpy as np
from scipy import signal
from scipy.interpolate import interp1d


#%% Get the data

data = np.loadtxt('../data/sample_07_10.dat')
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
# 5 Hz LP filter
f5 = 5
bLP, aLP = signal.butter(6, f5/fs*2, 'lowpass')
yfLP = signal.lfilter(bLP, aLP, ymmHg)

# 0.5 Hz HP filter
f05 = 0.5 # TODO: might be better to set lower ~0.3
bHP, aHP = signal.butter(2, f05/fs*2, 'highpass')
yfHP = signal.lfilter(bHP, aHP, yfLP)

yfBP = yfLP-yfHP

#%% Find peaks in derrivative (HP filtered)

localMax, _ = signal.find_peaks(yfHP, prominence = 0.3 )#distance = 500)


# get values of local maximas in pressure and time
yMaximas = yfBP[localMax]
tMaximas = t[localMax]
# the local max values of the oscillation
oscMax = yfHP[localMax]
# get indice of overal max pressure 
xPumpedUp = np.argmax(yMaximas)
# the pressure that was pumped up to
yPumpedUP = yMaximas[xPumpedUp]
# the start time of the deflation 
tPumpedUP = tMaximas[xPumpedUp]

localMin, _ = signal.find_peaks(-yfHP, prominence = 0.3 )
# get values of local maximas in pressure and time
yMinima = yfBP[localMin]
tMinima = t[localMin]
# the local max values of the oscillation
oscMin = yfHP[localMin]


deltaT = np.zeros(len(tMaximas))
delta2T = np.zeros(len(tMaximas))
validCnt = 0
oscStartInd = 0
oscEndInd = 0


#deltaT = np.diff(tMaximas)
#delta2T = np.diff(deltaT)
for i in range(1,len(tMaximas)-1):
    deltaT[i] = tMaximas[i]-tMaximas[i-1]
    delta2T[i] = deltaT[i]-deltaT[i-1]
    
    if oscStartInd == 0 :
        # check for start of oscillogram: 
        if np.abs(delta2T[i]) < 0.2 and i > (xPumpedUp + 5) :
            validCnt += 1
            if validCnt == 5 :
                oscStartInd = i - (validCnt-1)
        else:
            validCnt = 0
    elif oscEndInd == 0:
        #check for end of oscillogram
        # if np.abs(delta2T[i]) > 0.2:
        #     oscEndInd = i-3 #omitt the last one: -2 not -1       
        if (oscMax[oscStartInd]*1.2) > oscMax[i]: # more info on left side
            oscEndInd = i-3
            

if oscEndInd == 0:
    oscEndInd = len(tMaximas)-4

# data for processing:
yMaximasP = yMaximas[oscStartInd:oscEndInd+1]
tStart_a = np.where(yfBP == yMaximasP[0])
iStart = int(tStart_a[0]+100)
tEnd_a = np.where(yfBP == yMaximasP[-1])
iEnd = int(tEnd_a[0]-100)
tMaxP = tMaximas[oscStartInd:oscEndInd+1]
oscMaxP = oscMax[oscStartInd:oscEndInd+1]
deltaP = deltaT[oscStartInd:oscEndInd+1]
tP = t[iStart:iEnd+1]
yhpP = yfHP[iStart:iEnd+1]
ylpP = yfLP[iStart:iEnd+1]
ybpP = yfBP[iStart:iEnd+1]

# make sure minimas are (at least) defined for every maxima
minStart = np.argmax(tMinima>tMaximas[oscStartInd])-1
minEnd = np.argmax(tMinima>tMaximas[oscEndInd])
tMinP = tMinima[minStart:minEnd+1]
yMinimasP = yMinima[minStart:minEnd+1]

oscMinP = oscMin[minStart:minEnd+1]

dMaxMin = oscMaxP - oscMinP[0:len(tMaxP)]

avPulse = np.average(deltaP)
pulse=60/avPulse
print("Pulse: ", np.around(pulse, decimals=1))



# calculate systolic and diastolic blood pressure from derrivative
dPres = np.zeros(len(dMaxMin))
dPresN = np.zeros(len(dMaxMin))
dPresNP = np.zeros(len(dMaxMin))

# TODO: which is needed? Interpolation?
for i in range(0,len(dMaxMin)-1):
    dPres[i] = (dMaxMin[i+1]-dMaxMin[i])
    dPresN[i] = (dMaxMin[i+1]-dMaxMin[i])/(tMaxP[i+1]-tMaxP[i])
    dPresNP[i] = (dMaxMin[i]-dMaxMin[i+1])/(yMaximasP[i+1]-yMaximasP[i])
    
maxVal = np.argmax(dMaxMin)    
pMAP = yMaximasP[maxVal]

maxChange = np.argmax(dPresNP[0:maxVal])
minChange = np.argmin(dPresNP[maxVal:])+maxVal

pSBP = yMaximasP[maxChange]
pDBP = yMaximasP[minChange]

print("using only min/max values of oscillogram")
print("    MAP: ", np.around(pMAP, decimals=2) )
print("    SBP: ", np.around(pSBP, decimals=2) )
print("    DBP: ", np.around(pDBP, decimals=2) )




#interpolation
intMax = interp1d(yMaximasP, oscMaxP, kind='quadratic')
intMin = interp1d(yMinimasP, oscMinP, kind='quadratic')

omweInter = intMax(ybpP)-intMin(ybpP)
argMaxInter = np.argmax(omweInter)
pMAPinter = ybpP[argMaxInter]

# delata alculated by hand and normalised in pressure
dPresInter = np.zeros(len(omweInter))
for i in range(0,len(omweInter)-1):
    dPresInter[i] = (omweInter[i+1]-omweInter[i]) / (ybpP[i+1]-ybpP[i])
    

# delata alculated by hand and normalised in pressure
maxChangeInter = np.argmax(dPresInter[0:argMaxInter])
minChangeInter = np.argmin(dPresInter[argMaxInter:])+argMaxInter

pSBPInter = ybpP[maxChangeInter]
pDBPInter = ybpP[minChangeInter]

print("interpolation (by hand, normalised in pressure): " )
print("    MAP: ", np.around(pMAPinter, decimals=2) )
print("    SBP: ", np.around(pSBPInter, decimals=2) )
print("    DBP: ", np.around(pDBPInter, decimals=2) )

# delata alculated by python
dIPres = np.diff(omweInter)
dIPres = np.append(0,dIPres)

maxChangeI = np.argmax(dIPres[0:argMaxInter])
minChangeI = np.argmin(dIPres[argMaxInter:])+argMaxInter

pSBPInt = ybpP[maxChangeI]
pDBPInt = ybpP[minChangeI]

print("interpolation (by pyhton, not normalised): " )
print("    MAP: ", np.around(pMAPinter, decimals=2) )
print("    SBP: ", np.around(pSBPInt, decimals=2) )
print("    DBP: ", np.around(pDBPInt, decimals=2) )




#%% plot imporant part of singal:
fig_processSignal, (proc_Pressure, proc_env, proc_delta) = plt.subplots(3,1,
                sharex=False,sharey=False,num='derrivative envelope')
#fig_processSignal.subplots_adjust(hspace=0)

proc_Pressure.plot(tP,ylpP, 'r', label='pressure')
proc_Pressure.plot(tP,ylpP-yhpP, 'b', label='deflation')

#proc_delta.plot(yfLP, intMax(tP)-intMin(tP), 'g', label='OMVE interp.')
proc_env.plot(yMaximasP, dMaxMin,  label='OMVE max-min')
proc_env.plot(yMaximasP, oscMaxP, 'r',marker='x', label='OMVE max')
proc_env.plot(yMinimasP, oscMinP,'b', marker='x', label='OMVE min')
proc_env.plot(ybpP, intMin(ybpP), 'g', label='OMVE min')
proc_env.plot(ybpP, intMax(ybpP), 'm', label='OMVE max')
proc_env.plot(ybpP, omweInter, 'c', label='OMVE interp')

proc_delta.plot(yMaximasP, dPres,  label='dOMVE')
proc_delta.plot(yMaximasP, dPresN,  label='dOMVE n in time')
proc_delta.plot(yMaximasP, dPresNP,  label='dOMVE n in pressure')
proc_delta.plot(ybpP, dIPres,  label='dOMVE interpolation')
proc_delta.plot(ybpP, dPresInter,  label='dOMVE interpolation n in pressure')


proc_Pressure.legend()
proc_delta.legend()

fig_processSignal.suptitle('Blood Pressure', fontsize=20) 
proc_Pressure.set_ylabel('mmHg', fontsize=16)
proc_Pressure.set_xlabel('Time (s)', fontsize=16)
proc_Pressure.set_xlim(min(tP), max(tP))

proc_env.set_ylabel('detlat/mmHg', fontsize=16)
proc_env.set_xlabel('mmHg', fontsize=16)
proc_env.set_xlim(max(yMaximasP)+10, min(yMaximasP)-10)
proc_delta.set_ylabel('detlat/mmHg', fontsize=16)
proc_delta.set_xlabel('mmHg', fontsize=16)
proc_delta.set_xlim(max(yMaximasP)+10, min(yMaximasP)-10)
#proc_delta.set_ylim(-3, 3)
plt.get_current_fig_manager().window.showMaximized()


#%% Plot full signals
#(dMaxMin[i]-dMaxMin[i-1])/tMaxP[i]
# fig_timeSignal, (time_filt, time_LP, time_HP) = plt.subplots(3,1,
#                 sharex=True,sharey=False,num='mmHg Signal')
# fig_timeSignal.subplots_adjust(hspace=0)

# time_filt.plot(t,ymmHg, 'b', label='raw')
# time_LP.plot(t, yfLP, 'r', label='LP')
# time_HP.plot(t,yfHP, 'g', label='HP')
# time_HP.plot(tMaximas, oscMax, 'm', label='local maximas')
# time_HP.plot(tMinima, oscMin, 'm', label='local minimas')
# time_HP.plot(tMaximas, deltaT, 'c', label='time since last maxima')
# time_HP.plot(tMaximas, delta2T, 'k', label='2nd derrivative in time')
# time_HP.axvline(x = (iStart/1000))
# time_HP.axvline(x = (iEnd/1000))

# time_filt.legend()
# time_LP.legend()
# time_HP.legend()

# fig_timeSignal.suptitle('Blood Pressure', fontsize=20) 
# time_filt.set_ylabel('mmHg', fontsize=16)
# time_LP.set_ylabel('mmHg', fontsize=16)
# time_HP.set_ylabel('detlat/mmHg', fontsize=16)
# time_HP.set_xlabel('Time (s)', fontsize=16)
# time_LP.set_xlim(0, max(t))
# time_HP.set_ylim(-3, 3)
# plt.get_current_fig_manager().window.showMaximized()
 



#%% Print out data

plt.show()
