// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Pico SDK libraries
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// Hardware-specific libraries
#include "hardware/vreg.h"
#include "hardware/clocks.h"

// Custom libraries
#include "display.h"
#include "text.h"
#include "draw.h"
#include "patro_wifi_scanner.h"
#include "analog.h"
#include "buttons.h"

// Tempo de espera entre as varreduras (10 segundos)
#define NEW_SCAN_TIMER_MS 10000 
#define MAX_RESULTS 20

#define DEG2RAD 0.0174532925

// Pino do LED vermelho
const uint LED_PIN_RED = 13;

// Estrutura para armazenar informações de redes Wi-Fi
typedef struct {
    char ssid[33];      // SSID da rede Wi-Fi (32 caracteres + '\0')
    uint8_t bssid[6];   // Endereço MAC da rede (BSSID)
    int rssi;           // Intensidade do sinal (RSSI)
    uint64_t auth_mode;  // Modo de autenticação (WPA, WPA2, etc.)
    
} wifi_network_t;

// Array para armazenar os resultados da varredura
wifi_network_t networks[MAX_RESULTS];   

int scrollY = 0; // Posição de rolagem do menu

// Função para converter RSSI em barras de sinal (1 a 5)
int rssiToBars(int rssi) {
    if (rssi >= -50) return 5; // Excelente sinal
    if (rssi >= -60) return 4; // Bom sinal
    if (rssi >= -70) return 3; // Sinal razoável
    if (rssi >= -80) return 2; // Sinal fraco
    return 1; // Sem sinal
}

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

        // Armazena o modo de autenticação da rede encontrada
        networks[network_count].auth_mode = result->auth_mode;

        network_count++; // Incrementa o contador de redes encontradas
    }
    return 0; // Retorna 0 para continuar a varredura.
}

void drawNetworkDetailsAtBottom(int selectedOption) {
    int y = SCREEN_HEIGHT - TEXT_HEIGHT - 1; // Posição do texto na parte inferior
    char details[64];

    drawClearRectangle(0, y, SCREEN_WIDTH, SCREEN_HEIGHT); // Limpa a área do texto
    drawLine(0, y, SCREEN_WIDTH, y); // Linha horizontal

    uint64_t thisAuthMode = networks[selectedOption].auth_mode; // Modo de autenticação da rede selecionada
    snprintf(details, sizeof(details), "Mode: %s",
                thisAuthMode == CYW43_AUTH_OPEN ? "Open" :
                thisAuthMode == CYW43_AUTH_WPA2_AES_PSK ? "WPA2 (AES)" :
                thisAuthMode == CYW43_AUTH_WPA2_MIXED_PSK ? "WPA2 (Misto)" :
                thisAuthMode == CYW43_AUTH_WPA3_SAE_AES_PSK ? "WPA3 (SAE AES)" :
                thisAuthMode == CYW43_AUTH_WPA3_WPA2_AES_PSK ? "WPA3/WPA2 (AES)" :
                thisAuthMode == CYW43_AUTH_WPA_TKIP_PSK ? "WPA (TKIP)" :
                "Locked");

    if (thisAuthMode == CYW43_AUTH_WPA2_AES_PSK) {
        printf("Rede: %s\n", networks[selectedOption].ssid); // Exibe o SSID da rede selecionada
        printf("Modo de autenticação: WPA2 (AES)\n"); // Exibe o modo de autenticação no console
    }

    if (thisAuthMode == CYW43_AUTH_WPA2_MIXED_PSK) {
        printf("Rede: %s\n", networks[selectedOption].ssid); // Exibe o SSID da rede selecionada
        printf("Modo de autenticação: WPA2 (Misto)\n"); // Exibe o modo de autenticação no console
    }

    if (thisAuthMode == CYW43_AUTH_OPEN) {
        printf("Rede: %s\n", networks[selectedOption].ssid); // Exibe o SSID da rede selecionada
        printf("Modo de autenticação: Aberta\n"); // Exibe o modo de autenticação no console
    }

    drawText(0, y + 1, details); // Exibe o modo de autenticação da rede selecionada
}

/**
 * @brief Função para exibir as redes Wi-Fi encontradas no display.
 *        Limpa o display antes de exibir os resultados.
 */
