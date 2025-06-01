#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define TCS3472_ADDR 0x29
#define TCS3472_COMMAND_BIT 0x80
#define TCS3472_ENABLE 0x00
#define TCS3472_ATIME 0x01
#define TCS3472_CDATAL 0x14

// Inicializa el sensor TCS3472
int tcs3472_init(int i2c_fd) {
    uint8_t enable[2] = {TCS3472_COMMAND_BIT | TCS3472_ENABLE, 0x03}; // Power ON + ADC enable
    uint8_t atime[2] = {TCS3472_COMMAND_BIT | TCS3472_ATIME, 0xFF};   // Integration time (2.4ms)
    if (write(i2c_fd, enable, 2) != 2) {
        perror("Error al habilitar TCS3472");
        return -1;
    }
    usleep(3000); // Esperar 3ms
    if (write(i2c_fd, atime, 2) != 2) {
        perror("Error al configurar ATIME en TCS3472");
        return -1;
    }
    usleep(3000); // Esperar 3ms
    return 0;
}

// Lee los valores de color (rojo, verde, azul)
int tcs3472_read_colors(int i2c_fd, uint16_t *clear, uint16_t *red, uint16_t *green, uint16_t *blue) {
    uint8_t reg = TCS3472_COMMAND_BIT | TCS3472_CDATAL;
    uint8_t data[8];

    // Seleccionar registro de inicio
    if (write(i2c_fd, &reg, 1) != 1) {
        perror("Error al seleccionar registro de colorímetro");
        return -1;
    }
    // Leer 8 bytes (CL, CH, RL, RH, GL, GH, BL, BH)
    if (read(i2c_fd, data, 8) != 8) {
        perror("Error al leer datos de colorímetro");
        return -1;
    }
    // Solo asignar rojo, verde y azul
    *red   = (uint16_t)(data[3] << 8 | data[2]);
    *green = (uint16_t)(data[5] << 8 | data[4]);
    *blue  = (uint16_t)(data[7] << 8 | data[6]);
    // No leer ni usar el valor clear
    // Si el puntero clear no es NULL, se puede poner a cero para evitar valores basura
    if (clear) *clear = 0;

    return 0;
}

// Abre el bus I2C y selecciona el dispositivo TCS3472
int tcs3472_open(const char *i2c_device) {
    int fd = open(i2c_device, O_RDWR);
    if (fd < 0) {
        perror("No se pudo abrir el bus I2C");
        return -1;
    }
    if (ioctl(fd, I2C_SLAVE, TCS3472_ADDR) < 0) {
        perror("No se pudo seleccionar el dispositivo I2C");
        close(fd);
        return -1;
    }
    return fd;
}

// Cierra el bus I2C
void tcs3472_close(int fd) {
    close(fd);
}
