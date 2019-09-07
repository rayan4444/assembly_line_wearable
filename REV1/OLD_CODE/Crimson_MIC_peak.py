#!/usr/bin/env python
# Implementation of algorithm from http://stackoverflow.com/a/22640362/6029703
import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
import pandas as pd


def thresholding_algo(y, lag, threshold, influence):
    signals = np.zeros(len(y))
    filteredY = np.array(y)
    avgFilter = [0] * len(y)
    stdFilter = [0] * len(y)
    avgFilter[lag - 1] = np.mean(y[0:lag])
    stdFilter[lag - 1] = np.std(y[0:lag])
    for i in range(lag, len(y) - 1):
        if abs(y[i] - avgFilter[i - 1]) > threshold * stdFilter[i - 1]:
            if y[i] > avgFilter[i - 1]:
                signals[i] = 1
            else:
                signals[i] = -1

            filteredY[i] = influence * y[i] + (1 - influence) * filteredY[i - 1]
            avgFilter[i] = np.mean(filteredY[(i - lag) : i])
            stdFilter[i] = np.std(filteredY[(i - lag) : i])
        else:
            signals[i] = 0
            filteredY[i] = y[i]
            avgFilter[i] = np.mean(filteredY[(i - lag) : i])
            stdFilter[i] = np.std(filteredY[(i - lag) : i])

    return dict(
        signals=np.asarray(signals),
        avgFilter=np.asarray(avgFilter),
        stdFilter=np.asarray(stdFilter),
    )


# Data
fs, y = wavfile.read("output.wav")
# ax=np.genfromtxt('ax.csv',delimiter=',')

# Settings: lag = 30, threshold = 5, influence = 0
lag = 10
threshold = 10
influence = 0.5

# Run algo with settings from above
result = thresholding_algo(y, lag=lag, threshold=threshold, influence=influence)

# save signal arrays into pandas dataframes
df_y = pd.DataFrame(result["signals"])
print(df_y)
# Plot result
fig = plt.figure()
ax1 = fig.add_subplot(2, 1, 1)
ax1.plot(np.arange(1, len(y) + 1), y)

ax1.plot(np.arange(1, len(y) + 1), result["avgFilter"], color="cyan", lw=2)

ax1.plot(
    np.arange(1, len(y) + 1),
    result["avgFilter"] + threshold * result["stdFilter"],
    color="green",
    lw=2,
)


ax2 = fig.add_subplot(2, 1, 2)
ax2.plot(np.arange(1, len(y) + 1), result["signals"], color="red", lw=2)

plt.show()
