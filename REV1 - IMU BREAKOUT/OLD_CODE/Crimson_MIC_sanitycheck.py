#!/usr/bin/env python3

import numpy as np
from scipy.io import wavfile

fs = 44100
frequency = 440
length = 5

t = np.arange(length * fs) / fs
y = np.sin(frequency * 2 * np.pi * t)  #  Has frequency of 440Hz

wavfile.write("Sine.wav", fs, y)

