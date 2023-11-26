#pragma once

#include <iostream>
class vec3d {
public:
  float x, y, z, w;
  vec3d() {
    this->x = 0.0f;
    this->y = 0.0f;
    this->z = 0.0f;
    this->w = 1.0f;
  }
  vec3d(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = 1.0f;
  }
};

inline void printVec3d(vec3d v) {
  std::cout << v.x << " " << v.y << " " << v.z << "\n";
}
