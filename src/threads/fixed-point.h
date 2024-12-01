#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

/* Set scaling factor used in fixed-point arithmetic */
#define N_FRACTIONAL_BITS 14
#define FACTOR (1 << N_FRACTIONAL_BITS)

/* Converts an integer to a fixed-point value */
#define INT_TO_FIXED(n) ((n) * FACTOR)

/* Converts a fixed-point value to an integer via truncating */
#define FIXED_TO_INT_TRUNC(x) ((x) / FACTOR)

/* Converts a fixed-point value to an integer via rounding */
#define FIXED_TO_INT(x) (((x) >= 0) ? (((x) + (FACTOR / 2)) / FACTOR) : (((x) - (FACTOR / 2)) / FACTOR))

/* Adds two fixed-point values */
#define FIXED_ADD(x, y) ((x) + (y))

/* Adds a fixed-point value and an integer */
#define FIXED_ADD_INT(x, n) ((x) + ((n) * FACTOR))

/* Subtracts a fixed-point value from another */
#define FIXED_SUB(x, y) ((x) - (y))

/* Subtracts an integer from a fixed-point value */
#define FIXED_SUB_INT(x, n) ((x) - ((n) * FACTOR))

/* Subtracts a fixed-point value from an integer (returns fixedpoint)*/
#define INT_SUB_FIXED(n, x) (((n) * FACTOR) - (x))

/* Multiplies two fixed-point values */
#define FIXED_MUL(x, y) (((int64_t)(x)) * (y) / FACTOR)

/* Multiplies a fixed-point value with an integer */
#define FIXED_MUL_INT(x, n) ((x) * (n))

/* Divides two fixed-point values */
#define FIXED_DIV(x, y) (((int64_t)(x)) * FACTOR / (y)) 

/* Divides a fixed-point value by an integer */
#define FIXED_DIV_INT(x, n) ((x) / (n))

#endif /* threads/fixed-point.h */
