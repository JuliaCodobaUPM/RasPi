#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <stdint.h>

// Abre el bus I2C y selecciona el dispositivo MPU6000
int mpu6000_open(const char *i2c_device);

// Inicializa el sensor MPU6000
int mpu6000_init(int i2c_fd);

// Lee los valores de aceleración (X, Y, Z) en g
int mpu6000_read_accel(int i2c_fd, float *ax, float *ay, float *az);

// Cierra el bus I2C
void mpu6000_close(int fd);

#endif // ACCELEROMETER_H
