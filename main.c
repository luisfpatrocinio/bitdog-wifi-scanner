#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "hardware/vreg.h"
#include "hardware/clocks.h"

#include "libs/display.h"
#include "libs/text.h"

#define NEW_SCAN_TIMER_MS 5000 // Tempo de espera entre as varreduras (10 segundos)

const uint LED_PIN_RED = 13;

// Função chamada automaticamente sempre que um resultado de varredura
// é encontrado. O resultado é passado como argumento (result).
static int scanResult(void *env, const cyw43_ev_scan_result_t *result)
{
    if (result)
    {
        // Printa as informações do resultado da varredura.
        printf("SSID: %-32s RSSI: %4d CHAN: %3d MAC: %02x:%02x:%02x:%02x:%02x:%02x, SEC: %u\n",
               result->ssid,
               result->rssi,
               result->channel,
               result->bssid[0], result->bssid[1], result->bssid[2],
               result->bssid[3], result->bssid[4], result->bssid[5],
               result->auth_mode);
    }
    return 0; // Retorna 0 para continuar a varredura.
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