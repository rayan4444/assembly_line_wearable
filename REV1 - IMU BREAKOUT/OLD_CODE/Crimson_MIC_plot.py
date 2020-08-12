import numpy as np
from numpy import fft
from scipy.io import wavfile
import matplotlib.pyplot as plt

# recording of me whistling at ~1.2 and 1khz
fs_rate, signal = wavfile.read("Sine.wav")

# # convert stereo to mono
# signal = signal.mean(axis=1)

# generate time in seconds
t = np.arange(signal.shape[0]) / fs_rate

# create some plots
fig = plt.figure()
ax1 = fig.add_subplot(2, 1, 1)
ax2 = fig.add_subplot(2, 1, 2)

# generate FFT and frequencies
sp = abs(fft.fft(signal))
sp_half = sp[range(len(signal) // 2)]

freq = fft.fftfreq(len(signal), 1 / fs_rate)
freq_half = freq[range(len(signal) // 2)]


# plot
ax1.plot(t, signal)
ax2.plot(freq_half, abs(sp_half))

plt.show()
