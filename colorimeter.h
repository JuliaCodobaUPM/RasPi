#ifndef COLORIMETER_H
#define COLORIMETER_H

#include <stdint.h>

// Abre el bus I2C y selecciona el dispositivo TCS3472
int tcs3472_open(const char *i2c_device);

// Inicializa el sensor TCS3472
int tcs3472_init(int i2c_fd);

// Lee los valores de color (claro, rojo, verde, azul)
int tcs3472_read_colors(int i2c_fd, uint16_t *clear, uint16_t *red, uint16_t *green, uint16_t *blue);

// Cierra el bus I2C
void tcs3472_close(int fd);

#endif // COLORIMETER_H
