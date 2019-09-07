# importing packages
import time
from datetime import datetime
import csv
from icm20948 import ICM20948
import pandas as pd

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
    t=0

    # try:

    while t<=10:

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
    #     print(
    #         """
    # Accel: {:05.2f} {:05.2f} {:05.2f}
    # Gyro:  {:05.2f} {:05.2f} {:05.2f}
    # Mag:   {:05.2f} {:05.2f} {:05.2f}""".format(
    #             ax, ay, az, gx, gy, gz, mx, my, mz
    #         )
    #     )
        t=index*0.05
        print(t)
        time.sleep(0.01)
# finally:
    # close the csv file
    log.close()
    print("logging session finished")

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
