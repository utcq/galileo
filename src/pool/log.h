#ifndef LOG_H
#define LOG_H
#include <stdio.h>


#define DEBUG(x, ...) printf("[DEBUG] "); printf(x, ##__VA_ARGS__)
#define INFO(x, ...) printf("[INFO] "); printf(x, ##__VA_ARGS__)
#define WARN(x, ...) printf("[WARN] "); printf(x, ##__VA_ARGS__)
#define ERROR(x, ...) printf("[ERROR] "); printf(x, ##__VA_ARGS__)
#define PRINT(x, ...) printf(x, ##__VA_ARGS__)

#endif
