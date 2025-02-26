#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/led_matrix.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "pico/bootrom.h"
#include "hardware/timer.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

// Definições de hardware para comunicação e controle
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// Definições dos pinos do joystick e botões
#define JOYSTICK_X 26 
#define JOYSTICK_Y 27  
#define JOYSTICK_BUTTON 22 
#define A_BUTTON 5 
#define B_BUTTON 6

// Definição dos pinos da matriz de LEDs e LEDs RGB
#define LED_MATRIX 7
#define RED_LED 13
#define BLUE_LED 12
#define GREEN_LED 11

// Parâmetros para controle de ADC
#define ADC_THRESHOLD 3000 // Ajuste conforme necessário

// Variáveis para controle do PIO
PIO pio;
uint sm;

// Variáveis globais para controle do jogo
int selector = 0; // Controla a seleção de jogos
int selected_game = 3; // Indica o jogo selecionado
bool game_over = false; // Indica se o jogo acabou

// Variáveis para o jogo da velha
int turn = 0; // Indica o jogador atual
bool show_turn = false;  // Indica de qual jogador é a vez
bool game_started = false;  // Indica se o jogo já começou
int winner = 0; // Indica o vencedor
int score_p1 = 0; // Pontuação do jogador 1
int score_p2 = 0; // Pontuação do jogador 2
int cursor_x = 2; // Posição inicial do cursor na matriz
int cursor_y = 2; // Posição inicial do cursor na matriz

// Variáveis para o jogo Flappy Bird
int bird_y = 2; // Posição vertical do pássaro
int pipe_x = 4; // Posição do obstáculo (cano)
int pipe_gap_y; // Altura do espaço entre os canos
int score_fb = 0; // Pontuação do Flappy Bird
int highscore_fb = 0; // Recorde do jogo
int bird_color = 0; // Cor do pássaro


static volatile uint32_t last_event = 0; // Último evento de botão pressionado

double board[5][5] = {0};   // Tabuleiro do jogo

Matrix_config rgb_board = {0};  // Matriz que representa o jogo na matriz de LEDs

// Limpa a matriz de LEDs
void clear_matrix(){
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            rgb_board[i][j].red = 0;
            rgb_board[i][j].green = 0;
            rgb_board[i][j].blue = 0;
        }
    }
    draw_matrix(rgb_board, pio, sm);
}

// Limpa o tabuleiro
void clear_board(){
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            board[i][j] = 0;
        }
    }
}

// Inicializa o tabuleiro do jogo da velha com a grade
void init_board_tic_tac_toe(){
    double temp_board[5][5] = {
        {0, 1, 0, 1, 0},
        {1, 1, 1, 1, 1},
        {0, 1, 0, 1, 0},
        {1, 1, 1, 1, 1},
        {0, 1, 0, 1, 0}};
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            board[i][j] = temp_board[i][j];
        }
    }
}

// Atualiza a posição do cursor no jogo da velha conforme o joystick
void update_cursor_position_tic_tac_toe() {
    adc_select_input(0);
    uint16_t adc_x = adc_read();
    adc_select_input(1);
    uint16_t adc_y = adc_read();

    if (adc_x > 3600 && cursor_x > 0) cursor_x -= 2; // Cima
    else if (adc_x < 1200 && cursor_x < 4) cursor_x += 2; // Baixo

    if (adc_y > 3600 && cursor_y < 4) cursor_y += 2; // Direita
    else if (adc_y < 1200 && cursor_y > 0) cursor_y -= 2; // Esquerda
}

// Atualiza a posição do seletor de jogos conforme o joystick
void update_selector_position(){
    adc_select_input(0);
    uint16_t adc_x = adc_read();

    if (adc_x > 3600 && selector > 0) selector--; // Cima 
    else if (adc_x < 1200 && selector < 1) selector++; // Baixo 
}   

