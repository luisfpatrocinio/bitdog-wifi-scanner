// Standard libraries
#include <stdio.h>
#include <stdlib.h>

// Pico SDK libraries
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// Hardware-specific libraries
#include "hardware/vreg.h"
#include "hardware/clocks.h"

// Custom libraries
#include "libs/display.h"
#include "libs/text.h"
#include "libs/draw.h"



// Tempo de espera entre as varreduras (10 segundos)
#define NEW_SCAN_TIMER_MS 500 

// Pino do LED vermelho
const uint LED_PIN_RED = 13;

// Definições de hardware para o display I2C
#define MAX_RESULTS 5

// Estrutura para armazenar informações de redes Wi-Fi
typedef struct {
    char ssid[33];      // SSID da rede Wi-Fi (32 caracteres + '\0')
    uint8_t bssid[6];   // Endereço MAC da rede (BSSID)
    int rssi;           // Intensidade do sinal (RSSI)
} wifi_network_t;

// Array para armazenar os resultados da varredura
wifi_network_t networks[MAX_RESULTS];   

// Contador de redes encontradas
int network_count = 0;                  

// Função chamada automaticamente sempre que um resultado de varredura
// é encontrado. O resultado é passado como argumento (result).
static int scanResult(void *env, const cyw43_ev_scan_result_t *result)
{
    // Pular redes com SSID vazio ou nulo
    if (!result || strlen((const char*)result->ssid) == 0)
        return 0; 

    // Verifica se o BSSID já está na lista
    for (int i = 0; i < network_count; i++)
    {
        // Compara o BSSID da rede atual com os já encontrados
        if (memcmp(networks[i].bssid, result->bssid, sizeof(networks[i].bssid)) == 0)
        {
            // Atualiza o RSSI se já estiver na lista
            networks[i].rssi = result->rssi; 
            return 0;
        }
    }

    if (network_count < MAX_RESULTS)
    {
        // Copia o SSID da rede encontrada para a estrutura
        strncpy(networks[network_count].ssid, (const char*)result->ssid, 32);
        networks[network_count].ssid[32] = '\0'; // Garante que a string esteja terminada

        // Copia o BSSID da rede encontrada para a estrutura
        memcpy(networks[network_count].bssid, result->bssid, sizeof(networks[network_count].bssid)); 

        // Armazena a intensidade do sinal (RSSI) da rede encontrada
        networks[network_count].rssi = result->rssi; 

        network_count++; // Incrementa o contador de redes encontradas
    }
    return 0; // Retorna 0 para continuar a varredura.
}

/**
 * @brief Função para exibir as redes Wi-Fi encontradas no display.
 *        Limpa o display antes de exibir os resultados.
 */
void showNetworksOnDisplay() 
{
    clearDisplay(); 

    // Título:
    int y = 0; 
    drawTextCentered("Patro Wi-fi Scanner", y);
    y += TEXT_HEIGHT; 

    // Header:
    char header[64];
    snprintf(header, sizeof(header), "Redes encontradas (%d)", network_count);
    drawTextCentered(header, y);
    drawLine(0, 16, SCREEN_WIDTH, 16);
    
    y= 18;
    for (int i = 0; i < network_count; i++)
    {
        char ssid[33];
        char rssi[16];
        snprintf(ssid, sizeof(ssid), "%s", networks[i].ssid);
        snprintf(rssi, sizeof(rssi), "(%ddBm)", networks[i].rssi);
        drawText(0, y, ssid);
        
        int _rssi_x = SCREEN_WIDTH - 50; // Posição do RSSI
        drawClearRectangle(_rssi_x, y, 50, TEXT_HEIGHT); // Limpa a área do RSSI
        drawText(_rssi_x, y, rssi);

        y += 8;
        if (y >= SCREEN_HEIGHT) break;
    }
    showDisplay();
}

int compareByRSSI(const void *a, const void *b) {
    const wifi_network_t *networkA = (const wifi_network_t *)a;
    const wifi_network_t *networkB = (const wifi_network_t *)b;
    return networkB->rssi - networkA->rssi; // Ordena em ordem decrescente de RSSI
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

                // Ordena as redes encontradas por RSSI (intensidade do sinal)
                qsort(networks, network_count, sizeof(wifi_network_t), compareByRSSI);

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