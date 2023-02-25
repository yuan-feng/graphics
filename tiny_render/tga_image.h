#ifndef GRAPHICS_TINY_READER_TGA_IMAGE_H_
#define GRAPHICS_TINY_READER_TGA_IMAGE_H_

#include <fstream>

#pragma pack(push, 1)
struct TGAHeader {
  char id_length;
  char colormap_type;
  char datatype_code;
  short colormap_origin;
  short colormap_length;
  char colormap_depth;
  short x_origin;
  short y_origin;
  short width;
  short height;
  char bits_per_pixel;
  char image_descriptor;
};
#pragma pack(pop)

struct TGAColor {
  union {
    struct {
      unsigned char b, g, r, a;
    };
    unsigned char raw[4];
    unsigned int val;
  };
  int bytes_per_pixel;

  TGAColor() : val(0), bytes_per_pixel(1) {}

  TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A)
      : b(B), g(G), r(R), a(A), bytes_per_pixel(4) {}

  TGAColor(int v, int bpp) : val(v), bytes_per_pixel(bpp) {}

  TGAColor(const TGAColor &c)
      : val(c.val), bytes_per_pixel(c.bytes_per_pixel) {}

  TGAColor(const unsigned char *p, int bpp) : val(0), bytes_per_pixel(bpp) {
    for (int i = 0; i < bpp; i++) {
      raw[i] = p[i];
    }
  }

  TGAColor &operator=(const TGAColor &c) {
    if (this != &c) {
      bytes_per_pixel = c.bytes_per_pixel;
      val = c.val;
    }
    return *this;
  }
};

class TGAImage {
protected:
  unsigned char *data_;
  int width_;
  int height_;
  int bytes_per_pixel_;

  bool LoadRleData(std::ifstream &in);
  bool UnloadRleData(std::ofstream &out);

public:
  enum Format { GRAYSCALE = 1, RGB = 3, RGBA = 4 };

  TGAImage();
  TGAImage(int w, int h, int bpp);
  TGAImage(const TGAImage &img);
  bool ReadTgaFile(const char *filename);
  bool WriteTgaFile(const char *filename, bool rle = true);
  bool FlipHorizontally();
  bool FlipVertically();
  bool Scale(int w, int h);
  TGAColor Get(int x, int y);
  bool Set(int x, int y, TGAColor c);
  ~TGAImage();
  TGAImage &operator=(const TGAImage &img);
  int width() { return width_; }
  int height() { return height_; }
  int bytes_per_pixel() { return bytes_per_pixel_; }
  unsigned char *data() { return data_;}
  void Clear();
};

#endif //GRAPHICS_TINY_READER_TGA_IMAGE_H_