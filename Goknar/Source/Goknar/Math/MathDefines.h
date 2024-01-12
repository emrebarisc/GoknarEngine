#ifndef __MATH_DEFINES__
#define __MATH_DEFINES__

#define PI 3.14159265359f
#define TWO_PI 6.28318530718f
#define HALF_PI 1.57079632679f
#define ONE_OVER_PI 0.31830988618f
#define NATURAL_LOGARITHM 2.71828182845f
#define LN_OF_2 0.69314718056f

#define TO_RADIAN PI / 180.f
#define TO_DEGREE 180.f / PI

#define RADIAN_TO_DEGREE(radian) (radian * TO_DEGREE)
#define DEGREE_TO_RADIAN(degree) (degree * TO_RADIAN)

#define SMALLER_EPSILON 0.000001f
#define EPSILON 0.001f
#define BIGGER_EPSILON 0.1f
#define INTERSECTION_TEST_EPSILON 0.001f
#define SHADOW_EPSILON 0.001f

#endif