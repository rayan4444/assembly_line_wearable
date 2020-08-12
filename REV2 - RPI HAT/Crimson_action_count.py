#!/usr/bin/env python3
import time
import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import find_peaks
from scipy.io import wavfile
from board import SCL, SDA
import busio
from PIL import Image, ImageDraw, ImageFont
import adafruit_ssd1306


def a_count():
    # Create the I2C interface for the screen
    i2c = busio.I2C(SCL, SDA)

    # Create the SSD1306 OLED class.
    disp = adafruit_ssd1306.SSD1306_I2C(128, 32, i2c)

    # Clear display.
    disp.fill(0)
    disp.show()

    # Create blank image for drawing.
    # Make sure to create image with mode '1' for 1-bit color.
    width = disp.width
    height = disp.height
    image = Image.new("1", (width, height))

    # Get drawing object to draw on image.
    draw = ImageDraw.Draw(image)

    # Draw a black filled box to clear the image.
    draw.rectangle((0, 0, width, height), outline=0, fill=0)

    # Draw some shapes.
    # First define some constants to allow easy resizing of shapes.
    padding = -2
    top = padding
    bottom = height - padding
    # Move left to right keeping track of the current x position for drawing shapes.
    x = 0

    # Load nice silkscreen font
    font = ImageFont.truetype("/home/pi/Desktop/Crimson/slkscr.ttf", 18)

    # Draw a black filled box to clear the image.
    draw.rectangle((0, 0, width, height), outline=0, fill=0)

    # Display RECORDING label on screen
    draw.text((x, top + 0), "PROCESSING...", font=font, fill=255)
    disp.image(image)
    disp.show()

    # recording/logging duration (in seconds)
    # This number can be found from the Crimson_thresing_test.py file
    t = 40

    # load data
    ax = np.genfromtxt("/home/pi/Desktop/Crimson/ax.csv", delimiter=",")
    ay = np.genfromtxt("/home/pi/Desktop/Crimson/ay.csv", delimiter=",")
    az = np.genfromtxt("/home/pi/Desktop/Crimson/az.csv", delimiter=",")

    fs, y = wavfile.read("/home/pi/Desktop/Crimson/output.wav")

    # width= , distance=20, prominence= , threshold=
    peaks_ax, _ = find_peaks(ax, prominence=0.75)
    peaks_ay, _ = find_peaks(ay, prominence=0.75)
    peaks_az, _ = find_peaks(az, prominence=0.75)
    peaks_mic, _ = find_peaks(y, height=2, distance=2000)

    # plotting
    fig = plt.figure()

    ax1 = fig.add_subplot(4, 1, 1)

    ax1.set(xticks=(np.arange(0, len(ax), len(ax) / t)), ylabel="Ax")
    ax1.set_xticklabels(np.arange(0, t, 1))

    ax2 = fig.add_subplot(4, 1, 2, sharex=ax1)
    ax2.set(ylabel="Ay")

    ax3 = fig.add_subplot(4, 1, 3, sharex=ax1)
    ax3.set(ylabel="Az")

    ax4 = fig.add_subplot(4, 1, 4)
    ax4.set(xticks=(np.arange(0, len(y), len(y) / t)), ylabel="MIC")
    ax4.set_xticklabels(np.arange(0, t, 1))

    ax1.plot(peaks_ax, ax[peaks_ax], "xr")
    ax1.plot(ax)
    ax2.plot(peaks_ay, ay[peaks_ay], "xr")
    ax2.plot(ay)
    ax3.plot(peaks_az, az[peaks_az], "xr")
    ax3.plot(az)
    ax4.plot(peaks_mic, y[peaks_mic], "xr")
    ax4.plot(y)

    # print(peaks_ax)
    # print(peaks_ay)
    # print(peaks_az)
    # print(peaks_mic)

    # convert peak values from sample number to time
    events_ax = (peaks_ax) * t / len(ax)
    # print(events_ax)
    events_ay = (peaks_ay) * t / len(ay)
    # print(events_ay)
    events_az = (peaks_az) * t / len(az)
    # print(events_az)
    events_mic = peaks_mic * t / len(y)
    # print(events_mic)

    # show plot
    # plt.show()

    # ------------------------------------#
    # signal preprocessing:
    # algo to determine if it's an action:

    # STEP 1:
    # In each signal average out cluster of peaks:
    # Assumption 1: sensor and microphone data for each drilling motion is close to a normal distribution,
    # the center of the motion should be close to the average value of x-coordinates
    # Therefore, to find the center we can calculate the average x-coordinate of a cluster of peaks
    # Assumption 2: there's at least 1 second in between two drilling actions

    # do it for ax
    peak_counter = 1
    avg_peaks = events_ax[0]
    avg_peaks_ax = []

    for i in range(1, len(events_ax)):
        # if two consecutive peaks are less than a second apart, add them up and increase the peak counter
        if (events_ax[i] - events_ax[i - 1]) <= 1:
            avg_peaks = avg_peaks + events_ax[i]
            peak_counter = peak_counter + 1

        # if the gap between two consecutive peaks is larger than 1s or the list of peaks is finished, log the average value of the group of peaks
        if ((events_ax[i] - events_ax[i - 1]) > 1) or (i == (len(events_ax) - 1)):
            # average the average peaks (yea I know it sounds redundant)
            avg_peaks = avg_peaks / peak_counter
            # add the averaged value to the ax valid peaks list
            avg_peaks_ax.append(avg_peaks)
            # put back the first non successive peak into the peak average value
            avg_peaks = events_ax[i]
            # reset peak counter
            peak_counter = 1

    # print(avg_peaks_ax)

    # do it for ay
    avg_peaks = events_ay[0]
    avg_peaks_ay = []

    for i in range(1, len(events_ay)):
        # if two consecutive peaks are less than a second apart, add them up and increase the peak counter
        if (events_ay[i] - events_ay[i - 1]) <= 1:
            avg_peaks = avg_peaks + events_ay[i]
            peak_counter = peak_counter + 1

        # if the gap between two consecutive peaks is larger than 1s or the list of peaks is finished, log the average value of the group of peaks
        if ((events_ay[i] - events_ay[i - 1]) > 1) or (i == (len(events_ay) - 1)):
            # average the average peaks (yea I know it sounds redundant)
            avg_peaks = avg_peaks / peak_counter
            # add the averaged value to the ax valid peaks list
            avg_peaks_ay.append(avg_peaks)
            # put back the first non successive peak into the peak average value
            avg_peaks = events_ay[i]
            # reset peak counter
            peak_counter = 1

    # print(avg_peaks_ay)

    # do it for az
    peak_counter = 1
    avg_peaks = events_az[0]
    avg_peaks_az = []

    for i in range(1, len(events_az)):
        # if two consecutive peaks are less than half a second apart, add them up and increase the peak counter
        if (events_az[i] - events_az[i - 1]) <= 1:
            avg_peaks = avg_peaks + events_az[i]
            peak_counter = peak_counter + 1

        # if the gap between two consecutive peaks is larger than 1s or the list of peaks is finished, log the average value of the group of peaks
        if ((events_az[i] - events_az[i - 1]) > 1) or (i == (len(events_az) - 1)):
            # average the average peaks (yea I know it sounds redundant)
            avg_peaks = avg_peaks / peak_counter
            # add the averaged value to the ax valid peaks list
            avg_peaks_az.append(avg_peaks)
            # put back the first non successive peak into the peak average value
            avg_peaks = events_az[i]
            # reset peak counter
            peak_counter = 1

    # print(avg_peaks_az)

    # do it for microphone
    peak_counter = 1
    avg_peaks = events_mic[0]
    avg_peaks_mic = []

    for i in range(1, len(events_mic)):
        # if two consecutive peaks are less than a half a second apart, add them up and increase the peak counter
        if (events_mic[i] - events_mic[i - 1]) <= 0.5:
            avg_peaks = avg_peaks + events_mic[i]
            peak_counter = peak_counter + 1

        # if the gap between two consecutive peaks is larger than 1s or the list of peaks is finished, log the average value of the group of peaks
        if ((events_mic[i] - events_mic[i - 1]) > 0.5) or (i == (len(events_mic) - 1)):
            # average the average peaks (yea I know it sounds redundant)
            avg_peaks = avg_peaks / peak_counter
            # add the averaged value to the ax valid peaks list
            avg_peaks_mic.append(avg_peaks)
            # put back the first non successive peak into the peak average value
            avg_peaks = events_mic[i]
            # reset peak counter
            peak_counter = 1

    # print(avg_peaks_mic)

    # STEP 2: eliminate false positives from accelerometer data.
    # Assuming the worker doesn't move much between two overhead drilling actions, accelerometer peaks
    # represent the vibration from the drill/tool: all axes of the accelerometer should detect some vibration
    # To eliminate noise peaks from the averaging phase in step 1, we only consider an average peak valid if
    # it is detected by at least 2 sensors.
    # we consider average peaks within 1s of each other to belong to the same motion

    # container for the loop
    valid_peaks = []
    avg_selected = 0
    d1 = 0
    d2 = 0

    for i in range(len(avg_peaks_ax)):
        for j in range(len(avg_peaks_ay)):
            d1 = 0
            d = avg_peaks_ax[i] - avg_peaks_ay[j]
            if abs(d) <= 1:
                d1 = 1
            for k in range(len(avg_peaks_az)):
                d2 = 0
                # compare elements in peaks set
                e = avg_peaks_ax[i] - avg_peaks_az[k]
                # the peaks are considered to belong to the same action if they are less than half a second apart
                if (abs(e) <= 0.5) or (abs(d - e) <= 0.5):
                    d2 = 1
                if (d1 == 1) and (d2 == 1):
                    avg_selected = (
                        avg_peaks_ax[i] + avg_peaks_ay[j] + avg_peaks_az[k]
                    ) / 3
                    # add this value to a list of validated peaks
                    valid_peaks.append(avg_selected)

    # print(valid_peaks)

    # STEP 3:
    # Compare accelerometer peaks to microphone peaks
    # one action should have matching microphone and accelerometer peaks
    # Assumption: peaks are considered matching if they are within half a second from each other
    x = 0
    actions_count = 0

    for i in range(len(valid_peaks)):
        for j in range(len(avg_peaks_mic)):
            x = abs(valid_peaks[i] - avg_peaks_mic[j])
            if x <= 0.5:
                actions_count = actions_count + 1

    print("Number of tasks completed: ", actions_count)

    # Load nice silkscreen font
    font = ImageFont.truetype("/home/pi/Desktop/Crimson/slkscr.ttf", 18)

    # Draw a black filled box to clear the image.
    draw.rectangle((0, 0, width, height), outline=0, fill=0)

    # Display RECORDING label on screen
    draw.text((x, top + 0), "actions: " + str(actions_count), font=font, fill=255)
    disp.image(image)
    disp.show()
    time.sleep(5)


if __name__ == "__main__":
    a_count()

