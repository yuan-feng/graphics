#include "geometry.h"
#include "model.h"
#include "tga_image.h"
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

namespace {
constexpr const int kWidth = 800;
constexpr const int kHeight = 800;
constexpr const int kDepth = 255;
} // namespace

Vec3f m2v(Matrix m) {
  return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

Matrix v2m(Vec3f v) {
  Matrix m(4, 1);
  m[0][0] = v.x;
  m[1][0] = v.y;
  m[2][0] = v.z;
  m[3][0] = 1.f;
  return m;
}

Matrix viewport(int x, int y, int w, int h) {
  Matrix m = Matrix::Identity(4);
  m[0][3] = x + w / 2.f;
  m[1][3] = y + h / 2.f;
  m[2][3] = kDepth / 2.f;

  m[0][0] = w / 2.f;
  m[1][1] = h / 2.f;
  m[2][2] = kDepth / 2.f;
  return m;
}

void DrawTriangle(Model *model, Vec3i t0, Vec3i t1, Vec3i t2, Vec2i uv0,
                  Vec2i uv1, Vec2i uv2, TGAImage &image, float intensity,
                  std::array<float, kWidth * kHeight> &zbuffer) {
  if (t0.y == t1.y && t0.y == t2.y)
    return;
  if (t0.y > t1.y) {
    std::swap(t0, t1);
    std::swap(uv0, uv1);
  }
  if (t0.y > t2.y) {
    std::swap(t0, t2);
    std::swap(uv0, uv2);
  }
  if (t1.y > t2.y) {
    std::swap(t1, t2);
    std::swap(uv1, uv2);
  }

  int total_height = t2.y - t0.y;
  for (int i = 0; i < total_height; i++) {
    bool second_half = i > t1.y - t0.y || t1.y == t0.y;
    int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
    float alpha = (float)i / total_height;
    float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
    Vec3i A = t0 + Vec3f(t2 - t0) * alpha;
    Vec3i B =
        second_half ? t1 + Vec3f(t2 - t1) * beta : t0 + Vec3f(t1 - t0) * beta;
    Vec2i uvA = uv0 + (uv2 - uv0) * alpha;
    Vec2i uvB =
        second_half ? uv1 + (uv2 - uv1) * beta : uv0 + (uv1 - uv0) * beta;
    if (A.x > B.x) {
      std::swap(A, B);
      std::swap(uvA, uvB);
    }
    for (int j = A.x; j <= B.x; j++) {
      float phi = B.x == A.x ? 1. : (float)(j - A.x) / (float)(B.x - A.x);
      Vec3i P = Vec3f(A) + Vec3f(B - A) * phi;
      Vec2i uvP = uvA + (uvB - uvA) * phi;
      int idx = P.x + P.y * kWidth;
      if (zbuffer[idx] < P.z) {
        zbuffer[idx] = P.z;
        TGAColor color = model->Diffuse(uvP);
        image.Set(P.x, P.y,
                  TGAColor(color.r * intensity, color.g * intensity,
                           color.b * intensity));
      }
    }
  }
}

int main(int argc, char **argv) {
  std::unique_ptr<Model> model;
  if (2 == argc) {
    model = std::make_unique<Model>(argv[1]);
  } else {
    model = std::make_unique<Model>("../obj/african_head.obj");
  }

  std::array<float, kWidth * kHeight> zbuffer;
  std::fill(zbuffer.begin(), zbuffer.end(), -std::numeric_limits<float>::max());
  const Vec3f light_dir(0, 0, -1);
  const Vec3f camera(0, 0, 3);

  Matrix Projection = Matrix::Identity(4);
  Matrix ViewPort =
      viewport(kWidth / 8, kHeight / 8, kWidth * 3 / 4, kHeight * 3 / 4);
  Projection[3][2] = -1.f / camera.z;

  TGAImage image(kWidth, kHeight, TGAImage::RGB);
  for (size_t i = 0; i < model->nfaces(); i++) {
    std::vector<size_t> face = model->face(i);
    Vec3i screen_coords[3];
    Vec3f world_coords[3];
    for (int j = 0; j < 3; j++) {
      Vec3f v = model->vert(face[j]);
      screen_coords[j] = m2v(ViewPort * Projection * v2m(v));
      world_coords[j] = v;
    }
    Vec3f n = (world_coords[2] - world_coords[0]) ^
              (world_coords[1] - world_coords[0]);
    n.Normalize();
    float intensity = n * light_dir;
    if (intensity > 0) {
      Vec2i uv[3];
      for (int k = 0; k < 3; k++) {
        uv[k] = model->uv(i, k);
      }
      DrawTriangle(model.get(), screen_coords[0], screen_coords[1],
                   screen_coords[2], uv[0], uv[1], uv[2], image, intensity,
                   zbuffer);
    }
  }

  image.FlipVertically();
  image.WriteTgaFile("output.tga");

  TGAImage depth_image(kWidth, kHeight, TGAImage::GRAYSCALE);
  for (int i = 0; i < kWidth; i++) {
    for (int j = 0; j < kHeight; j++) {
      depth_image.Set(i, j, TGAColor(zbuffer[i + j * kWidth], 1));
    }
  }
  depth_image.FlipVertically();
  depth_image.WriteTgaFile("depth_image.tga");
  return 0;
}
