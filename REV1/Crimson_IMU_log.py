# importing packages
import time
from datetime import datetime
import csv
from icm20948 import ICM20948


# print warning
print(
    """Crimson_IMU_log.py
Logs 30 seconds of IMU data in a .csv file. 
Press Ctrl+C to exit!
"""
)

# create IMU object: this library only added I2C support, by default reads 0x68, I changed it to read 0x69
imu = ICM20948()

# create csv file to log sensor data
with open("/home/pi/Desktop/Crimson/sensor_log.csv", "a", newline="") as log:
    # create an instance of csv writer
    csv_write = csv.writer(log)

    # write header file
    csv_write.writerow(
        ["Index", "Timestamp", "Ax", "Ay", "Az", "Gx", "Gy", "Gz", "Mx", "My", "Mz"]
    )

    # create index counter
    index = 0

    try:

        while True:

            # read IMU data
            mx, my, mz = imu.read_magnetometer_data()
            ax, ay, az, gx, gy, gz = imu.read_accelerometer_gyro_data()

            # increment index counter
            index = index + 1

            # save IMU data in a row
            log_row = [
                index,
                datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
                ax,
                ay,
                az,
                gx,
                gy,
                gz,
                mx,
                my,
                mz,
            ]

            # write row to csv file
            csv_write.writerow(log_row)

            # print values out to terminal
            print(
                """
        Accel: {:05.2f} {:05.2f} {:05.2f}
        Gyro:  {:05.2f} {:05.2f} {:05.2f}
        Mag:   {:05.2f} {:05.2f} {:05.2f}""".format(
                    ax, ay, az, gx, gy, gz, mx, my, mz
                )
            )

            time.sleep(0.01)
    finally:
        # close the csv file
        log.close()
        print("logging session finished")

