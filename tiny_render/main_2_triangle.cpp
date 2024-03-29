#include "tga_image.h"

#include <array>
#include <iostream>
#include <memory>

#include "model.h"

void DrawTriangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image,
                  const TGAColor &color) {
  if (t0.y > t1.y) {
    std::swap(t0, t1);
  }
  if (t0.y > t2.y) {
    std::swap(t0, t2);
  }
  if (t1.y > t2.y) {
    std::swap(t1, t2);
  }
  const int total_height = t2.y - t0.y;
  const int segment_height_a = t1.y - t0.y + 1;
  for (int y = t0.y; y < t1.y; ++y) {
    const float alpha = static_cast<float>(y - t0.y) / segment_height_a;
    const float beta = static_cast<float>(y - t0.y) / total_height;
    Vec2i p1 = t0 * (1 - alpha) + t1 * alpha;
    Vec2i p2 = t0 * (1 - beta) + t2 * beta;
    if (p1.x > p2.x) {
      std::swap(p1, p2);
    }
    for (int x = p1.x; x < p2.x; ++x) {
      image.Set(x, y, color);
    }
  }

  const int segment_height_b = t2.y - t1.y + 1;
  for (int y = t1.y; y < t2.y; ++y) {
    const float alpha = static_cast<float>(t2.y - y) / segment_height_b;
    const float beta = static_cast<float>(t2.y - y) / total_height;
    Vec2i p1 = t2 * (1 - alpha) + t1 * alpha;
    Vec2i p2 = t2 * (1 - beta) + t0 * beta;
    if (p1.x > p2.x) {
      std::swap(p1, p2);
    }
    for (int x = p1.x; x < p2.x; ++x) {
      image.Set(x, y, color);
    }
  }
}

#pragma warning(push)
#pragma warning(disable : 4244)
Vec3f GetBarycentricCoordinates(std::array<Vec2i, 3> pts, Vec2i P) {
  const Vec3f u =
      Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x) ^
      Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y);
  if (std::abs(u.z) < 1)
    return Vec3f(-1, 1, 1);
  return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}
#pragma warning(pop)

void DrawTriangle(std::array<Vec2i, 3> pts, TGAImage &image,
                  const TGAColor &color) {
  Vec2i bboxmin(image.width() - 1, image.height() - 1);
  Vec2i bboxmax(0, 0);
  Vec2i clamp(image.width() - 1, image.height() - 1);
  for (int i = 0; i < 3; i++) {
    bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
    bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

    bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
    bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
  }
  Vec2i P;
  for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
    for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
      const Vec3f bc_screen = GetBarycentricCoordinates(pts, P);
      if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
        continue;
      image.Set(P.x, P.y, color);
    }
  }
}

int main(int argc, char **argv) {
  std::unique_ptr<Model> model;
  if (2 == argc) {
    model = std::make_unique<Model>(argv[1]);
  } else {
    model = std::make_unique<Model>("obj/african_head.obj");
  }

  constexpr const int width = 800;
  constexpr const int height = 800;
  TGAImage image(width, height, TGAImage::RGB);

  Vec3f light_dir(0, 0, -1); // define light_dir

  for (int i = 0; i < model->nfaces(); i++) {
    std::vector<size_t> face = model->face(i);
    Vec2i screen_coords[3];
    Vec3f world_coords[3];
    for (int j = 0; j < 3; j++) {
      Vec3f v = model->vert(face[j]);
      screen_coords[j] = Vec2i(static_cast<int>((v.x + 1.) * width / 2.),
                               static_cast<int>((v.y + 1.) * height / 2.));
      world_coords[j] = v;
    }
    Vec3f n = (world_coords[2] - world_coords[0]) ^
              (world_coords[1] - world_coords[0]);
    n.Normalize();
    float intensity = n * light_dir;
    if (intensity > 0) {
      const auto intensity_v = static_cast<unsigned char>(intensity * 255);
      DrawTriangle(screen_coords[0], screen_coords[1], screen_coords[2], image,
                   TGAColor(intensity_v, intensity_v, intensity_v, 255));
    }
  }
  image.FlipVertically();
  image.WriteTgaFile("output.tga");
  return 0;
}
