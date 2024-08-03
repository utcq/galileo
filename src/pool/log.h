#ifndef LOG_H
#define LOG_H
#include <stdio.h>


#define DEBUG(x, ...) printf("\x1b[32m[DEBUG] "); printf(x, ##__VA_ARGS__); printf("\x1b[0m")
#define INFO(x, ...) printf("\x1b[36m[INFO] "); printf(x, ##__VA_ARGS__); printf("\x1b[0m")
#define WARN(x, ...) printf("\x1b[33m[WARN] "); printf(x, ##__VA_ARGS__); printf("\x1b[0m")
#define ERROR(x, ...) printf("\x1b[31m[ERROR] "); printf(x, ##__VA_ARGS__); printf("\x1b[0m")
#define PRINT(x, ...) printf(x, ##__VA_ARGS__)

#endif
