#ifndef GEOPROCESSING_H
#define GEOPROCESSING_H

#include <stdint.h>
#include <stdlib.h>
#include "hashutils.h"

// Структуры данных
/*
struct coords {
    uint16_t H, M, S, MS; // Время
    float ALT, LONG;      // Высота и долгота
};
*/


struct Location {
    double latitude;
    double longitude;
};

struct CellTower {
    uint16_t MCC;      // Код страны
    uint16_t MNC;      // Код оператора
    uint16_t LAC;      // Код региона
    uint32_t CID;      // CellID
    int16_t receiveLevel;
};

void UART1_SendString(char *String);
void UART2_SendString(char *String);
uint8_t ParseCENGresponse(char *response, struct CellTower *);
struct Location trilaterate(struct CellTower *towers, uint8_t towerCount, struct Node **hash_table);

#endif