// Atualiza a matriz de LEDs para representar o estado atual do jogo da velha
void update_led_matrix_tic_tac_toe() {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if(board[i][j] == 4){ // Linha vencedora
                rgb_board[i][j].red = 0;
                rgb_board[i][j].green = 0.2;
                rgb_board[i][j].blue = 0;
            }else if (i == cursor_x && j == cursor_y && turn == 0) {
                if(board[i][j] == 0){
                    rgb_board[i][j].red = 0.2; // Destaque para o cursor do jogador 1
                    rgb_board[i][j].green = 0.2;
                    rgb_board[i][j].blue = 0;
                }else{
                    rgb_board[i][j].red = 0.2; // Destaque para o cursor do jogador 1
                    rgb_board[i][j].green = 0;
                    rgb_board[i][j].blue = 0;
                }
                
            } else if(i == cursor_x && j == cursor_y && turn == 1){
                if(board[i][j] == 0){
                    rgb_board[i][j].red = 0.2; // Destaque para o cursor do jogador 2
                    rgb_board[i][j].green = 0;
                    rgb_board[i][j].blue = 0.2;
                }else{
                    rgb_board[i][j].red = 0.2; // Destaque para o cursor do jogador 2
                    rgb_board[i][j].green = 0;
                    rgb_board[i][j].blue = 0;
                }
            }else if (board[i][j] == 0) {
                rgb_board[i][j].red = 0;
                rgb_board[i][j].green = 0;
                rgb_board[i][j].blue = 0;
            } else if (board[i][j] == 1) {
                rgb_board[i][j].red = 0.005;
                rgb_board[i][j].green = 0.005;
                rgb_board[i][j].blue = 0.005;
            } else if (board[i][j] == 2) { // Jogada do jogador 1
                rgb_board[i][j].red = 0.05;
                rgb_board[i][j].green = 0.05;
                rgb_board[i][j].blue = 0;
            } else if (board[i][j] == 3) { // Jogada do jogador 2
                rgb_board[i][j].red = 0.05;
                rgb_board[i][j].green = 0;
                rgb_board[i][j].blue = 0.05;
            }
        }
    }
    draw_matrix(rgb_board, pio, sm);
}

// Verifica se houve um vencedor no jogo da velha ou se houve um empate
int verify_win_tic_tac_toe(){
    bool temp = false;
    for(int i = 0; i < 3; i++){
        if(board[i*2][0] == board[i*2][2] && board[i*2][2] == board[i*2][4] && board[i*2][0] != 0){
            if (board[i*2][0] == 2){
                winner = 1;
                score_p1++;
            }
            if(board[i*2][0] == 3){
                winner = 2;
                score_p2++;
            }
            board[i*2][0] = 4;
            board[i*2][2] = 4;
            board[i*2][4] = 4;
        }
        if(board[0][i*2] == board[2][i*2] && board[2][i*2] == board[4][i*2] && board[0][i*2] != 0){
            if(board[0][i*2] == 2){
                winner = 1;
                score_p1++;
            }
            if((board[0][i*2] == 3)){
                winner = 2;
                score_p2++;
            }
            board[0][i*2] = 4;
            board[2][i*2] = 4;
            board[4][i*2] = 4;
        }
        if(board[0][0] == board[2][2] && board[2][2] == board[4][4] && board[0][0] != 0){
            if(board[0][0] == 2){
                winner = 1;
                score_p1++;
            }
            if(board[0][0] == 3){
                winner = 2;
                score_p2++;
            }
            board[0][0] = 4;
            board[2][2] = 4;
            board[4][4] = 4;
        }
        if(board[0][4] == board[2][2] && board[2][2] == board[4][0] && board[0][4] != 0){
            if(board[0][4] == 2){
                winner = 1;
                score_p1++;
            }
            if(board[0][4] == 3){
                winner = 2;
                score_p2++;
            }
            board[0][4] = 4;
            board[2][2] = 4;
            board[4][0] = 4;
        }
    }
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            if(board[i][j] == 0){
                temp = true;
            }
        }
    }
    if(temp == false && winner == 0){
        winner = 3;
    }
    return winner;
}

// Inicializa o jogo Flappy Bird
void init_flappy_bird(){
    bird_y = 2;
    pipe_x = 4;
    pipe_gap_y = rand() % 3;
    score_fb = 0;
}

// Atualiza o tabuleiro do Flappy Bird
void update_flappy_bird_board(){
    clear_board();

    board[bird_y][0] = 2;

    for(int i = 0; i < 5; i++){
        if(i < pipe_gap_y || i >= pipe_gap_y + 2){
            board[i][pipe_x] = 1;
        }
    }

    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            printf("%d ", board[i][j]);
        }
        printf("\n");
    }
}

//Atualiza a matriz de LEDs para representar o estado atual do Flappy Bird
void update_led_matrix_flappy_bird(){
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            if(board[i][j] == 0){
                rgb_board[i][j].red = 0;
                rgb_board[i][j].green = 0;
                rgb_board[i][j].blue = 0;
            }else if(board[i][j] == 1){
                rgb_board[i][j].red = 0;
                rgb_board[i][j].green = 0.05;
                rgb_board[i][j].blue = 0;
            }else if(board[i][j] == 2){
                if(bird_color == 0){
                    rgb_board[i][j].red = 0.05;
                    rgb_board[i][j].green = 0.05;
                    rgb_board[i][j].blue = 0;
                }else if(bird_color == 1){
                    rgb_board[i][j].red = 0;
                    rgb_board[i][j].green = 0;
                    rgb_board[i][j].blue = 0.05;
                }else if(bird_color == 2){
                    rgb_board[i][j].red = 0.05;
                    rgb_board[i][j].green = 0.02;
                    rgb_board[i][j].blue = 0;
                }
            }else if(board[i][j] == 3){
                rgb_board[i][j].red = 0.05;
                rgb_board[i][j].green = 0;
                rgb_board[i][j].blue = 0;
            }
        }
    }
    draw_matrix(rgb_board, pio, sm);
}

