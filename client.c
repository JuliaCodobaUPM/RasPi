#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "accelerometer.h"
#include "colorimeter.h"

#define SERVER_IP "192.168.0.21"//"192.168.31.138"
#define PORT 12345
#define BUF_SIZE 2048

#define SAMPLES 10

typedef struct {
    float ax, ay, az;
    uint16_t clear, red, green, blue;
} sensor_sample_t;

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    ssize_t recv_len;
    socklen_t addr_len = sizeof(server_addr);

    // Inicializar sensores
    int accel_fd = mpu6000_open("/dev/i2c-1");
    if (accel_fd < 0 || mpu6000_init(accel_fd) < 0) {
        fprintf(stderr, "Error inicializando acelerómetro\n");
        exit(EXIT_FAILURE);
    }
    int color_fd = tcs3472_open("/dev/i2c-1");
    if (color_fd < 0 || tcs3472_init(color_fd) < 0) {
        fprintf(stderr, "Error inicializando colorímetro\n");
        mpu6000_close(accel_fd);
        exit(EXIT_FAILURE);
    }

    // Crear socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        mpu6000_close(accel_fd);
        tcs3472_close(color_fd);
        exit(EXIT_FAILURE);
    }

    // Configurar dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        mpu6000_close(accel_fd);
        tcs3472_close(color_fd);
        exit(EXIT_FAILURE);
    }

    sensor_sample_t samples[SAMPLES];
    int sample_idx = 0;

    while (1) {
        // Tomar muestra de acelerómetro
        if (mpu6000_read_accel(accel_fd, &samples[sample_idx].ax, &samples[sample_idx].ay, &samples[sample_idx].az) < 0) {
            fprintf(stderr, "Error leyendo acelerómetro\n");
        }
        // Tomar muestra de colorímetro
        if (tcs3472_read_colors(color_fd, &samples[sample_idx].clear, &samples[sample_idx].red, &samples[sample_idx].green, &samples[sample_idx].blue) < 0) {
            fprintf(stderr, "Error leyendo colorímetro\n");
        }

        sample_idx++;

        if (sample_idx == SAMPLES) {
            // Formatear datos en JSON simple
            char data[BUF_SIZE];
            int pos = 0;
            pos += snprintf(data + pos, BUF_SIZE - pos, "{ \"samples\": [");
            for (int i = 0; i < SAMPLES; ++i) {
                pos += snprintf(data + pos, BUF_SIZE - pos,
                    "{\"ax\":%.4f,\"ay\":%.4f,\"az\":%.4f,"
                    "\"clear\":%u,\"red\":%u,\"green\":%u,\"blue\":%u}%s",
                    samples[i].ax, samples[i].ay, samples[i].az,
                    samples[i].clear, samples[i].red, samples[i].green, samples[i].blue,
                    (i < SAMPLES - 1) ? "," : "");
            }
            snprintf(data + pos, BUF_SIZE - pos, "] }");

            // Enviar datos al servidor
            if (sendto(sockfd, data, strlen(data), 0,
                       (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("sendto");
            }

            // Esperar respuesta del servidor (opcional)
            recv_len = recvfrom(sockfd, buffer, BUF_SIZE - 1, 0,
                                (struct sockaddr *)&server_addr, &addr_len);
            if (recv_len > 0) {
                buffer[recv_len] = '\0';
                printf("Respuesta del servidor: %s\n", buffer);
            }

            sample_idx = 0; // Reiniciar índice de muestras
        }

        sleep(1); // Esperar 1 segundo antes de la siguiente muestra
    }

    // Cerrar recursos (nunca se alcanza en este bucle infinito)
    close(sockfd);
    mpu6000_close(accel_fd);
    tcs3472_close(color_fd);
    return 0;
}
