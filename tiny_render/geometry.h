#ifndef GRAPHICS_TINY_READER_GEOMETRY_H_
#define GRAPHICS_TINY_READER_GEOMETRY_H_

#include <cmath>
#include <ostream>

#pragma warning(push)
#pragma warning(disable : 4244)
template <typename t> struct Vec2 {
  union {
    struct {
      t u, v;
    };
    struct {
      t x, y;
    };
    t raw[2];
  };
  Vec2() : u(0), v(0) {}
  Vec2(t _u, t _v) : u(_u), v(_v) {}
  inline Vec2<t> operator+(const Vec2<t> &V) const {
    return Vec2<t>(u + V.u, v + V.v);
  }
  inline Vec2<t> operator-(const Vec2<t> &V) const {
    return Vec2<t>(u - V.u, v - V.v);
  }
  inline Vec2<t> operator*(float f) const { return Vec2<t>(u * f, v * f); }
  inline t operator[](size_t i) const { return raw[i]; }
  inline t &operator[](size_t i) { return raw[i]; }
  template <typename>
  friend std::ostream &operator<<(std::ostream &s, Vec2<t> &v);
};
#pragma warning(pop)

template <typename t> struct Vec3 {
  union {
    struct {
      t x, y, z;
    };
    struct {
      t ivert, iuv, inorm;
    };
    t raw[3];
  };
  Vec3() : x(0), y(0), z(0) {}
  Vec3(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
  inline Vec3<t> operator^(const Vec3<t> &v) const {
    return Vec3<t>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
  }
  inline Vec3<t> operator+(const Vec3<t> &v) const {
    return Vec3<t>(x + v.x, y + v.y, z + v.z);
  }
  inline Vec3<t> operator-(const Vec3<t> &v) const {
    return Vec3<t>(x - v.x, y - v.y, z - v.z);
  }
  inline Vec3<t> operator*(float f) const {
    return Vec3<t>(x * f, y * f, z * f);
  }
  inline t operator[](size_t i) const {
    return raw[i];
  }
  inline t& operator[](size_t i) {
    return raw[i];
  }
  inline t operator*(const Vec3<t> &v) const {
    return x * v.x + y * v.y + z * v.z;
  }
  float Norm() const { return std::sqrt(x * x + y * y + z * z); }
  Vec3<t> &Normalize(t l = 1) {
    *this = (*this) * (l / Norm());
    return *this;
  }
  template <typename>
  friend std::ostream &operator<<(std::ostream &s, Vec3<t> &v);
};

typedef Vec2<float> Vec2f;
typedef Vec2<int> Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;

template <typename T> Vec3<T> cross(Vec3<T> v1, Vec3<T> v2) {
  return Vec3<T>(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

template <typename t> std::ostream &operator<<(std::ostream &s, Vec2<t> &v) {
  s << "(" << v.x << ", " << v.y << ")\n";
  return s;
}

template <typename t> std::ostream &operator<<(std::ostream &s, Vec3<t> &v) {
  s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
  return s;
}

#endif // GRAPHICS_TINY_READER_GEOMETRY_H_