// Função de callback para tratamento de eventos dos botões
void button_pressed(uint gpio, uint32_t events){
    printf("Botão pressionado: GPIO %d\n", gpio);
    uint32_t current_time = to_us_since_boot(get_absolute_time());
  
    // Debounce para evitar múltiplas leituras em curto intervalo
    if(current_time - last_event > 300000){
        last_event = current_time;
  
        // Verifica qual botão foi pressionado e executa a ação correspondente
        if(gpio == JOYSTICK_BUTTON){
            if(selected_game == 0){
                show_turn = !show_turn;
                gpio_put(RED_LED, 0);
                gpio_put(GREEN_LED, 0);
                gpio_put(BLUE_LED, 0);
            }else if(selected_game == 1){
                bird_color = (bird_color + 1) % 3;
            }
        }else if(gpio == B_BUTTON){
            if(selected_game == 0){
                if(!game_started){
                    turn = 1 - turn;
                }
            }
            if(game_over){
                selected_game = 3;
                game_over = false;
            }
          //reset_usb_boot(0, 0); // Reinicia no modo USB Boot
        }else if(gpio == A_BUTTON){
            if(selected_game == 0){
                if(turn == 0 && board[cursor_x][cursor_y] == 0){
                    board[cursor_x][cursor_y] = 2;
                    turn = 1 - turn;
                }else if(turn == 1 && board[cursor_x][cursor_y] == 0){
                    board[cursor_x][cursor_y] = 3;
                    turn = 1 - turn;
                }
                if(winner == 0){
                    game_started = true;
                }else{
                    winner = 0;
                    cursor_x = 2;
                    cursor_y = 2;
                    game_started = false;
                    game_over = false;
                }
            }else if(selected_game == 1){
                if(!game_over){
                    bird_y-=2;
                }else if(game_over){
                    game_over = false;
                    init_flappy_bird();
                }
            }
            if(selector == 0 && selected_game == 3){
                selected_game = 0;
                //clear_matrix();
                winner = 0;
                cursor_x = 2;
                cursor_y = 2;
                game_started = false;
                game_over = false;
            }else if(selector == 1 && selected_game == 3){
                selected_game = 1;
            }
        }
    }
}

// Configuração dos pinos dos LEDs como saída
void init_rgb(){
    gpio_init(RED_LED);
    gpio_set_dir(RED_LED, GPIO_OUT);

    gpio_init(GREEN_LED);
    gpio_set_dir(GREEN_LED, GPIO_OUT);

    gpio_init(BLUE_LED);
    gpio_set_dir(BLUE_LED, GPIO_OUT);
}

// Configuração dos botões como entrada com pull-up interno ativado
void init_buttons(){
    gpio_init(JOYSTICK_BUTTON);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON);

    gpio_init(A_BUTTON);
    gpio_set_dir(A_BUTTON, GPIO_IN);
    gpio_pull_up(A_BUTTON);

    gpio_init(B_BUTTON);
    gpio_set_dir(B_BUTTON, GPIO_IN);
    gpio_pull_up(B_BUTTON);

    // Configuração da interrupção com callback para botão A
    gpio_set_irq_enabled_with_callback(A_BUTTON, GPIO_IRQ_EDGE_FALL, true, &button_pressed); 

    // Configuração da interrupção com callback para botão B
    gpio_set_irq_enabled_with_callback(B_BUTTON, GPIO_IRQ_EDGE_FALL, true, &button_pressed);

    // Configuração da interrupção com callback para botão do joystick
    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON, GPIO_IRQ_EDGE_FALL, true, &button_pressed);
}



