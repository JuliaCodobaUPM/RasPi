#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#define PORT 12345
#define BUF_SIZE 4096

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(client_addr);
    ssize_t recv_len;

    // Crear socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Configurar dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Enlazar el socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor esperando datos...\n");

    while (1) {
        recv_len = recvfrom(sockfd, buffer, BUF_SIZE - 1, 0,
                            (struct sockaddr *)&client_addr, &addr_len);
        if (recv_len < 0) {
            perror("recvfrom");
            continue;
        }
        buffer[recv_len] = '\0';

        // Parseo simple del JSON para extraer los datos
        char *samples_start = strstr(buffer, "[");
        char *samples_end = strstr(buffer, "]");
        if (samples_start && samples_end && samples_end > samples_start) {
            float ax_arr[100], ay_arr[100], az_arr[100];
            unsigned int r_arr[100], g_arr[100], b_arr[100];
            int n = 0;
            char *ptr = samples_start + 1;
            while (ptr < samples_end && n < 100) {
                float ax = 0, ay = 0, az = 0;
                unsigned int r = 0, g = 0, b = 0;
                sscanf(ptr,
                    "{\"ax\":%f,\"ay\":%f,\"az\":%f,\"clear\":%*u,\"red\":%u,\"green\":%u,\"blue\":%u}",
                    &ax, &ay, &az, &r, &g, &b);
                ax_arr[n] = ax;
                ay_arr[n] = ay;
                az_arr[n] = az;
                r_arr[n] = r;
                g_arr[n] = g;
                b_arr[n] = b;
                n++;
                ptr = strchr(ptr, '}');
                if (!ptr || ptr >= samples_end) break;
                ptr++;
                while (*ptr == ',' || *ptr == ' ' || *ptr == '\n') ptr++;
            }

            if (n > 0) {
                // Imprimir los 10 valores recibidos
                printf("VALORES RECIBIDOS:\n");
                for (int i = 0; i < n; ++i) {
                    printf("  ACCEL => X: %.4f | Y: %.4f | Z: %.4f\n", ax_arr[i], ay_arr[i], az_arr[i]);
                    printf("  COLOR => R: %u | G: %u | B: %u\n", r_arr[i], g_arr[i], b_arr[i]);
                }

                // ACCEL
                double sum_x = 0, sum_y = 0, sum_z = 0;
                float max_x = ax_arr[0], min_x = ax_arr[0];
                float max_y = ay_arr[0], min_y = ay_arr[0];
                float max_z = az_arr[0], min_z = az_arr[0];
                for (int i = 0; i < n; ++i) {
                    sum_x += ax_arr[i];
                    sum_y += ay_arr[i];
                    sum_z += az_arr[i];
                    if (ax_arr[i] > max_x) max_x = ax_arr[i];
                    if (ax_arr[i] < min_x) min_x = ax_arr[i];
                    if (ay_arr[i] > max_y) max_y = ay_arr[i];
                    if (ay_arr[i] < min_y) min_y = ay_arr[i];
                    if (az_arr[i] > max_z) max_z = az_arr[i];
                    if (az_arr[i] < min_z) min_z = az_arr[i];
                }
                double mean_x = sum_x / n;
                double mean_y = sum_y / n;
                double mean_z = sum_z / n;

                double std_x = 0, std_y = 0, std_z = 0;
                for (int i = 0; i < n; ++i) {
                    std_x += (ax_arr[i] - mean_x) * (ax_arr[i] - mean_x);
                    std_y += (ay_arr[i] - mean_y) * (ay_arr[i] - mean_y);
                    std_z += (az_arr[i] - mean_z) * (az_arr[i] - mean_z);
                }
                std_x = sqrt(std_x / n);
                std_y = sqrt(std_y / n);
                std_z = sqrt(std_z / n);

                // COLOR
                double sum_r = 0, sum_g = 0, sum_b = 0;
                unsigned int max_r = r_arr[0], min_r = r_arr[0];
                unsigned int max_g = g_arr[0], min_g = g_arr[0];
                unsigned int max_b = b_arr[0], min_b = b_arr[0];
                for (int i = 0; i < n; ++i) {
                    sum_r += r_arr[i];
                    sum_g += g_arr[i];
                    sum_b += b_arr[i];
                    if (r_arr[i] > max_r) max_r = r_arr[i];
                    if (r_arr[i] < min_r) min_r = r_arr[i];
                    if (g_arr[i] > max_g) max_g = g_arr[i];
                    if (g_arr[i] < min_g) min_g = g_arr[i];
                    if (b_arr[i] > max_b) max_b = b_arr[i];
                    if (b_arr[i] < min_b) min_b = b_arr[i];
                }
                double mean_r = sum_r / n;
                double mean_g = sum_g / n;
                double mean_b = sum_b / n;

                double std_r = 0, std_g = 0, std_b = 0;
                for (int i = 0; i < n; ++i) {
                    std_r += (r_arr[i] - mean_r) * (r_arr[i] - mean_r);
                    std_g += (g_arr[i] - mean_g) * (g_arr[i] - mean_g);
                    std_b += (b_arr[i] - mean_b) * (b_arr[i] - mean_b);
                }
                std_r = sqrt(std_r / n);
                std_g = sqrt(std_g / n);
                std_b = sqrt(std_b / n);

                printf("MEASURES:\n");
                printf("  ACCEL =>\n");
                printf("    media => X: %.4f | Y: %.4f | Z: %.4f\n", mean_x, mean_y, mean_z);
                printf("    máximo => X: %.4f | Y: %.4f | Z: %.4f\n", max_x, max_y, max_z);
                printf("    mínimo => X: %.4f | Y: %.4f | Z: %.4f\n", min_x, min_y, min_z);
                printf("    desviación típica => X: %.4f | Y: %.4f | Z: %.4f\n", std_x, std_y, std_z);
                printf("  COLOR =>\n");
                printf("    media => R: %.2f | G: %.2f | B: %.2f\n", mean_r, mean_g, mean_b);
                printf("    máximo => R: %u | G: %u | B: %u\n", max_r, max_g, max_b);
                printf("    mínimo => R: %u | G: %u | B: %u\n", min_r, min_g, min_b);
                printf("    desviación típica => R: %.2f | G: %.2f | B: %.2f\n", std_r, std_g, std_b);
            }
        } else {
            printf("Datos recibidos (sin formato esperado):\n%s\n", buffer);
        }

        // Enviar acknowledgement
        const char *ack = "acknowledgement";
        sendto(sockfd, ack, strlen(ack), 0,
               (struct sockaddr *)&client_addr, addr_len);
    }

    // Cerrar el socket (nunca se alcanza)
    close(sockfd);
    return 0;
}
