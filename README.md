# Ultimate Frisbee Tracker
It's a wearable ultimate Frisbee data recording device to facilitate frisbee players to monitor their Frisbee data during competition and training. The sensing device will incorporate a small, lightweight enclosure that can be easily attached to the frisbee players. Considering the dynamic nature of Ultimate Frisbee, a Bluetooth Low Energy (BLE) connection will ensure real-time data transfer without impacting the player's performance. The display device, designed for player convenience, will feature a wrist-mounted or clothing-attachable enclosure with intuitive controls.

![Untitled_Artwork](https://github.com/Michelleyukli/514-Final-Project/assets/148395276/cb1edeb7-dad0-4de7-95f7-71f81d545834)


## Sensing Device：
Hardware Setup:

 1.Use ESP32 as the microcontroller for my sensing device.
 2.Attach an MPU6050 accelerometer and gyroscope sensor to ESP32. Attach the sensor securely to the frisbee. (Consider the orientation and placement for accurate readings.) 
 3.Battery: Compact and lightweight with enough capacity for the entire game.
Configure the sensors to provide readings at an appropriate sampling rate for my application.

Data Fusion:
   - Combine accelerometer and gyroscope data using sensor fusion algorithms (e.g., complementary filter, Kalman filter) to obtain a more accurate representation of motion.
   - Calibrate the sensors to reduce errors and improve accuracy.

Movement Tracking:
   - Acceleration Data: Use accelerometer data to track changes in velocity and position. Integrate acceleration to get velocity and double-integrate to get displacement.

Speed Calculation:
   - Combine the linear velocity components from the accelerometer data to get the overall velocity magnitude.
   - Angular Speed: Use the gyroscope data to calculate the rotational speed.

Filtering and Smoothing:
   - Noise Reduction: Implement filtering techniques to reduce noise in the sensor data.
   - Smoothing: Apply smoothing algorithms to obtain more stable readings.

Data Visualization:
   - Graphs/Plots: Visualize the frisbee's movement using graphs or plots to understand speed variations, direction changes, etc.

Testing and Calibration:
   - Field Testing: Test my device in real-world scenarios to validate the accuracy of my tracking system.
   - Calibration: Fine-tune sensor calibration based on field testing results.

Power Management:
   - Efficient Sampling: Optimize the sampling rate and power consumption to extend battery life, especially if using a portable setup.

<img width="885" alt="截屏2024-01-16 下午6 01 32" src="https://github.com/Michelleyukli/514-Final-Project/assets/148395276/4383e754-75d8-4be0-b490-3209336c8d8e">



## Display Device：
   1.Gauge Needle and LEDs: Stepper motor-driven gauge needle to indicate the distance of the frisbee's throwing yards and a LED for visual feedback whether the Frisbee was successfully thrown. 
   2.Button: A simple button for player interaction, allowing them to reset the training. 
   3.Battery: Compact and lightweight with enough capacity for the entire game.
   4.Additional Displays: OLED display for real-time statistics (If needed)
   5.ESP32: Receiving the data from the sensing device, served as a transmitter for data processing.

<img width="1227" alt="截屏2024-01-16 下午6 15 47" src="https://github.com/Michelleyukli/514-Final-Project/assets/148395276/a0dcc847-a023-4a57-823e-2d0661c75ea0">
