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
from scipy.interpolate import interp1d


#%% Get the data

data = np.loadtxt('../data/2020-07-21_15-13-52_test.dat')
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
#if np.max(y1) > vmax:
    #for raw data: 
 #   y = ((y1 * (vmax-vmin) / xmax ) + vmin) 
  #  t = t/1000
#else:
 #   y = y1 #for raw data: ((y1 * (vmax-vmin) / xmax ) + vmin) 



#%% Convert voltage to mmHg

#
ambientV = 0.710#0.675 # from calibration
mmHg_per_kPa = 7.5006157584566 # from literature
kPa_per_V = 50 # 20mV per 1kPa / 0.02 or * 50 - from sensor datasheet
corrFact = 2.50 # from calibration

ymmHg = y1#(y - ambientV)  * mmHg_per_kPa * kPa_per_V * corrFact 


#%% Filter design
#
## 50 Hz notch filter
#f0 = 48.0
#f1 = 52.0
#sos50 = signal.butter(6, [f0/fs*2,f1/fs*2], 'bandstop', output='sos')
#filt50 = iir_filter.IIR_filter(sos50)

# 5 Hz LP filter
f5 = 10
bLP, aLP = signal.butter(6, f5/fs*2, 'lowpass')
yfLP = signal.lfilter(bLP, aLP, ymmHg)
#yfLP = ymmHg

# 0.5 Hz HP filter
f05 = 0.4 # TODO: might be better to set lower ~0.3
bHP, aHP = signal.butter(4, f05/fs*2, 'highpass')
yfHP = signal.lfilter(bHP, aHP, yfLP)



#%% Find peaks in derrivative (HP filtered)

localMax, _ = signal.find_peaks(yfHP, prominence = 0.3 )#distance = 500)


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

localMin, _ = signal.find_peaks(-yfHP, prominence = 0.3 )
# get values of local maximas in pressure and time
yMinima = yfLP[localMin]
tMinima = t[localMin]
# the local max values of the oscillation
oscMin = yfHP[localMin]


deltaT = np.zeros(len(tMaximas))
delta2T = np.zeros(len(tMaximas))
validCnt = 0
oscStartInd = 0
oscEndInd = 0

#%%


deltaTtest = np.diff(tMaximas)
#delta2T = np.diff(deltaT)
for i in range(1,len(tMaximas)-1):
    deltaT[i] = tMaximas[i]-tMaximas[i-1] #TODO: normalisation in time necessary?
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
        if np.abs(delta2T[i]) > 0.2:
            oscEndInd = i-3 #omitt the last one: -2 not -1       
        # if (oscMax[oscStartInd]*1.1) > oscMax[i]: # more info on left side
        #     oscEndInd = i
#%%
if oscEndInd == 0:
    oscEndInd = len(tMaximas)-3

# data for processing:
tStart = tMaximas[oscStartInd]
tEnd = tMaximas[oscEndInd]
iStart = int(tStart*1000)
iEnd = int(tEnd*1000)
tMaxP = tMaximas[oscStartInd:oscEndInd+1]
oscMaxP = oscMax[oscStartInd:oscEndInd+1]
deltaP = deltaT[oscStartInd:oscEndInd+1]
deltaPtest = deltaTtest[oscStartInd:oscEndInd+1]
tP = t[iStart:iEnd+1]
yhpP = yfHP[iStart:iEnd+1]
ylpP = yfLP[iStart:iEnd+1]

# make sure minimas are (at least) defined for every maxima
minStart = np.argmax(tMinima>tMaximas[oscStartInd])-1
minEnd = np.argmax(tMinima>tMaximas[oscEndInd])
tMinP = tMinima[minStart:minEnd+1]
oscMinP = oscMin[minStart:minEnd+1]

dMaxMin = oscMaxP - oscMinP[0:len(tMaxP)]

#%%

avPulse = np.average(deltaP)
pulse=60/avPulse
print("Pulse: ", np.around(pulse, decimals=1))

# only use max values
argMax = np.argmax(oscMaxP)

pMAP = yfLP[int(round(tMaxP[argMax]*1000))] # simple version: just take highest value
#pMAP = yfLP[int(round(tMaxP[np.argmax(oscMaxP)]*1000))] #equal
#old:pMAPav = np.average(ylpP[maxArg-int(avPulse*1000/2):maxArg+int(avPulse*1000/2)]) #averaged over one pulse

# just look at the max value
print("MAP (max value): ", np.around(pMAP, decimals=2))
#print("MAP (av value over pulse len): ", np.around(pMAPav, decimals=2))
          
searchSys = np.max(oscMaxP)*0.55
indS = np.argmax(oscMaxP > searchSys)
dt = tMaxP[indS]-tMaxP[indS-1]
dosc = oscMaxP[indS]-oscMaxP[indS-1]
pSBPmax = yfLP[int(round((((searchSys-oscMaxP[indS-1])*dt/dosc)+tMaxP[indS-1])*1000))]

searchDia = np.max(oscMaxP)*0.70
indD = argMax + np.argmax(oscMaxP[argMax:] < searchDia)
dt = tMaxP[indD]-tMaxP[indD-1]
dosc = oscMaxP[indD]-oscMaxP[indD-1]
pDBPmax = yfLP[int(round((((searchDia-oscMaxP[indD-1])*dt/dosc)+tMaxP[indD-1])*1000))]
print("    SBP: ", np.around(pSBPmax, decimals=2) )
print("    DBP: ", np.around(pDBPmax, decimals=2) )

