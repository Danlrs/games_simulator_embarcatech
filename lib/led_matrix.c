#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "lib/led_matrix.h"
#include "ws2812.pio.h"

#define MATRIX_PIN 7

uint32_t generate_color(double r, double g, double b) { 
    unsigned char R, G, B;
    R = (unsigned char)(r * 255);
    G = (unsigned char)(g * 255);
    B = (unsigned char)(b * 255);
    
    return (G << 24) | (R << 16) | (B << 8);
}

uint init_matrix(PIO pio){

    stdio_init_all();

    bool clock_config = set_sys_clock_khz(128000, false);

    // Adiciona o programa ao pio às maquinas de estado
    uint offset = pio_add_program(pio, &ws2812_program);
    // Reivindica uma máquina de estado livre para executar o programa PIO
    uint sm = pio_claim_unused_sm(pio, true); 
    // Configura a SM para executar o programa PIO
    ws2812_program_init(pio, sm, offset, MATRIX_PIN, 800000, false); 

    return sm;
}


void draw_matrix(Matrix_config config, PIO pio, uint sm){
    for(int i = 4; i>=0; i--){
        if(i % 2){
            for (int j = 0; j < 5; j ++){
                uint32_t color = generate_color(
                    config[i][j].red,
                    config[i][j].green,
                    config[i][j].blue
                );

                pio_sm_put_blocking(pio, sm, color);
            }
        }else{
            for (int j = 4; j >= 0; j --){
                uint32_t color = generate_color(
                    config[i][j].red,
                    config[i][j].green,
                    config[i][j].blue
                );

                pio_sm_put_blocking(pio, sm, color);
            }
        }
    }
}