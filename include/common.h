#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <math.h>
#include <glad.h>
#include <GLFW/glfw3.h>
#ifdef __linux__
	#include <sys/stat.h>
	#include <unistd.h>
	#include <dirent.h>
#endif

#define MEMORY_POOL_CAPACITY (16 * 1024 * 1024) // 16 MiB
#define PROGRAM_NAME "Flasher version 1.0.0 (beta)"


typedef uint32_t RGBA;

typedef struct Point2f Point2f;
struct Point2f { double x, y; };

typedef struct Vec2f Vec2f;
struct Vec2f { double u, v; };


#define log_info(fmt, ...) \
    do { \
        printf("[%s:%d] ", __FILE__, __LINE__); \
        printf(fmt, ##__VA_ARGS__); \
        fflush(stdout); \
    } while(0)


#define log_exit(fmt, ...) \
    do { \
        printf("[%s:%d] ", __FILE__, __LINE__); \
        printf(fmt, ##__VA_ARGS__); \
        fflush(stdout); \
		exit(0); \
    } while(0)


void*	memory_alloc(size_t space);
size_t	memory_getOffset();


#endif