#%%
#use min and max values (dMaxMin)

argMax = np.argmax(dMaxMin)

pMAPdif = yfLP[int(round(tMaxP[argMax]*1000))] # simple version: just take highest value

# just look at the max value
print("MAP (max-min value): ", np.around(pMAPdif, decimals=2))
#print("MAP (av value over pulse len): ", np.around(pMAPav, decimals=2))
          
searchSys = np.max(dMaxMin)*0.55
indS = np.argmax(dMaxMin > searchSys)
dt = tMaxP[indS]-tMaxP[indS-1]
dosc = dMaxMin[indS]-dMaxMin[indS-1]
pSBPmax = yfLP[int(round((((searchSys-dMaxMin[indS-1])*dt/dosc)+tMaxP[indS-1])*1000))]

searchDia = np.max(dMaxMin)*0.70
indD = argMax + np.argmax(dMaxMin[argMax:] < searchDia)
dt = tMaxP[indD]-tMaxP[indD-1]
dosc = dMaxMin[indD]-dMaxMin[indD-1]
pDBPmax = yfLP[int(round((((searchDia-dMaxMin[indD-1])*dt/dosc)+tMaxP[indD-1])*1000))]
print("    SBP: ", np.around(pSBPmax, decimals=2) )
print("    DBP: ", np.around(pDBPmax, decimals=2) )



# #interpolation
# intMax = interp1d(tMaxP, oscMaxP, kind='linear')
# intMin = interp1d(tMinP, oscMinP, kind='linear')

# omweInter = intMax(tP) - intMin(tP)
# pMAPinter = ylpP[np.argmax(omweInter)]
# print("MAP (interpolation): ", np.around(pMAPinter, decimals=2) )

# maxArg = np.argmax(omweInter)
# pSBPInt = ylpP[np.argmax(omweInter > np.max(omweInter)*0.55)]
# pDBPInt = ylpP[maxArg + np.argmax(omweInter[maxArg:] < np.max(omweInter)*0.70)]
# print("    SBP: ", np.around(pSBPInt, decimals=2) )
# print("    DBP: ", np.around(pDBPInt, decimals=2) )

#poly fitting
polyFit = np.polyfit(tMaxP, dMaxMin,4)
poly = np.poly1d(polyFit)
omwePoly = poly(tP)

pMAPpoly = ylpP[np.argmax(omwePoly)]
print("MAP (polyfit): ", np.around(pMAPpoly, decimals=2) )

maxArg = np.argmax(omwePoly)
pSBPpoly = ylpP[np.argmax(omwePoly > np.max(omwePoly)*0.55)]
pDBPpoly = ylpP[maxArg + np.argmax(omwePoly[maxArg:] < np.max(omwePoly)*0.70)]
print("    SBP: ", np.around(pSBPpoly, decimals=2) )
print("    DBP: ", np.around(pDBPpoly, decimals=2) )


#%% plot imporant part of singal:
fig_processSignal, (proc_Pressure, proc_delta, proc_omve) = plt.subplots(3,1,
                sharex=True,sharey=False,num='proces Signal')
fig_processSignal.subplots_adjust(hspace=0)

proc_Pressure.plot(tP,ylpP, 'r', label='pressure')
proc_Pressure.plot(tP,ylpP-yhpP, 'b', label='deflation')
proc_delta.plot(tMaxP, deltaP, 'c', label='time since last maxima')
proc_delta.plot(tMaxP, deltaPtest, 'c', label='time since last maxima')
proc_delta.plot(tP, yhpP, 'g', label='oscillogram')
proc_delta.plot(tMaxP, oscMaxP, 'm', label='envelope')
proc_delta.plot(tMinP, oscMinP, 'm')

# proc_delta.plot(tP, intMax(tP), 'b', label='OMV interp.')
# proc_delta.plot(tP, intMin(tP), 'b')

# proc_omve.plot(tP, intMax(tP)-intMin(tP), 'g', label='OMVE interp.')
proc_omve.plot(tMaxP, dMaxMin, 'b', label='OMVE from min/max')
proc_omve.plot(tP, omwePoly, 'm', label='OMVE polyfit')

proc_Pressure.legend()
proc_delta.legend()
proc_omve.legend()

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

time_filt.plot(t,ymmHg, 'b', label='raw')
time_LP.plot(t, yfLP, 'r', label='LP')
# time_LP.plot(t[localMax], yfLP[localMax], "x")
# time_LP.plot(t[localMin], yfLP[localMin], "x")
# time_HP.plot(t[localMax], yfHP[localMax], "x")
# time_HP.plot(t[localMin], yfHP[localMin], "x")
time_HP.plot(t,yfHP, 'g', label='HP')
time_HP.plot(tMaximas, oscMax, 'm', label='local maximas')
time_HP.plot(tMinima, oscMin, 'm', label='local minimas')
time_HP.plot(tMaximas, deltaT, 'c', label='time since last maxima')
time_HP.plot(tMaximas, delta2T, 'k', label='2nd derrivative in time')
time_HP.axvline(x = tStart)
time_HP.axvline(x = tEnd)

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
 


dataOSC = np.loadtxt('../data/osc.dat')
yOSC = dataOSC[:,1]
tOSC = dataOSC[:,0] 

fig_osc, (plt_osc) = plt.subplots(1,1)
plt_osc.plot(tOSC, yOSC)
plt_osc.set_xlim(9, max(tOSC))
#%% Print out data

plt.show()
