#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "geoprocessing.h"
#include "hashutils.h"

#define MAX_TOWERS 7

size_t DBSIZE;

int main() {
    const char *filename = "250.csv";
    DBSIZE = count_db_lines(filename);
    struct CellTower RESPONSE_BUF[MAX_TOWERS];

    struct Node **hash_table = (struct Node **) calloc(DBSIZE, sizeof(struct Node *));
    if (!hash_table) {
        fprintf(stderr, "memory error!\n");
        return 1;
    }
    printf("Memory allocated, DBSIZE = %zu\n", DBSIZE);
    printf("Parsing database started...\n");
    
    parse_and_insert_db(filename, hash_table);
    printf("Parsing database ended.\n");

    char response[] = "+CENG: 0,\"0034,47,00,250,99,40,3152,01,05,6d07,255\"\n"
                      "+CENG: 1,\"0072,37,44,27f7,250,1,27f7\"\n"
                      "+CENG: 2,\"0035,30,54,d139,250,99,6d07\"\n"
                      "+CENG: 3,\"0061,30,04,3153,250,99,6d07\"\n"
                      "+CENG: 4,\"0033,28,28,c931,250,99,6d07\"\n"
                      "+CENG: 5,\"0037,28,16,3dea,250,99,6d07\"\n"
                      "+CENG: 6,\"0063,25,17,4bd6,250,99,6d06\"\n";
    printf("Starting ParseCENGresponse...\n");

    uint8_t parsed_count = ParseCENGresponse(response, RESPONSE_BUF);
    printf("Finished ParseCENGresponse. Parsed %d towers.\n", parsed_count);

    for (int i = 0; i < parsed_count; i++) {
        printf("CellTower %d: MCC=%d, MNC=%d, LAC=%d, CID=%d, ReceiveLevel=%d\n",
               i, RESPONSE_BUF[i].MCC, RESPONSE_BUF[i].MNC, RESPONSE_BUF[i].LAC, RESPONSE_BUF[i].CID, RESPONSE_BUF[i].receiveLevel);
    }

    struct Node *result = search_in_hash_table(hash_table, 250,1,214716422);
    if (result != NULL) {
        printf("Found tower: LON=%f, LAT=%f\n", result->LAT, result->LONG);
    } else {
        printf("TOWER NOT FOUND.\n");
    }

    struct Location location = trilaterate(RESPONSE_BUF, parsed_count, hash_table);
    
    printf("Estimated Location: LAT=%f, LON=%f\n", location.latitude, location.longitude);

    free(hash_table);

    return 0;
}
