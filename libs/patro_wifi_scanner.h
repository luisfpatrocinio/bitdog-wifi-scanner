#ifndef PATRO_WIFI_SCANNER_H
#define PATRO_WIFI_SCANNER_H
#include "text.h"
#include "draw.h"

// Contador de redes encontradas
extern int network_count;
// Opção selecionada no menu
extern int selectedOption;
// Cooldown para evitar múltiplas leituras rápidas
extern int inputCooldown;

void drawSignalBars(int x, int y, int bars);
void drawAppHeader();


#endif