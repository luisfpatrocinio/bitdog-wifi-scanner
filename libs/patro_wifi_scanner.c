#include "patro_wifi_scanner.h"

int selectedOption = 0;
int network_count = 0;
int inputCooldown = 0;

// Função para desenhar barras de sinal no display
void drawSignalBars(int x, int y, int bars) {
    int barWidth = 2;
    int barSpacing = 1;
    int barHeightUnit = 2;

    for (int i = 0; i < 4; i++) {
        int height = (i < bars) ? (i + 1) * barHeightUnit : 0;
        if (height > 0) {
            int barX = x + i * (barWidth + barSpacing);
            int barY = y + (TEXT_HEIGHT - height);
            drawRectangle(barX, barY, barWidth, height);
        }
    }
}

void drawAppHeader() {
    drawClearRectangle(0, 0, SCREEN_WIDTH, 16); // Limpa a área do cabeçalho

    // Título:
    int y = 0; 
    drawTextCentered("Patro Wi-fi Scanner", y);
    y += TEXT_HEIGHT; 

    // Header:
    char header[64];
    snprintf(header, sizeof(header), "Networks found (%d)", network_count);
    drawTextCentered(header, y);
    drawLine(0, 16, SCREEN_WIDTH, 16);
}