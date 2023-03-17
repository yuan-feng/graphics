#include "geometry.h"
#include "model.h"
#include "tga_image.h"
#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>
#include <vector>

namespace {
constexpr const int width = 800;
constexpr const int height = 800;
} // namespace

Vec3f GetBarycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
  Vec3f s[2];
  for (int i = 2; i--;) {
    s[i][0] = C[i] - A[i];
    s[i][1] = B[i] - A[i];
    s[i][2] = A[i] - P[i];
  }
  Vec3f u = cross(s[0], s[1]);
  if (std::abs(u[2]) > 1e-2) {
    return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
  }
  return Vec3f(-1, 1, 1);
}

void DrawTriangle(const Vec3f *pts, std::array<float, width * height> &zbuffer,
                  TGAImage &image, const TGAColor &color) {
  Vec2f bboxmin(std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max());
  Vec2f bboxmax(-std::numeric_limits<float>::max(),
                -std::numeric_limits<float>::max());
  Vec2f clamp(image.width() - 1, image.height() - 1);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
      bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
    }
  }
  Vec3f P;
  for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
    for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
      Vec3f bc_screen = GetBarycentric(pts[0], pts[1], pts[2], P);
      if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
        continue;
      P.z = 0;
      for (int i = 0; i < 3; i++)
        P.z += pts[i][2] * bc_screen[i];
      if (zbuffer[int(P.x + P.y * width)] < P.z) {
        zbuffer[int(P.x + P.y * width)] = P.z;
        image.Set(P.x, P.y, color);
      }
    }
  }
}

Vec3f WorldToScreen(Vec3f v) {
  return Vec3f(int((v.x + 1.) * width / 2. + .5),
               int((v.y + 1.) * height / 2. + .5), v.z);
}

int main(int argc, char **argv) {
  std::unique_ptr<Model> model;
  if (2 == argc) {
    model = std::make_unique<Model>(argv[1]);
  } else {
    model = std::make_unique<Model>("obj/african_head.obj");
  }

  std::array<float, width * height> zbuffer;
  std::fill(zbuffer.begin(), zbuffer.end(), -std::numeric_limits<float>::max());

  Vec3f light_dir(0, 0, -1); // define light_dir
  TGAImage image(width, height, TGAImage::RGB);
  for (int i = 0; i < model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    Vec3f screen_coords[3];
    Vec3f world_coords[3];
    for (int i = 0; i < 3; i++) {
      world_coords[i] = model->vert(face[i]);
      screen_coords[i] = WorldToScreen(world_coords[i]);
    }

    Vec3f n = (world_coords[2] - world_coords[0]) ^
              (world_coords[1] - world_coords[0]);
    n.Normalize();
    float intensity = n * light_dir;
    const auto intensity_v = static_cast<unsigned char>(intensity * 255);
    const auto color = TGAColor(intensity_v, intensity_v, intensity_v, 255);
    DrawTriangle(screen_coords, zbuffer, image, color);
  }

  image.FlipVertically();
  image.WriteTgaFile("output.tga");
  return 0;
}