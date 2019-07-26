# reference code: https://www.thepoorengineer.com/en/arduino-python-plot/

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import struct

from threading import Thread
import serial
import time
import collections


class serialPlot:
    def __init__(
        self, serialPort="COM11", serialBaud=115200, plotLength=100, dataNumBytes=2
    ):
        self.port = serialPort
        self.baud = serialBaud
        self.plotMaxLength = plotLength
        self.dataNumBytes = dataNumBytes
        self.rawData = bytearray(dataNumBytes)
        self.data = collections.deque([0] * plotLength, maxlen=plotLength)
        self.isRun = True
        self.isReceiving = False
        self.thread = None
        self.plotTimer = 0
        self.previousTimer = 0
        # self.csvData= []

        print(
            "Trying to connect to: "
            + str(serialPort)
            + "at"
            + str(serialBaud)
            + "BAUD."
        )
        try:
            self.serialConneciton = serial.Serial(
                port=serialPort, baudrate=serialBaud, timeout=4
            )

            print("Connected to" + str(serialPort) + "at" + str(serialBaud) + "BAUD.")
        except:
            print("Failed to connect")

    def readSerialStart(self):
        if self.thread == None:
            self.thread = Thread(target=self.backgroundThread)
            self.thread.start()
            # Block till we start receiving values
            while self.isReceiving != True:
                time.sleep(0.1)

    def getSerialData(self, frame, lines, lineValueText, lineLabel, timeText):
        currentTimer = time.perf_counter()
        self.plotTimer = int((currentTimer - self.previousTimer) * 1000)
        self.previousTimer = currentTimer
        timeText.set_text("Plot Interval = " + str(self.plotTimer) + "ms")
        value, = struct.unpack(
            "h", self.rawData
        )  # use 'f' for a float (or arduino double)
        self.data.append(value)  # get the latest data point and append it to our array
        lines.set_data(range(self.plotMaxLength), self.data)
        lineValueText.set_text("[" + lineLabel + "] = " + str(value))
        # self.csvDara.append(self.data[-1])

    def backgroundThread(self):
        time.sleep(1.0)
        self.serialConneciton.reset_input_buffer()
        while self.isRun:
            self.serialConneciton.readinto(self.rawData)
            self.isReceiving = True
            #print(self.rawData)

    def close(self):
        self.isRun = False
        self.thread.join()
        self.serialConneciton.close()
        print("Disconnected...")
        # df = pd.DataFrame(self.csvData)
        # df.to_csv('/home/rikisenia/Desktop/data.csv')


def main():

    portName= "COM11"
    baudRate = 115200
    maxPlotLength = 100
    dataNumBytes = 2  # use 4 if sending float/double data typer from arduino

    s = serialPlot(portName, baudRate, maxPlotLength, dataNumBytes)
    s.readSerialStart()

    # plotting
    pltInterval = 50  # Period at whihc the plot animation updates [ms]
    xmin = 0
    xmax = maxPlotLength
    ymin = -(1)
    ymax = 1000
    fig = plt.figure()
    ax = plt.axes(xlim=(xmin, xmax), ylim=(float(ymin - (ymax - ymin) / 10), float(ymax + (ymax - ymin) / 10)))
    ax.set_title("arduino Analog Read")
    ax.set_xlabel("time")
    ax.set_ylabel("AnalogRead Value")

    lineLabel = "Potentiometer Value"
    timeText = ax.text(0.5, 0.95, "", transform=ax.transAxes)
    lines = ax.plot([], [], label=lineLabel)[0]
    lineValueText = ax.text(0.50, 0.90, '', transform=ax.transAxes)
    anim = animation.FuncAnimation(
        fig,
        s.getSerialData,
        fargs=(lines, lineValueText, lineLabel, timeText),
        interval=pltInterval,
    )  # fargs has to be a tuple

    plt.legend(loc="upper left")
    plt.show()

    s.close()


if __name__ == "__main__":
    main()

