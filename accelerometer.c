#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define MPU6000_ADDR 0x68
#define MPU6000_REG_PWR_MGMT_1 0x6B
#define MPU6000_REG_ACCEL_XOUT_H 0x3B
#define MPU6000_REG_ACCEL_CONFIG 0x1C

// Inicializa el sensor MPU6000
int mpu6000_init(int i2c_fd) {
    // Despertar el sensor (PWR_MGMT_1 = 0)
    uint8_t buf[2] = {MPU6000_REG_PWR_MGMT_1, 0x00};
    if (write(i2c_fd, buf, 2) != 2) {
        perror("Error al inicializar MPU6000");
        return -1;
    }
    usleep(100000); // Esperar 100ms

    // Configurar acelerómetro a ±2g (ACCEL_CONFIG = 0x00)
    uint8_t accel_cfg[2] = {MPU6000_REG_ACCEL_CONFIG, 0x00};
    if (write(i2c_fd, accel_cfg, 2) != 2) {
        perror("Error al configurar rango de acelerómetro");
        return -1;
    }
    usleep(10000); // Esperar 10ms

    return 0;
}

// Lee los valores de aceleración (X, Y, Z) en g
int mpu6000_read_accel(int i2c_fd, float *ax, float *ay, float *az) {
    uint8_t reg = MPU6000_REG_ACCEL_XOUT_H;
    uint8_t data[6];

    // Seleccionar registro de inicio
    if (write(i2c_fd, &reg, 1) != 1) {
        perror("Error al seleccionar registro de acelerómetro");
        return -1;
    }
    // Leer 6 bytes (XH, XL, YH, YL, ZH, ZL)
    if (read(i2c_fd, data, 6) != 6) {
        perror("Error al leer datos de acelerómetro");
        return -1;
    }
    int16_t raw_ax = (int16_t)((data[0] << 8) | data[1]);
    int16_t raw_ay = (int16_t)((data[2] << 8) | data[3]);
    int16_t raw_az = (int16_t)((data[4] << 8) | data[5]);

    // Conversión a g (sensibilidad ±2g: 16384 LSB/g)
    *ax = raw_ax / 16384.0f;
    *ay = raw_ay / 16384.0f;
    *az = raw_az / 16384.0f;
    return 0;
}

// Abre el bus I2C y selecciona el dispositivo MPU6000
int mpu6000_open(const char *i2c_device) {
    int fd = open(i2c_device, O_RDWR);
    if (fd < 0) {
        perror("No se pudo abrir el bus I2C");
        return -1;
    }
    if (ioctl(fd, I2C_SLAVE, MPU6000_ADDR) < 0) {
        perror("No se pudo seleccionar el dispositivo I2C");
        close(fd);
        return -1;
    }
    return fd;
}

// Cierra el bus I2C
void mpu6000_close(int fd) {
    close(fd);
}
