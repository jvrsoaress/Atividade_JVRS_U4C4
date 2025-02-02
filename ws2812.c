#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

// Configurações globais
#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7
#define LED_RED_PIN 13
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define DEBOUNCE_TIME_MS 250

// Variáveis globais
uint8_t led_r = 0; // Intensidade do vermelho 
uint8_t led_g = 1;   // Intensidade do verde (0 a 255)
uint8_t led_b = 0;   // Intensidade do azul
int current_number = 0; // Número atual exibido na matriz
uint32_t last_time = 0; // Último tempo de interrupção

// Buffer para armazenar quais LEDs estão ligados (matriz 5x5)
bool led_buffer[NUM_PIXELS] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0
};

// Função para definir um pixel na matriz de LEDs
static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Função para converter RGB em formato WS2812
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Função para atualizar a matriz de LEDs com base no número atual
void update_led_matrix(int number)
{
    // Limpa o buffer
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        led_buffer[i] = 0;
    }

    // Define os LEDs que devem ser ligados para cada número
    switch (number)
    {
        case 0:
            // Número 0
            led_buffer[24] = 0; led_buffer[23] = 1; led_buffer[22] = 1; led_buffer[21] = 1; led_buffer[20] = 0;
            led_buffer[15] = 1; led_buffer[16] = 0; led_buffer[17] = 0; led_buffer[18] = 0; led_buffer[19] = 1;
            led_buffer[14] = 1; led_buffer[13] = 0; led_buffer[12] = 0; led_buffer[11] = 0; led_buffer[10] = 1;
            led_buffer[5] = 1; led_buffer[6] = 0; led_buffer[7] = 0; led_buffer[8] = 0; led_buffer[9] = 1;
            led_buffer[4] = 0; led_buffer[3] = 1; led_buffer[2] = 1; led_buffer[1] = 1; led_buffer[0] = 0;

            break;
        case 1:
            // Número 1
            led_buffer[24] = 0; led_buffer[23] = 0; led_buffer[22] = 1; led_buffer[21] = 0; led_buffer[20] = 0;
            led_buffer[15] = 0; led_buffer[16] = 1; led_buffer[17] = 1; led_buffer[18] = 0; led_buffer[19] = 0;
            led_buffer[14] = 0; led_buffer[13] = 0; led_buffer[12] = 1; led_buffer[11] = 0; led_buffer[10] = 0;
            led_buffer[5] = 0; led_buffer[6] = 0; led_buffer[7] = 1; led_buffer[8] = 0; led_buffer[9] = 0;
            led_buffer[4] = 0; led_buffer[3] = 1; led_buffer[2] = 1; led_buffer[1] = 1; led_buffer[0] = 0;
            break;
            case 2:
            // Número 2
            led_buffer[24] = 0; led_buffer[23] = 1; led_buffer[22] = 1; led_buffer[21] = 1; led_buffer[20] = 0;
            led_buffer[15] = 0; led_buffer[16] = 0; led_buffer[17] = 0; led_buffer[18] = 1; led_buffer[19] = 0;
            led_buffer[14] = 0; led_buffer[13] = 1; led_buffer[12] = 1; led_buffer[11] = 1; led_buffer[10] = 0;
            led_buffer[5] = 0; led_buffer[6] = 1; led_buffer[7] = 0; led_buffer[8] = 0; led_buffer[9] = 0;
            led_buffer[4] = 0; led_buffer[3] = 1; led_buffer[2] = 1; led_buffer[1] = 1; led_buffer[0] = 0;
            break;
            case 3:
            // Número 3
            led_buffer[24] = 0; led_buffer[23] = 1; led_buffer[22] = 1; led_buffer[21] = 1; led_buffer[20] = 0;
            led_buffer[15] = 0; led_buffer[16] = 0; led_buffer[17] = 0; led_buffer[18] = 1; led_buffer[19] = 0;
            led_buffer[14] = 0; led_buffer[13] = 1; led_buffer[12] = 1; led_buffer[11] = 1; led_buffer[10] = 0;
            led_buffer[5] = 0; led_buffer[6] = 0; led_buffer[7] = 0; led_buffer[8] = 1; led_buffer[9] = 0;
            led_buffer[4] = 0; led_buffer[3] = 1; led_buffer[2] = 1; led_buffer[1] = 1; led_buffer[0] = 0;
            break;
            case 4:
            // Número 4
            led_buffer[24] = 0; led_buffer[23] = 1; led_buffer[22] = 0; led_buffer[21] = 1; led_buffer[20] = 0;
            led_buffer[15] = 0; led_buffer[16] = 1; led_buffer[17] = 0; led_buffer[18] = 1; led_buffer[19] = 0;
            led_buffer[14] = 0; led_buffer[13] = 1; led_buffer[12] = 1; led_buffer[11] = 1; led_buffer[10] = 0;
            led_buffer[5] = 0; led_buffer[6] = 0; led_buffer[7] = 0; led_buffer[8] = 1; led_buffer[9] = 0;
            led_buffer[4] = 0; led_buffer[3] = 0; led_buffer[2] = 0; led_buffer[1] = 1; led_buffer[0] = 0;
            break;
            case 5:
            // Número 5
            led_buffer[24] = 0; led_buffer[23] = 1; led_buffer[22] = 1; led_buffer[21] = 1; led_buffer[20] = 0;
            led_buffer[15] = 0; led_buffer[16] = 1; led_buffer[17] = 0; led_buffer[18] = 0; led_buffer[19] = 0;
            led_buffer[14] = 0; led_buffer[13] = 1; led_buffer[12] = 1; led_buffer[11] = 1; led_buffer[10] = 0;
            led_buffer[5] = 0; led_buffer[6] = 0; led_buffer[7] = 0; led_buffer[8] = 1; led_buffer[9] = 0;
            led_buffer[4] = 0; led_buffer[3] = 1; led_buffer[2] = 1; led_buffer[1] = 1; led_buffer[0] = 0;
            break;
            case 6:
            // Número 6
            led_buffer[24] = 0; led_buffer[23] = 0; led_buffer[22] = 1; led_buffer[21] = 1; led_buffer[20] = 0;
            led_buffer[15] = 0; led_buffer[16] = 1; led_buffer[17] = 0; led_buffer[18] = 0; led_buffer[19] = 0;
            led_buffer[14] = 0; led_buffer[13] = 1; led_buffer[12] = 1; led_buffer[11] = 1; led_buffer[10] = 0;
            led_buffer[5] = 0; led_buffer[6] = 1; led_buffer[7] = 0; led_buffer[8] = 1; led_buffer[9] = 0;
            led_buffer[4] = 0; led_buffer[3] = 1; led_buffer[2] = 1; led_buffer[1] = 1; led_buffer[0] = 0;
            break;
            case 7:
            // Número 7
            led_buffer[24] = 0; led_buffer[23] = 1; led_buffer[22] = 1; led_buffer[21] = 1; led_buffer[20] = 0;
            led_buffer[15] = 0; led_buffer[16] = 0; led_buffer[17] = 0; led_buffer[18] = 1; led_buffer[19] = 0;
            led_buffer[14] = 0; led_buffer[13] = 0; led_buffer[12] = 0; led_buffer[11] = 1; led_buffer[10] = 0;
            led_buffer[5] = 0; led_buffer[6] = 0; led_buffer[7] = 0; led_buffer[8] = 1; led_buffer[9] = 0;
            led_buffer[4] = 0; led_buffer[3] = 0; led_buffer[2] = 0; led_buffer[1] = 1; led_buffer[0] = 0;
            break;
            case 8:
            // Número 8
            led_buffer[24] = 0; led_buffer[23] = 1; led_buffer[22] = 1; led_buffer[21] = 1; led_buffer[20] = 0;
            led_buffer[15] = 0; led_buffer[16] = 1; led_buffer[17] = 0; led_buffer[18] = 1; led_buffer[19] = 0;
            led_buffer[14] = 0; led_buffer[13] = 1; led_buffer[12] = 1; led_buffer[11] = 1; led_buffer[10] = 0;
            led_buffer[5] = 0; led_buffer[6] = 1; led_buffer[7] = 0; led_buffer[8] = 1; led_buffer[9] = 0;
            led_buffer[4] = 0; led_buffer[3] = 1; led_buffer[2] = 1; led_buffer[1] = 1; led_buffer[0] = 0;
            break;
            case 9:
            // Número 9
            led_buffer[24] = 0; led_buffer[23] = 1; led_buffer[22] = 1; led_buffer[21] = 1; led_buffer[20] = 0;
            led_buffer[15] = 0; led_buffer[16] = 1; led_buffer[17] = 0; led_buffer[18] = 1; led_buffer[19] = 0;
            led_buffer[14] = 0; led_buffer[13] = 1; led_buffer[12] = 1; led_buffer[11] = 1; led_buffer[10] = 0;
            led_buffer[5] = 0; led_buffer[6] = 0; led_buffer[7] = 0; led_buffer[8] = 1; led_buffer[9] = 0;
            led_buffer[4] = 0; led_buffer[3] = 1; led_buffer[2] = 1; led_buffer[1] = 1; led_buffer[0] = 0;
            break;
            
        default:
            break;
    }

    // Atualiza a matriz de LEDs
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (led_buffer[i])
        {
            put_pixel(urgb_u32(led_r, led_g, led_b)); // Liga o LED
        }
        else
        {
            put_pixel(0); // Desliga o LED
        }
    }
}

// Função de interrupção com debouncing
void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    // Verifica se passou tempo suficiente desde o último evento
    if (current_time - last_time > DEBOUNCE_TIME_MS)
    {
        last_time = current_time; // Atualiza o tempo do último evento
        printf("Interrupção ocorreu no pino %d, no evento %d\n", gpio, events);

        if (gpio == BUTTON_A_PIN)
        {
            current_number = (current_number + 1) % 10; // Incrementa o número
        }
        else if (gpio == BUTTON_B_PIN)
        {
            current_number = (current_number - 1 + 10) % 10; // Decrementa o número
        }

        update_led_matrix(current_number); // Atualiza a matriz de LEDs
    }
}

int main()
{
    // Inicializa o SDK
    stdio_init_all();

    // Configura o LED vermelho
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);

    // Configura os botões
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    // Configura as interrupções dos botões
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Inicializa a matriz de LEDs
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // Loop principal
    while (1)
    {
        // Pisca o LED vermelho 5 vezes por segundo
        gpio_put(LED_RED_PIN, 1);
        sleep_ms(200);
        gpio_put(LED_RED_PIN, 0);
        sleep_ms(200);
    }

    return 0;
}