void showNetworksOnDisplay() 
{
    static int _timer = 0;
    _timer += 4;
    if (_timer > 360) {
        _timer -= 360;
    }

    clearDisplay(); 

    int targetScrollY = MIN(selectedOption, network_count - 3)* 10; // Posição alvo para rolagem

    scrollY = approach(scrollY, targetScrollY, 1); // Atualiza a posição de rolagem

    int y = 22 - scrollY;
    for (int i = 0; i < network_count; i++)
    {
        // Formata o SSID e RSSI para exibição
        char ssid[33];
        char rssi[16];
        snprintf(ssid, sizeof(ssid), "%s", networks[i].ssid);
        snprintf(rssi, sizeof(rssi), "(%ddBm)", networks[i].rssi);

        // Desenha o nome da rede (SSID)
        if (i == selectedOption) {
            int _x = 2 + sin(_timer * DEG2RAD) * 2; // Animação de destaque
            drawText(_x, y, ">"); 
            drawText(8, y, ssid); 
        } else {
            drawText(0, y, ssid); 
        }        

        // Desenhar o Sinal (RSSI)
        int _rssi_x = SCREEN_WIDTH - 20; // Posição do RSSI
        drawClearRectangle(_rssi_x - 2, y, 50, TEXT_HEIGHT); // Limpa a área do RSSI
        
        int bars = rssiToBars(networks[i].rssi);
        drawSignalBars(_rssi_x, y, bars);

        y += 10;
        if (y >= SCREEN_HEIGHT) break;
    }

    drawAppHeader(); // Desenha o cabeçalho
    drawNetworkDetailsAtBottom(selectedOption); // Exibe detalhes da rede selecionada

    showDisplay();
}

int compareByRSSI(const void *a, const void *b) {
    const wifi_network_t *networkA = (const wifi_network_t *)a;
    const wifi_network_t *networkB = (const wifi_network_t *)b;
    return networkB->rssi - networkA->rssi; // Ordena em ordem decrescente de RSSI
}


void confirmButtonCallback(uint gpio, uint32_t events) {
    if (gpio == BTA) {
        
    } else if (gpio == BTB) {
        // Selecionar rede
        if (network_count > 0 && selectedOption < network_count) {
            // Conectar à rede selecionada
            printf("Conectando à rede: %s\n", networks[selectedOption].ssid);
            printf("Ainda não implementado.\n");
            int err = cyw43_arch_wifi_connect_timeout_ms(networks[selectedOption].ssid, NULL, networks[selectedOption].auth_mode, 10000);
            if (err == 0) {
                printf("Conectado com sucesso!\n");
                // Aqui você pode adicionar código para lidar com a conexão bem-sucedida
            } else {
                printf("Falha ao conectar: %d\n", err);
                // Aqui você pode adicionar código para lidar com a falha de conexão
            }
        } 
    }
}


int main()
{
    stdio_init_all(); // Inicializa a comunicação serial.
    sleep_ms(369);
    printf("* Patro Wi-fi Scanner - Embarcatech 2025\n");
    
    initAnalog(); // Inicializa os pinos analógicos
    initializeButtons(); // Inicializa os botões
    setButtonCallback(confirmButtonCallback); // Configura o callback para os botões

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

        // Obter input
        updateAxis();
        if (inputCooldown < 0) {
            if (analog_y < 0)
            {
                selectedOption--;
                inputCooldown = 10; 
            }
            else if (analog_y > 0)
            {
                selectedOption++;
                inputCooldown = 10;
            }
            if (selectedOption < 0) selectedOption = 0;
            if (selectedOption >= network_count) selectedOption = network_count - 1;
        } else {
            inputCooldown--;
        }

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
                
                // Reiniciar
                selectedOption = 0; // Reinicia a seleção
                scrollY = 0; // Reinicia a rolagem

                // Ordena as redes encontradas por RSSI (intensidade do sinal)
                qsort(networks, network_count, sizeof(wifi_network_t), compareByRSSI);

                scanTime = make_timeout_time_ms(NEW_SCAN_TIMER_MS);
                scanning = false;
            }
        }

        showNetworksOnDisplay();

#if PICO_CYW43_ARCH_POLL
            cyw43_arch_poll();
            cyw43_arch_wait_for_work_until(scanTime);
#else
            
        // sleep_ms(NEW_SCAN_TIMER_MS);
#endif
    }
        // Libera os recursos do wi-fi antes de encerrar o programa.
        cyw43_arch_deinit();

        printf("Wi-Fi desativado\n");
        printf("Encerrando o programa...\n");
        return 0;
}