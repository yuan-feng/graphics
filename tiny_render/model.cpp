#include "model.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

Model::Model(const char *filename) : verts_(), faces_() {
  std::ifstream in;
  in.open(filename, std::ifstream::in);
  if (in.fail())
    return;
  std::string line;
  while (!in.eof()) {
    std::getline(in, line);
    std::istringstream iss(line.c_str());
    char trash;
    if (!line.compare(0, 2, "v ")) {
      iss >> trash;
      Vec3f v;
      for (int i = 0; i < 3; i++)
        iss >> v.raw[i];
      verts_.push_back(v);
    } else if (!line.compare(0, 2, "f ")) {
      std::vector<Vec3i> f;
      Vec3i tmp;
      iss >> trash;
      while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
        // in wavefront obj all indices start at 1, not zero
        for (int i = 0; i < 3; ++i) {
          tmp[i]--;
        }
        f.push_back(tmp);
      }
      faces_.push_back(f);
    } else if (!line.compare(0, 3, "vt ")) {
      iss >> trash >> trash;
      Vec2f uv;
      iss >> uv[0] >> uv[1];
      uv_.push_back(uv);
    }
  }
  std::cout << "Loaded # v# " << verts_.size() << " f# " << faces_.size()
            << " vt# " << uv_.size() << std::endl;
}

std::vector<size_t> Model::face(size_t idx) const {
  std::vector<size_t> ans;
  ans.reserve(faces_[idx].size());
  for (size_t i = 0; i < faces_[idx].size(); ++i) {
    ans.push_back(faces_[idx][i][0]);
  }
  return ans;
}

Vec2f Model::uv(size_t face_id, size_t vertex_id) const {
  return uv_[faces_[face_id][vertex_id][1]];
}