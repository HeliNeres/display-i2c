#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "matriz/desenho.h"
//arquivo .pio
#include "matriz.pio.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define LED_G 11
#define LED_B 12
#define OUT_PIN 7

const uint button_a = 5;
const uint button_b = 6;

const char *strings[2] = {"0", "1"};

uint last_time = 0;
bool state_g = false;
bool state_b = false;
uint8_t num = 10;

PIO pio;
uint sm;

//rotina da interrupção
static void gpio_irq_handler(uint gpio, uint32_t events);
void inicializar();

int main(){
    // Inicializa todos os códigos stdio padrão que estão ligados ao binário.
    stdio_init_all();

    //inicializando PIO
    pio = pio0;
    bool ok;
    uint16_t i;

    //coloca a frequência de clock para 128 MHz, facilitando a divisão pelo clock
    ok = set_sys_clock_khz(128000, false);

    printf("iniciando a transmissão PIO");
    if (ok) printf("clock set to %ld\n", clock_get_hz(clk_sys));

    //configurações da PIO
    uint offset = pio_add_program(pio, &matriz_program);
    sm = pio_claim_unused_sm(pio, true);
    matriz_program_init(pio, sm, offset, OUT_PIN, 800000);

    sleep_ms(1000);

    // inicializando I2C a 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    ssd1306_t ssd; // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    inicializar();

    //interrupção da gpio habilitada
    gpio_set_irq_enabled_with_callback(button_a, GPIO_IRQ_EDGE_FALL, 1, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_b, GPIO_IRQ_EDGE_FALL, 1, &gpio_irq_handler);

    bool cor = true;
    char buffer[12] = "Caractere ";
    int last_num = 10;

    while (true)
    {
        int c = getchar_timeout_us(50);
        cor = !cor;
        // Atualiza o conteúdo do display com animações
        ssd1306_fill(&ssd, !cor); // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
        ssd1306_draw_string(&ssd, "Heli N. Silva", 8, 10); // Desenha uma string
        ssd1306_draw_string(&ssd, "LedG   LedB", 8, 30); // Desenha uma string
        ssd1306_draw_string(&ssd, strings[state_g], (5*8)+8, 30); // Desenha uma string
        ssd1306_draw_string(&ssd, strings[state_b], (12*8)+8, 30); // Desenha uma string
        ssd1306_draw_string(&ssd, buffer, 8, 50);
        ssd1306_send_data(&ssd); // Atualiza o display
        
        if(c != PICO_ERROR_TIMEOUT){
            buffer[10] = (char)c;
            buffer[11] = '\0';
            last_num = c - '0';
            printf("Enviado caractere '%c' %d\n", c, last_num);
            if(last_num>=0 && last_num<=9){
                desenho_pio(digitos[last_num], pio, sm);
            }else{
                desenho_pio(digitos[10], pio, sm);
            }
        }

        sleep_ms(200);
    }
}

//rotina da interrupção botão a
static void gpio_irq_handler(uint gpio, uint32_t events){
    if (absolute_time_diff_us(last_time,get_absolute_time()) > 200000){ //debouncing de 200ms
        if(gpio==button_a){
            state_g = !state_g;
            gpio_put(LED_G,state_g);
            printf("Alterado estado do LED Verde para %d\n", state_g);
        }else if(gpio==button_b){
            state_b = !state_b;
            gpio_put(LED_B,state_b);
            printf("Alterado estado do LED Azul para %d\n", state_b);
        }
        last_time = get_absolute_time();
    }
}

void inicializar(){
    //inicializar o botão A
    gpio_init(button_a);
    gpio_set_dir(button_a, GPIO_IN);
    gpio_pull_up(button_a);

    //inicializar o botão B
    gpio_init(button_b);
    gpio_set_dir(button_b, GPIO_IN);
    gpio_pull_up(button_b);

    // inicializar led verde
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_put(LED_G,0);

    // inicializar led azul
    gpio_init(LED_B);
    gpio_set_dir(LED_B, GPIO_OUT);
    gpio_put(LED_B,0);
}
