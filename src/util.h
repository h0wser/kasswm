#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/* Bools */
#define TRUE 1
#define FALSE 0

/* Debug print macro */
#ifdef NDEBUG
	#define pdebug(...)
#else
	#define pdebug(M, ...) fprintf(stderr, "[DEBUG: %s:%d]: " M "\n", __FILE__, __LINE__,##__VA_ARGS__)
#endif

/* Logging */
#define errno_fmt() (errno == 0 ? "" : strerror(errno))
#define log_info(M, ...) fprintf(stderr, "[INFO]: " M "\n", ##__VA_ARGS__)
#define log_warn(M, ...) fprintf(stderr, "[WARN]: %d: %s " M "\n", errno, errno_fmt(), ##__VA_ARGS__)
#define log_error(M, ...) fprintf(stderr, "[ERROR]: errno: %d: %s | " M "\n", errno, errno_fmt(), ##__VA_ARGS__)

/* Error checking */
#define check(A, M, ...) if (!(A)) {errno=0; log_error(M, ##__VA_ARGS__); goto error;}


#endif /* UTIL_H */