int main()
{
    stdio_init_all();

    init_rgb();

    init_buttons();

    pio = pio0;
    sm = init_matrix(pio);

    // Inicialização do barramento I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicialização do display OLED
    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    // Limpa o display
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicialização do ADC para leitura do joystick
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    uint16_t adc_value_x;
    uint16_t adc_value_y;

    char score_p1_str[20];
    char score_p2_str[20];
    char score_fb_str[20];
    char tic_tac_toe_score_str[20];
    char highscore_fb_str[20];

    while (true) {
        if(selected_game == 3){
            clear_matrix();
            ssd1306_fill(&ssd, false);
            ssd1306_draw_string(&ssd, "Games:", 20, 10);
            update_selector_position();
            if(selector == 0){
                ssd1306_draw_string(&ssd, "-Tic Tac Toe", 20, 20);
                ssd1306_draw_string(&ssd, "Flappy Bird", 20, 30);
            }else if(selector == 1){
                ssd1306_draw_string(&ssd, "Tic Tac Toe", 20, 20);
                ssd1306_draw_string(&ssd, "-Flappy Bird", 20, 30);
            }
        }else if(selected_game == 0){
            sprintf(score_p1_str, "Yellow: %d", score_p1);
            sprintf(score_p2_str, "Purple: %d", score_p2);
            sprintf(tic_tac_toe_score_str, "   %d - %d", score_p1, score_p2);
            if(!game_started && winner == 0){
                init_board_tic_tac_toe();
            }
            if(show_turn && turn == 0){
                gpio_put(RED_LED, 1);
                gpio_put(GREEN_LED, 1);
                gpio_put(BLUE_LED, 0);
            }else if(show_turn && turn == 1){
                gpio_put(RED_LED, 1);
                gpio_put(GREEN_LED, 0);
                gpio_put(BLUE_LED, 1);
            }
            winner = verify_win_tic_tac_toe();
            if(winner == 0){
                ssd1306_fill(&ssd, false);
                update_cursor_position_tic_tac_toe();
                update_led_matrix_tic_tac_toe();
                ssd1306_draw_string(&ssd, score_p1_str, 10, 20);
                ssd1306_draw_string(&ssd, score_p2_str, 10, 40);
            }else{
                update_led_matrix_tic_tac_toe();
                ssd1306_fill(&ssd, false);
                if(winner == 1){
                    ssd1306_draw_string(&ssd, "Yellow Wins!", 20, 10 );
                    ssd1306_draw_string(&ssd, tic_tac_toe_score_str, 20, 25);
                    ssd1306_draw_string(&ssd, "A: Play Again", 20, 40);
                    ssd1306_draw_string(&ssd, "B: Menu", 20, 50); 
                    game_over = true;
                }
                if(winner == 2){
                    ssd1306_draw_string(&ssd, "Purple Wins!", 20, 10 );
                    ssd1306_draw_string(&ssd, tic_tac_toe_score_str, 20, 25);
                    ssd1306_draw_string(&ssd, "A: Play Again", 20, 40);
                    ssd1306_draw_string(&ssd, "B: Menu", 20, 50);                    
                    game_over = true;
                }
                if(winner == 3){
                    ssd1306_draw_string(&ssd, "It's a Draw!", 20, 10 );
                    ssd1306_draw_string(&ssd, tic_tac_toe_score_str, 20, 25);
                    ssd1306_draw_string(&ssd, "A: Play Again", 20, 40);
                    ssd1306_draw_string(&ssd, "B: Menu", 20, 50); 
                    game_over = true;
                }
                
            }
        }else if(selected_game == 1){
            ssd1306_fill(&ssd, false);
            sprintf(highscore_fb_str, "Highscore: %d", highscore_fb);
            sprintf(score_fb_str, "Score: %d", score_fb);
            ssd1306_draw_string(&ssd, highscore_fb_str, 20, 10);
            ssd1306_draw_string(&ssd, score_fb_str, 20, 30);
            if(!game_over){
                bird_y++;

                pipe_x--;

                if(pipe_x < 0){
                    pipe_x = 4;
                    pipe_gap_y = rand() % 3;
                    score_fb++;
                    if(score_fb > highscore_fb){
                        highscore_fb = score_fb;
                    }
                }

                update_flappy_bird_board();
                update_led_matrix_flappy_bird();

                if(bird_y < 0 || bird_y > 4 || board[bird_y][0] == 1){
                    game_over = true;
                    board[bird_y][0] = 3;
                    update_led_matrix_flappy_bird();
                }
                sleep_ms(200);
            }else{
                update_led_matrix_flappy_bird();
                ssd1306_fill(&ssd, false);
                ssd1306_draw_string(&ssd, "Game Over!", 20, 10);
                ssd1306_draw_string(&ssd, highscore_fb_str, 20, 20);
                ssd1306_draw_string(&ssd, score_fb_str, 20, 30);
                ssd1306_draw_string(&ssd, "A: Play Again", 20, 40);
                ssd1306_draw_string(&ssd, "B: Menu", 20, 50); 
            }
        }
        ssd1306_send_data(&ssd);
        sleep_ms(200); // Pequeno delay para evitar múltiplas leituras rápidas
    }
}
