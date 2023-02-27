#ifndef GRAPHICS_TINY_READER_MODEL_H_
#define GRAPHICS_TINY_READER_MODEL_H_

#include "geometry.h"
#include <vector>

class Model {
private:
  std::vector<Vec3f> verts_;
  std::vector<std::vector<int>> faces_;

public:
  Model(const char *filename);

  int nverts() { return (int)verts_.size(); }

  int nfaces() { return (int)faces_.size(); }

  std::vector<int> face(int idx) { return faces_[idx]; }

  Vec3f vert(int i) { return verts_[i]; }
};

#endif // GRAPHICS_TINY_READER_MODEL_H_
