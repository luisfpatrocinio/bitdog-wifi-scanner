#include "patro_wifi_scanner.h"

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