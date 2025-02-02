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

/******************************************MOSTRAR NÚMEROS 0 A 9************************************/

double matriz_numeros[10][5][5] =  { 

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
     0.0, 0.0, 1.0, 1.0, 0.0,
     0.0, 0.0, 1.0, 0.0, 0.0},

    // Número 2
    {1.0, 1.0, 1.0, 1.0, 1.0,
     0.0, 0.0, 0.0, 0.0, 1.0, 
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 0.0,
     1.0, 1.0, 1.0, 1.0, 1.0},

    // Número 3
    {1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 0.0, 
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 0.0,
     1.0, 1.0, 1.0, 1.0, 1.0},

    // Número 4
    {1.0, 0.0, 0.0, 0.0, 0.0,
     1.0, 0.0, 0.0, 0.0, 0.0, 
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0},

    // Número 5
    {1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 0.0, 
     1.0, 1.0, 1.0, 1.0, 1.0,
     0.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0},

    // Número 6
    {1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0, 
     1.0, 1.0, 1.0, 1.0, 1.0,
     0.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0},

    // Número 7
    {1.0, 0.0, 0.0, 0.0, 0.0,
     1.0, 0.0, 0.0, 0.0, 0.0, 
     1.0, 0.0, 0.0, 0.0, 0.0,
     1.0, 0.0, 0.0, 0.0, 0.0,
     1.0, 1.0, 1.0, 1.0, 1.0},

    // Número 8
    {1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0, 
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0},

    // Número 9
    {1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 0.0, 
     1.0, 1.0, 1.0, 1.0, 1.0,
     1.0, 0.0, 0.0, 0.0, 1.0,
     1.0, 1.0, 1.0, 1.0, 1.0},
};

void number_animation(uint32_t valor_led, PIO pio, uint sm, double r, double g, double b)
{
  int num_matrizes = sizeof(matriz_numeros) / sizeof(matriz_numeros[0]);

for (int frame = 0; frame < num_matrizes; frame++) {
    desenho_pio(matriz_numeros[frame], valor_led, pio, sm, r, g, b);
    sleep_ms(3000); // Intervalo entre quadros
}
}

//funcao para mapear os leds da matriz
int map_led_index(int row, int col, int width) {
    if (row % 2 == 0) {
        // Linha par (esquerda para direita)
        return row * width + col;
    } else {
        // Linha ímpar (direita para esquerda)
        return row * width + (width - 1 - col);
    }
}

//rotina para definição da intensidade de cores do led
uint32_t matrix_rgb(double b, double r, double g)
{
  unsigned char R, G, B;
  R = r * 255;
  G = g * 255;
  B = b * 255;
  return (G << 24) | (R << 16) | (B << 8);
}

//rotina para acionar a matrix de leds - ws2812b
void desenho_pio(double *matriz_numeros, uint32_t valor_led, PIO pio, uint sm, double r, double g, double b) {
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            int led_index = map_led_index(row, col, 5);
            valor_led = matrix_rgb(b = 0.0, r = matriz_numeros[led_index], g = 0.0);
            pio_sm_put_blocking(pio, sm, valor_led);
            
        }
    }
}

int main()
{
    PIO pio = pio0; 
    bool ok;
    uint16_t i;
    uint32_t valor_led;
    double r = 0.0, b = 0.0 , g = 0.0;

    ok = set_sys_clock_khz(128000, false);

    stdio_init_all();     

    printf("iniciando a transmissão PIO");
    if (ok) printf("clock set to %ld\n", clock_get_hz(clk_sys));                                                      // Inicializa bibliotecas 

    gpio_init(LED_VERMELHO);                                                    // Inicializa GPIO 13 (Led Vermelho)
    gpio_init(LED_VERDE);                                                       // Inicializa GPIO 12 (Led Amarelo)
    gpio_init(LED_AZUL);                                                        // Inicializa GPIO 11 (Led Verde)

    gpio_set_dir(LED_VERMELHO, GPIO_OUT);                                       // Define GPIO 11 como saída
    gpio_set_dir(LED_AZUL,     GPIO_OUT);                                       // Define GPIO 12 como saída
    gpio_set_dir(LED_VERDE,    GPIO_OUT);                                       // Define GPIO 13 como saída

    gpio_put(LED_VERMELHO, 0);                                                  // Liga o LED vermelho (estado inicial)
    gpio_put(LED_AZUL,  0);                                                     // Desliga o LED amarelo
    gpio_put(LED_VERDE,    0);                                                  // Desliga o LED verde

    //configurações da PIO
    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, WS2812_PIN);


    while (true) {
       number_animation(valor_led, pio, sm, r, g, b);
       
    }
}
