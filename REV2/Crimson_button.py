#!/usr/bin/env python3

import RPi.GPIO as GPIO
import time
import subprocess, os
from board import SCL, SDA
import busio
from PIL import Image, ImageDraw, ImageFont
import adafruit_ssd1306
import Crimson_action_count
import Crimson_data_log

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


GPIO.setmode(GPIO.BCM)

GPIO.setup(17, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(27, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(22, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(5, GPIO.OUT)
GPIO.setup(6, GPIO.OUT)
GPIO.output(5, GPIO.LOW)
GPIO.output(6, GPIO.LOW)

try:
    while True:
        input_state1 = GPIO.input(22)
        input_state2 = GPIO.input(27)
        input_state3 = GPIO.input(17)

        # Draw a black filled box to clear the image.
        draw.rectangle((0, 0, width, height), outline=0, fill=0)

        # Display RECORDING label on screen
        draw.text((x, top + 0), "READY", font=font, fill=255)
        disp.image(image)
        disp.show()

        if input_state1 == False:
            print("Data Log Button pressed")
            GPIO.output(6, GPIO.HIGH)
            time.sleep(0.2)
            # str2 = "/home/pi/Desktop/Crimson/Crimson_data_log.py"
            # subprocess.call([str2])
            # try:
            Crimson_data_log.log()

            # except Exception as e:
            #     print("Exception raised: {}".format(e))
            #     GPIO.output(6, GPIO.LOW)

            #     # Draw a black filled box to clear the image.
            #     draw.rectangle((0, 0, width, height), outline=0, fill=0)
            #     # Display CLEARED label on screen
            #     draw.text((x, top + 0), "Error", font=font, fill=255)
            #     draw.text((x, top + 16), "Try Again", font=font, fill=255)
            #     disp.image(image)
            #     disp.show()
            #     time.sleep(2)

        if input_state2 == False:

            print("Action Count Button Pressed")
            GPIO.output(5, GPIO.HIGH)
            time.sleep(0.2)
            # str1 = "/home/pi/Desktop/Crimson/Crimson_action_count.py"
            # subprocess.call([str1])
            try:
                Crimson_action_count.a_count()
            except Exception as e:
                print("Exception raised: {}".format(e))
                GPIO.output(5, GPIO.LOW)

                # Draw a black filled box to clear the image.
                draw.rectangle((0, 0, width, height), outline=0, fill=0)
                # Display CLEARED label on screen
                draw.text((x, top + 0), "Error", font=font, fill=255)
                draw.text((x, top + 16), "Try Again", font=font, fill=255)
                disp.image(image)
                disp.show()
                time.sleep(2)

        if input_state3 == False:
            print("Clear Data Button pressed")
            time.sleep(0.2)
            try:
                subprocess.call(["rm /home/pi/Desktop/Crimson/output.wav"], shell=True)
                subprocess.call(["rm /home/pi/Desktop/Crimson/ax.csv"], shell=True)
                subprocess.call(["rm /home/pi/Desktop/Crimson/ay.csv"], shell=True)
                subprocess.call(["rm /home/pi/Desktop/Crimson/az.csv"], shell=True)
                subprocess.call(
                    ["rm /home/pi/Desktop/Crimson/sensor_log.csv"], shell=True
                )
                # Draw a black filled box to clear the image.
                draw.rectangle((0, 0, width, height), outline=0, fill=0)

                # Display CLEARED label on screen
                draw.text((x, top + 0), "Cleared", font=font, fill=255)
                disp.image(image)
                disp.show()
                GPIO.output(5, GPIO.LOW)
                GPIO.output(6, GPIO.LOW)
                time.sleep(2)

            except Exception as e:
                print("Exception raised: {}".format(e))
                # Draw a black filled box to clear the image.
                draw.rectangle((0, 0, width, height), outline=0, fill=0)
                # Display CLEARED label on screen
                draw.text((x, top + 0), "Error", font=font, fill=255)
                draw.text((x, top + 10), "Try Again", font=font, fill=255)
                disp.image(image)
                disp.show()
                time.sleep(2)


except KeyboardInterrupt:
    GPIO.cleanup()
