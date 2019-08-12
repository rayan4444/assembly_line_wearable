# importing packages
import time
from datetime import datetime
import csv
from icm20948 import ICM20948
import matplotlib.pyplot as plt
import matplotlib.animation as animation

#print warning
print("""read-all.py
Reads all ranges of movement: accelerometer, gyroscope and
compass heading.
Press Ctrl+C to exit!
""")

#create IMU object: this library only added I2C support, by default reads 0x68, I changed it to read 0x69
imu = ICM20948()

#create csv file to log sensor data
#with open ("/home/pi/Desktop/Crimson/sensor_log.csv","a", newline="") as log:
    #create an instance of csv writer
    #csv_write = csv.writer(log)

    #write header file
    #csv_write.writerow(["Index","Timestamp", "Ax", "Ay", "Az", "Gx", "Gy", "Gz","Mx", "My", "Mz"])

    #create index counter
    #index = 0

#set plotting parameters
x_len = 200 #number of points to display
accel_y_range= [-2, 2] #Y bounds for accelerometer plot
gyro_y_range= [-500, 500] #Y bounds for gyro plot
mag_y_range= [-100, 100] #Y bounds for magnetometer plot

#create figure for plotting
fig = plt.figure()
ax1 = fig.add_subplot(3,1,1) #change later to add gyro and mag
ax2= fig.add_subplot(3,1,2)
ax3= fig.add_subplot(3,1,3)


xs= list(range(0,x_len)) #xvalues
axs= [0]*x_len
ays= [0]*x_len
azs= [0]*x_len
gxs= [0]*x_len
gys= [0]*x_len
gzs= [0]*x_len
mxs= [0]*x_len
mys= [0]*x_len
mzs= [0]*x_len

ax1.set_ylim(accel_y_range)
ax2.set_ylim(gyro_y_range)
ax3.set_ylim(mag_y_range)

line_ax, = ax1.plot(xs, axs)# blank line to be animated  later
line_ay, = ax1.plot(xs, ays)
line_az, = ax1.plot(xs, azs)
line_gx, = ax2.plot(xs, gxs)
line_gy, = ax2.plot(xs, gys)
line_gz, = ax2.plot(xs, gzs)
line_mx, = ax3.plot(xs, mxs)
line_my, = ax3.plot(xs, mys)
line_mz, = ax3.plot(xs, mzs)

#plot labels
ax1.set_title('Accelerometer')
ax1.set_xlabel("Samples")
ax1.set_ylabel('Accelerometer readings')

ax2.set_title('Gyroscope')
ax2.set_xlabel("Samples")
ax2.set_ylabel('Gyroscope readings')

ax3.set_title('Magnetometer')
ax3.set_xlabel("Samples")
ax3.set_ylabel('Magnetometer readings')

fig.legend((line_ax,line_ay,line_az),('Ax','Ay','Az'), 'upper right')
fig.legend((line_gx,line_gy,line_gz),('Gx','Gy','Gz'), 'center right')
fig.legend((line_mx,line_my,line_mz),('Mx','My','Mz'), 'lower right')

#function called  periodically from FuncAnimation
def animate(i, axs, ays, azs, gxs, gys, gzs, mxs, mys, mzs):

    #read IMU data
    mx, my, mz = imu.read_magnetometer_data()
    ax, ay, az, gx, gy, gz = imu.read_accelerometer_gyro_data()

    #increment index counter
    #index= index+1

    #save IMU data in a row
    #log_row=[index, datetime.now().strftime("%Y-%m-%d %H:%M:%S"),ax, ay, az, gx, gy, gz, mx, my, mz]

    #write row to csv file
    #csv_write.writerow(log_row)

    #print values out to terminal
    print("""
Accel: {:05.2f} {:05.2f} {:05.2f}
Gyro:  {:05.2f} {:05.2f} {:05.2f}
Mag:   {:05.2f} {:05.2f} {:05.2f}""".format(
        ax, ay, az, gx, gy, gz, mx, my, mz
        ))

    #append values to list for plotting
    axs.append(ax)
    ays.append(ay)
    azs.append(az)
    gxs.append(gx)
    gys.append(gy)
    gzs.append(gz)
    mxs.append(mx)
    mys.append(my)
    mzs.append(mz)

    #limit the y lists to set number of items
    axs=axs[-x_len:]
    ays=ays[-x_len:]
    azs=azs[-x_len:]
    gxs=gxs[-x_len:]
    gys=gys[-x_len:]
    gzs=gzs[-x_len:]
    mxs=mxs[-x_len:]
    mys=mys[-x_len:]
    mzs=mzs[-x_len:]

    #update line with new Y values
    line_ax.set_ydata(axs)
    line_ay.set_ydata(ays)
    line_az.set_ydata(azs)
    line_gx.set_ydata(gxs)
    line_gy.set_ydata(gys)
    line_gz.set_ydata(gzs)
    line_mx.set_ydata(mxs)
    line_my.set_ydata(mys)
    line_mz.set_ydata(mzs)

    #function output
    return line_ax, line_ay, line_az, line_gx, line_gy, line_gz, line_mx, line_my, line_mz

#setup plot to call animation
ani = animation.FuncAnimation(fig,
    animate,
    fargs=(axs, ays, azs, gxs, gys, gzs, mxs, mys, mzs,),
    interval=10,
    blit=True)

plt.show()

