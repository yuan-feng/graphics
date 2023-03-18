#ifndef GRAPHICS_TINY_READER_MODEL_H_
#define GRAPHICS_TINY_READER_MODEL_H_

#include "geometry.h"
#include <vector>

class Model {
private:
  std::vector<Vec3f> verts_;
  std::vector<std::vector<Vec3i>> faces_;
  std::vector<Vec2f> uv_;

public:
  Model(const char *filename);

  std::vector<size_t> face(size_t idx) const;
  Vec2f uv(size_t face_id, size_t vertex_id) const;

  size_t nverts() const { return verts_.size(); }

  size_t nfaces() const { return faces_.size(); }

  Vec3f vert(size_t i) { return verts_[i]; }
};

#endif // GRAPHICS_TINY_READER_MODEL_H_
