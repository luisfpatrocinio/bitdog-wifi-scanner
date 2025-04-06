#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "hardware/vreg.h"
#include "hardware/clocks.h"

#include "libs/display.h"
#include "libs/text.h"

#define NEW_SCAN_TIMER_MS 5000 // Tempo de espera entre as varreduras (10 segundos)

const uint LED_PIN_RED = 13;

#define MAX_RESULTS 5

typedef struct {
    char ssid[33]; // SSID da rede Wi-Fi (32 caracteres + terminador nulo)
    int rssi; // Intensidade do sinal (RSSI)
} wifi_network_t;

wifi_network_t networks[MAX_RESULTS]; // Array para armazenar os resultados da varredura
int network_count = 0; // Contador de redes encontradas

// Função chamada automaticamente sempre que um resultado de varredura
// é encontrado. O resultado é passado como argumento (result).
static int scanResult(void *env, const cyw43_ev_scan_result_t *result)
{
    printf("SSID: %s, RSSI: %d\n", result->ssid, result->rssi);
    if (result && network_count < MAX_RESULTS)
    {
        strncpy(networks[network_count].ssid, (const char*)result->ssid, 32);
        networks[network_count].ssid[32] = '\0'; // Garante que a string esteja terminada
        networks[network_count].rssi = result->rssi; // Armazena a intensidade do sinal
        network_count++; // Incrementa o contador de redes encontradas
        printf("Rede encontrada: %s (RSSI: %d)\n", networks[network_count - 1].ssid, networks[network_count - 1].rssi);
    }
    return 0; // Retorna 0 para continuar a varredura.
}

void showNetworksOnDisplay() 
{
    clearDisplay(); 
    int y = 0; // Posição vertical
    drawTextCentered("Patro Wi-fi Scanner", y);
    y += TEXT_HEIGHT; 

    // Header:
    char header[64];
    snprintf(header, sizeof(header), "Redes encontradas (%d)", network_count);
    drawTextCentered(header, y);

    for (int i = 0; i < network_count; i++)
    {
        char line[64];
        snprintf(line, sizeof(line), "%s (%ddBm)", networks[i].ssid, networks[i].rssi);
        y += 8;
        drawText(0, y, line);
        printf(line);
        if (y >= SCREEN_HEIGHT) break;
    }
    showDisplay();
}

int main()
{
    stdio_init_all(); // Inicializa a comunicação serial.
    sleep_ms(1000);
    printf("* Patro Wi-fi Scanner - Embarcatech 2025\n");
    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);

    // Inicializar LED
    initI2C();
    initDisplay(); // Inicializa o display I2C
    clearDisplay();
    drawTextCentered("Patro Wi-fi Scanner", SCREEN_HEIGHT / 2 - 8);
    showDisplay(); // Limpa o display

    // Inicializar wi-fi
    if (cyw43_arch_init())
    {
        printf("Falha ao inicializar o Wi-Fi\n");
        return 1;
    }

    printf("Wi-Fi inicializado com sucesso\n");

    // Ativa o modo Station (STA)
    cyw43_arch_enable_sta_mode();

    // Inicia varredura imeditamente.
    absolute_time_t scanTime = make_timeout_time_ms(0);

    bool scanning = false;

    while (true)
    {
        if (absolute_time_diff_us(get_absolute_time(), scanTime) < 0)
        {
            // Se nenhuma varredura estiver em andamento, inicia uma nova varredura.
            if (!scanning)
            {
                // Cria uma estrutura para configurar as opções de varredura.
                cyw43_wifi_scan_options_t scanOptions = {0};

                // Inicia a varredura wi-fi utilizando as opções configuradas.
                int err = cyw43_wifi_scan(&cyw43_state, &scanOptions, NULL, scanResult);
                
                if (err == 0)
                {
                    printf("Iniciando varredura...\n");
                    network_count = 0; // Limpa os dados antigos
                    scanning = true;
                }
                else
                {
                    printf("Erro ao iniciar varredura: %d\n", err);
                    scanTime = make_timeout_time_ms(NEW_SCAN_TIMER_MS);
                }
            }
            else if (!cyw43_wifi_scan_active(&cyw43_state))
            {
                printf("Varredura concluída\n");
                showNetworksOnDisplay();
                scanTime = make_timeout_time_ms(NEW_SCAN_TIMER_MS);
                scanning = false;
            }
        }
        gpio_put(LED_PIN_RED, true);
        sleep_ms(169);
        gpio_put(LED_PIN_RED, false);
        sleep_ms(169);

#if PICO_CYW43_ARCH_POLL
            cyw43_arch_poll();
            cyw43_arch_wait_for_work_until(scanTime);
#else
            sleep_ms(NEW_SCAN_TIMER_MS);
#endif
    }
        // Libera os recursos do wi-fi antes de encerrar o programa.
        cyw43_arch_deinit();

        printf("Wi-Fi desativado\n");
        printf("Encerrando o programa...\n");
        return 0;
}