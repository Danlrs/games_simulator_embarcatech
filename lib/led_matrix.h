#include "hardware/pio.h"

typedef struct{
    double red;
    double green;
    double blue;
}Led_config;

typedef Led_config RGB;

typedef Led_config Matrix_config[5][5];

uint32_t generate_color(double r, double g, double b);

uint init_matrix(PIO pio);

void draw_matrix(Matrix_config config, PIO pio, uint sm);