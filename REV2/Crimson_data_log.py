#!/usr/bin/env python3


from threading import Thread
import time
from datetime import datetime
import csv
from icm20948 import ICM20948
import pandas as pd
import pyaudio
import wave
from board import SCL, SDA
import busio
from PIL import Image, ImageDraw, ImageFont
import adafruit_ssd1306

# declaring a bunch of global variables that will be used accross threads
global p
global t
global t_0
global t_run
t_run = 0

global index

# initialise variables

index = 0

# initialise IMU object
imu = ICM20948()

# audio recording parameters
INDEX = 2
CHUNK = 1024
FORMAT = pyaudio.paInt32
CHANNELS = 1
RATE = 44100
RECORD_SECONDS = 40
WAVE_OUTPUT_FILENAME = "output.wav"

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
draw.text((x, top + 0), "RECORDING... ", font=font, fill=255)
disp.image(image)
disp.show()

# class to handle logging accelerometer data
class accel_recording:
    # When you initialise an instance of class, the ._running flag is set to True
    def __init__(self):
        self._running = True

    # when you terminate the instance of the class, the _running flag is set to False
    def terminate(self):
        self._running = False
        print("logging session finished")

    # What happens after the class is initialised
    def run(self):
        # first create a csv file top to save all the logged data
        with open("/home/pi/Desktop/Crimson/sensor_log.csv", "a", newline="") as log:
            # create an instance of csv writer
            csv_write = csv.writer(log)

            # write header file
            csv_write.writerow(
                ["Index", "Timestamp", "Ax", "Ay", "Az", "Gx", "Gy", "Gz"]
            )

            # start time
            global t_0
            t_0 = time.time()

            while self._running:
                global index
                global t
                global t_run
                # read IMU data
                ax, ay, az, gx, gy, gz = imu.read_accelerometer_gyro_data()

                # increment index counter
                index = index + 1

                # save IMU data in a row
                # right now we don't use gyroscope data. however it can be used later to supplement raising/lowering
                # of the arm detection with an exoskeleton mounted sensor
                log_row = [
                    index,
                    datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
                    ax,
                    ay,
                    az,
                    gx,
                    gy,
                    gz,
                ]

                # write row to csv file
                csv_write.writerow(log_row)
                t = time.time()
                # the sampling timing isn't precise at all but that doens't matter for our application
                # half a second precision is enough to get the results we want. therefore a rough estimate
                # of the loop time is used here to find time elapsed.
                t_run = t - t_0
                time.sleep(0.01)


class mic_recording:
    def __init__(self):
        self._running = True

    def terminate(self):
        self._running = False

    def run(self):
        p = pyaudio.PyAudio()

        stream = p.open(
            format=FORMAT,
            channels=CHANNELS,
            rate=RATE,
            input=True,
            input_device_index=INDEX,
            frames_per_buffer=CHUNK,
        )

        print("* recording")

        frames = []

        for i in range(0, int(RATE / CHUNK * RECORD_SECONDS)):
            data = stream.read(CHUNK, exception_on_overflow=False)
            frames.append(data)

        print("* done recording")

        stream.stop_stream()
        stream.close()
        p.terminate()

        wf = wave.open(WAVE_OUTPUT_FILENAME, "wb")
        wf.setnchannels(CHANNELS)
        wf.setsampwidth(p.get_sample_size(FORMAT))
        wf.setframerate(RATE)
        wf.writeframes(b"".join(frames))
        wf.close()


# create Class
mic_1 = mic_recording()
# Create_Thread
mic_Thread = Thread(target=mic_1.run)
# start Thread
mic_Thread.start()

# the recording and accelerometer logging don't start at the same time because of differences in initialisation processes
# this is not a clean way of doing things but gets the job done realigning them
time.sleep(0.3)

# Create Class
imu_1 = accel_recording()
# Create Thread
imu_Thread = Thread(target=imu_1.run)
# Start Thread
imu_Thread.start()

Exit = False  # Exit flag
while t_run <= 40.0:

    if t_run > 45.0:
        Exit = True  # Exit Program

imu_1.terminate()
mic_1.terminate()

# here you can split the files in ax, ay, az
# import csv file
df = pd.read_csv("sensor_log.csv", header=0)

df_ax = df[["Ax"]].copy()
# print(df_ax.head())
df_ax.Ax = pd.to_numeric(df_ax.Ax, errors="coerce")

df_ax.to_csv(r"~/Desktop/Crimson/ax.csv", index=False)

# ay dataset
df_ay = df[["Ay"]].copy()
# print(df_ay.head())
df_ay.Ay = pd.to_numeric(df_ay.Ay, errors="coerce")
df_ay.to_csv(r"~/Desktop/Crimson/ay.csv", index=False)

df_az = df[["Az"]].copy()
# print(df_az.head())
df_az.Az = pd.to_numeric(df_az.Az, errors="coerce")
df_az.to_csv(r"~/Desktop/Crimson/az.csv", index=False)

font = ImageFont.truetype("/home/pi/Desktop/Crimson/slkscr.ttf", 12)
# Draw a black filled box to clear the image.
draw.rectangle((0, 0, width, height), outline=0, fill=0)

# Display RECORDING label on screen
draw.text((x, top + 8), "Done Recording", font=font, fill=255)
disp.image(image)
disp.show()
time.sleep(2)
