import numpy as np
import scipy as sp
import matplotlib.pyplot as plt
import scipy.fftpack
from scipy.fftpack import fft, fftfreq
import pandas as pd

# import csv file
df = pd.read_csv("sensor_log.csv", header=0)


size = df.shape
row_num = size[0]
col_num = size[1]

# FFT parameters
N = row_num
T = 1.0 / 100.0 #sample every 10ms, that's 100 times per second

ticksteps = 20

# ax dataset
df_ax = df[["Ax"]].copy()
print(df_ax.head())
# trick python into thinking these are floats
df_ax.Ax = pd.to_numeric(df_ax.Ax, errors="coerce")
# FFT on  ax
ax_yf = fft(df_ax)
ax_amp = 2.0 / N * np.abs(ax_yf)

# ay dataset
df_ay = df[["Ay"]].copy()
print(df_ay.head())
# trick python into thinking these are floats
df_ay.Ay = pd.to_numeric(df_ay.Ay, errors="coerce")
# FFT on  ay
ay_yf = fft(df_ay)
ay_amp = 2.0 / N * np.abs(ay_yf)

# az dataset
df_az = df[["Az"]].copy()
print(df_az.head())
# trick python into thinking these are floats
df_az.Az = pd.to_numeric(df_az.Az, errors="coerce")
# FFT on  az
az_yf = fft(df_az)
az_amp = 2.0 / N * np.abs(az_yf)

# N1 = 600
# T1 = 1.0 / 800.0
# x = np.linspace(0.0, N1 * T1, N1)
# # test plot
# y1 = np.sin(55.789 * 2.0 * np.pi * x) + np.sin(123.887 * 2.0 * np.pi * x)
# yf = fft(y1)
# amp = 2.0 / N1 * np.abs(yf)

# plot results
fig = plt.figure()
ax1 = fig.add_subplot(3, 1, 1)
ax2 = fig.add_subplot(3, 1, 2)
# ax3 = fig.add_subplot(3,1,3)

ax2.set(xticks=(np.arange(0, N // 2, ticksteps)),ylabel="Amplitude",xlabel="Frequency (Hz)",title="FFT result",xlim=[0, N // 2])
ax2.set_xticklabels(np.round(fftfreq(N, T)[:N//2],2)[::ticksteps],rotation=0)

# ax3.set(xticks=(np.arange(0,N1//2,ticksteps)), ylabel='Amplitude',xlabel='Frequency (Hz)',title='FFT result',xlim=[0,N1//2])
# ax3.set_xticklabels(np.round(fftfreq(N1,T1)[:N1//2],2)[::ticksteps],rotation=0)

df_ax.plot(ax=ax1, legend=True)
df_ay.plot(ax=ax1, legend=True)
df_az.plot(ax=ax1, legend=True)
ax2.plot(ax_amp[: N // 2])
ax2.plot(ay_amp[: N // 2])
ax2.plot(az_amp[: N // 2])
# ax3.plot(amp[:N1//2])
ax2.grid()
plt.show()

