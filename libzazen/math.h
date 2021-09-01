#ifndef LIBZAZEN_MATH_H
#define LIBZAZEN_MATH_H

typedef struct {
  float x, y, z;
} Point;

typedef struct {
  Point begin, end;
} Line;

typedef struct {
  Point p1, p2, p3;
} Triangle;

#endif  //  LIBZAZEN_MATH_H
