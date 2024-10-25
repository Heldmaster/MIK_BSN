#include "geoprocessing.h"
#include "hashutils.h"
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>



void UART1_SendString(char *String) {
    printf("%s", String);
}

void UART2_SendString(char *String) {
    printf("%s", String);
}

uint8_t ParseCENGresponse(char *response, struct CellTower *towers) {
    uint8_t count = 0;
    char *line = strtok(response, "\r\n");

    while (line != NULL && count < 7) {
        printf("Parsing line: %s\n", line); 

        if (count == 0 && strstr(line, "+CENG: 0") != NULL) {
            int tempMCC, tempMNC, tempLAC, tempCID, tempLevel;
            if (sscanf(line, "+CENG: 0,\"%*[^,],%d,%*[^,],%d,%d,%*d,%d,%*[^,],%*[^,],%x",
                       &tempLevel, &tempMCC, &tempMNC, &tempLAC, &tempCID) == 5) {
                towers[count].MCC = tempMCC;
                towers[count].MNC = tempMNC;
                towers[count].LAC = tempLAC;
                towers[count].CID = tempCID;
                towers[count].receiveLevel = tempLevel;
                count++;
                printf("Parsed main tower: MCC=%d, MNC=%d, LAC=%d, CID=%d, ReceiveLevel=%d\n",
                       tempMCC, tempMNC, tempLAC, tempCID, tempLevel);
            } else {
                printf("Failed to parse main tower.\n"); 
            }
        } 
        else if (strstr(line, "+CENG:") != NULL) {
            int tempMCC, tempMNC, tempLAC, tempCID, tempLevel;
            if (sscanf(line, "+CENG: %*d,\"%*[^,],%d,%*[^,],%x,%d,%d,%x",
                       &tempLevel, &tempCID, &tempMCC, &tempMNC, &tempLAC) >= 4) {
                if (tempMCC != 0xFFFF && tempMNC != 0xFFFF && tempLAC != 0xFFFF && tempCID != 0xFFFF) {
                    towers[count].MCC = tempMCC;
                    towers[count].MNC = tempMNC;
                    towers[count].LAC = tempLAC;
                    towers[count].CID = tempCID;
                    towers[count].receiveLevel = tempLevel;
                    count++;
                    printf("Parsed tower: MCC=%d, MNC=%d, LAC=%d, CID=%d, ReceiveLevel=%d\n",
                           tempMCC, tempMNC, tempLAC, tempCID, tempLevel);
                }
            } else {
                printf("Failed to parse line: %s\n", line); 
            }
        }

        line = strtok(NULL, "\r\n");
    }

    printf("Total parsed towers: %d\n", count); 
    return count;
}

double signal_to_distance(int16_t receiveLevel, double frequency) {
    double PL = receiveLevel; 
    double d = pow(10, (PL - 20 * log10(frequency) + 147.55) / 20);
    return d; 
}

// трилатерация
struct Location trilaterate(struct CellTower *towers, uint8_t towerCount, struct Node **hash_table) {
    double totalX = 0.0, totalY = 0.0;
    double totalWeight = 0.0;

    for (int i = 0; i < towerCount; i++) {
        struct Node *tower = search_in_hash_table(hash_table, towers[i].MCC, towers[i].MNC, towers[i].CID);
        
        if (tower == NULL) {
            fprintf(stderr, "Error finding tower location in database for tower %d.\n", i);
            continue; 
        }

        double distance = signal_to_distance(towers[i].receiveLevel, 900e6);
        totalX += tower->LONG * distance;
        totalY += tower->LAT * distance;
        totalWeight += distance; 
    }

  
    if (totalWeight > 0) {
        totalX /= totalWeight;
        totalY /= totalWeight;
    } else {
        fprintf(stderr, "Total weight is zero, cannot calculate location.\n");
        return (struct Location){0, 0};
    }

    return (struct Location){totalY, totalX};
}