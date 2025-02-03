#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"

//arquivo .pio
#include "pio_matrix.pio.h"

#define BOTAO_A 5 
#define BOTAO_B 6

#define LED_VERMELHO 13 
#define LED_VERDE    12 
#define LED_AZUL     11 

#define WS2812_PIN    7

volatile int numero_atual = -1;
volatile bool atualizar_display = false;
volatile uint32_t valor_led; 
volatile PIO pio; 
volatile uint sm; 
volatile double r; 
volatile double g; 
volatile double b;

// Blink a 5 Hz → 10 toggles por segundo = 100 ms cada toggle
#define LED_BLINK_INTERVAL_US  100000  

// Armazena o último tempo em que o LED foi toggled
static uint64_t ultimo_toggle_led = 0;

// Armazena o último tempo em que houve pressionamento detectado

#define DEBOUNCE_DELAY_US  100000
static uint64_t ultimo_tempo_a = 0;
static uint64_t ultimo_tempo_b = 0;


/******************************************MOSTRAR NÚMEROS 0 A 9************************************/

int matriz_numeros[10][5][5] =  { 

    // Número 0
    {1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0, 
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0},

    // Número 1
    {1.0, 1.0, 1.0, 1.0, 1.0,
     0.0, 0.0, 1.0, 0.0, 0.0, 
     0.0, 0.0, 1.0, 0.0, 0.0,
     0.0, 1.0, 1.0, 0.0, 0.0,
     0.0, 0.0, 1.0, 0.0, 0.0},

    // Número 2
    {1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 0.0, 
     1.0, 1.0, 1.0, 1.0, 1.0,
     0.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0},

    // Número 3
    {1.0, 1.0, 1.0, 1.0, 1.0,
     0.0, 0.0, 0.0, 0.0, 1.0, 
     1.0, 1.0, 1.0, 1.0, 1.0,
     0.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0},

    // Número 4
    {1.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 1.0, 
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0},

    // Número 5
    {1.0, 1.0, 1.0, 1.0, 1.0,
     0.0, 0.0, 0.0, 0.0, 1.0, 
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 0.0,
     1.0, 1.0, 1.0, 1.0, 1.0},

    // Número 6
    {1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0, 
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 0.0,
     1.0, 1.0, 1.0, 1.0, 1.0},

    // Número 7
    {1.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 1.0, 
     1.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0},

    // Número 8
    {1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0, 
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0},

    // Número 9
    {1.0, 1.0, 1.0, 1.0, 1.0,
     0.0, 0.0, 0.0, 0.0, 1.0, 
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0},
};


// Callback único para todos os pinos
void botoes_irq_handler(uint gpio, uint32_t events) {
    uint64_t agora = time_us_64();  // tempo atual em microssegundos

    if (gpio == BOTAO_A && gpio_get(BOTAO_A) == 0) {
        // Verifica se tempo atual - ultimo_tempo_a >= DEBOUNCE_DELAY_US
        if ( (agora - ultimo_tempo_a) >= DEBOUNCE_DELAY_US ) {
            ultimo_tempo_a = agora;   // atualiza o tempo
            numero_atual = (numero_atual + 1) % 10;
            atualizar_display = true;
        }
    }
    else if (gpio == BOTAO_B && gpio_get(BOTAO_B) == 0) {
        if ( (agora - ultimo_tempo_b) >= DEBOUNCE_DELAY_US ) {
            ultimo_tempo_b = agora;
            numero_atual = (numero_atual - 1 + 10) % 10;
            atualizar_display = true;
        }
    }
}

uint32_t matrix_rgb(double b, double r, double g) {
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

void desenho_pio(int *matriz_numeros, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b);

void number_animation(uint32_t valor_led, PIO pio, uint sm, double r, double g, double b) {
    int num_matrizes = sizeof(matriz_numeros) / sizeof(matriz_numeros[0]);
    for (int frame = 0; frame < num_matrizes; frame++) {
        desenho_pio((int *)matriz_numeros[frame], valor_led, pio, sm, r, g, b);
    }
}

void desenho_pio(int *matriz_numeros, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b) {
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            int led_index = row * 5 + col;
            valor_led = matrix_rgb(b, matriz_numeros[led_index], g);
            pio_sm_put_blocking(pio, sm, valor_led);
        }
    }
}

int main() {
    PIO pio = pio0;
    uint32_t valor_led;
    double r = 1.0, g = 0.0, b = 0.0;
    
    stdio_init_all();
    
    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_put(LED_VERMELHO, 0);
    
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &botoes_irq_handler);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &botoes_irq_handler);
    
    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, WS2812_PIN);
    
    // Marca o último toggle do LED como o momento atual
    ultimo_toggle_led = time_us_64();

    while (true) {

        // ----- Blink do LED vermelho a 5 Hz sem delay -----
        uint64_t agora = time_us_64();
        if ((agora - ultimo_toggle_led) >= LED_BLINK_INTERVAL_US) {
            ultimo_toggle_led = agora;
            // Inverte o estado do LED vermelho
            gpio_xor_mask(1 << LED_VERMELHO);
        }
        
        if (atualizar_display) {
            desenho_pio((int *)matriz_numeros[numero_atual], valor_led, pio, sm, r, g, b);
            atualizar_display = false;
        }
    }
}

