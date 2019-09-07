import numpy as np
import scipy as sp
import matplotlib.pyplot as plt
import scipy.fftpack
from scipy.fftpack import fft, fftfreq
import pandas as pd

# import csv file
df = pd.read_csv("sensor_log.csv", header=0)

df_ax = df[["Ax"]].copy()
print(df_ax.head())
df_ax.Ax = pd.to_numeric(df_ax.Ax, errors="coerce")

df_ax.to_csv(r"~/Desktop/Crimson/ax.csv", index=False)

# ay dataset
df_ay = df[["Ay"]].copy()
print(df_ay.head())
df_ay.Ay = pd.to_numeric(df_ay.Ay, errors="coerce")
df_ay.to_csv(r"~/Desktop/Crimson/ay.csv", index=False)

df_az = df[["Az"]].copy()
print(df_az.head())
df_az.Az = pd.to_numeric(df_az.Az, errors="coerce")
df_az.to_csv(r"~/Desktop/Crimson/az.csv", index=False)

