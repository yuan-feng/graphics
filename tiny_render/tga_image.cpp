#include <math.h>
#include <string.h>
#include <time.h>

#include <fstream>
#include <iostream>

#include "tga_image.h"

TGAImage::TGAImage()
    : data_(NULL), width_(0), height_(0), bytes_per_pixel_(0) {}

TGAImage::TGAImage(int w, int h, int bpp)
    : data_(NULL), width_(w), height_(h), bytes_per_pixel_(bpp) {
  unsigned long nbytes = width_ * height_ * bytes_per_pixel_;
  data_ = new unsigned char[nbytes];
  memset(data_, 0, nbytes);
}

TGAImage::TGAImage(const TGAImage &img) {
  width_ = img.width_;
  height_ = img.height_;
  bytes_per_pixel_ = img.bytes_per_pixel_;
  unsigned long nbytes = width_ * height_ * bytes_per_pixel_;
  data_ = new unsigned char[nbytes];
  memcpy(data_, img.data_, nbytes);
}

TGAImage::~TGAImage() {
  if (data_)
    delete[] data_;
}

TGAImage &TGAImage::operator=(const TGAImage &img) {
  if (this != &img) {
    if (data_)
      delete[] data_;
    width_ = img.width_;
    height_ = img.height_;
    bytes_per_pixel_ = img.bytes_per_pixel_;
    unsigned long nbytes = width_ * height_ * bytes_per_pixel_;
    data_ = new unsigned char[nbytes];
    memcpy(data_, img.data_, nbytes);
  }
  return *this;
}

bool TGAImage::ReadTgaFile(const char *filename) {
  if (data_)
    delete[] data_;
  data_ = NULL;
  std::ifstream in;
  in.open(filename, std::ios::binary);
  if (!in.is_open()) {
    std::cerr << "can't open file " << filename << "\n";
    in.close();
    return false;
  }
  TGAHeader header;
  in.read((char *)&header, sizeof(header));
  if (!in.good()) {
    in.close();
    std::cerr << "an error occured while reading the header\n";
    return false;
  }
  width_ = header.width;
  height_ = header.height;
  bytes_per_pixel_ = header.bits_per_pixel >> 3;
  if (width_ <= 0 || height_ <= 0 ||
      (bytes_per_pixel_ != GRAYSCALE && bytes_per_pixel_ != RGB &&
       bytes_per_pixel_ != RGBA)) {
    in.close();
    std::cerr << "bad bpp (or width/height) value\n";
    return false;
  }
  unsigned long nbytes = bytes_per_pixel_ * width_ * height_;
  data_ = new unsigned char[nbytes];
  if (3 == header.datatype_code || 2 == header.datatype_code) {
    in.read((char *)data_, nbytes);
    if (!in.good()) {
      in.close();
      std::cerr << "an error occured while reading the data\n";
      return false;
    }
  } else if (10 == header.datatype_code || 11 == header.datatype_code) {
    if (!LoadRleData(in)) {
      in.close();
      std::cerr << "an error occured while reading the data\n";
      return false;
    }
  } else {
    in.close();
    std::cerr << "unknown file format " << (int)header.datatype_code << "\n";
    return false;
  }
  if (!(header.image_descriptor & 0x20)) {
    FlipVertically();
  }
  if (header.image_descriptor & 0x10) {
    FlipHorizontally();
  }
  std::cerr << width_ << "x" << height_ << "/" << bytes_per_pixel_ * 8 << "\n";
  in.close();
  return true;
}

bool TGAImage::LoadRleData(std::ifstream &in) {
  unsigned long pixel_count = width_ * height_;
  unsigned long current_pixel = 0;
  unsigned long current_byte = 0;
  TGAColor color_buffer;
  do {
    unsigned char chunk_header = 0;
    chunk_header = in.get();
    if (!in.good()) {
      std::cerr << "an error occured while reading the data\n";
      return false;
    }
    if (chunk_header < 128) {
      chunk_header++;
      for (int i = 0; i < chunk_header; i++) {
        in.read((char *)color_buffer.raw, bytes_per_pixel_);
        if (!in.good()) {
          std::cerr << "an error occured while reading the header\n";
          return false;
        }
        for (int t = 0; t < bytes_per_pixel_; t++)
          data_[current_byte++] = color_buffer.raw[t];
        current_pixel++;
        if (current_pixel > pixel_count) {
          std::cerr << "Too many pixels read\n";
          return false;
        }
      }
    } else {
      chunk_header -= 127;
      in.read((char *)color_buffer.raw, bytes_per_pixel_);
      if (!in.good()) {
        std::cerr << "an error occured while reading the header\n";
        return false;
      }
      for (int i = 0; i < chunk_header; i++) {
        for (int t = 0; t < bytes_per_pixel_; t++)
          data_[current_byte++] = color_buffer.raw[t];
        current_pixel++;
        if (current_pixel > pixel_count) {
          std::cerr << "Too many pixels read\n";
          return false;
        }
      }
    }
  } while (current_pixel < pixel_count);
  return true;
}

bool TGAImage::WriteTgaFile(const char *filename, bool rle) {
  unsigned char developer_area_ref[4] = {0, 0, 0, 0};
  unsigned char extension_area_ref[4] = {0, 0, 0, 0};
  unsigned char footer[18] = {'T', 'R', 'U', 'E', 'V', 'I', 'S', 'I', 'O',
                              'N', '-', 'X', 'F', 'I', 'L', 'E', '.', '\0'};
  std::ofstream out;
  out.open(filename, std::ios::binary);
  if (!out.is_open()) {
    std::cerr << "can't open file " << filename << "\n";
    out.close();
    return false;
  }
  TGAHeader header;
  memset((void *)&header, 0, sizeof(header));
  header.bits_per_pixel = bytes_per_pixel_ << 3;
  header.width = width_;
  header.height = height_;
  header.datatype_code =
      (bytes_per_pixel_ == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));
  header.image_descriptor = 0x20; // top-left origin
  out.write((char *)&header, sizeof(header));
  if (!out.good()) {
    out.close();
    std::cerr << "can't dump the tga file\n";
    return false;
  }
  if (!rle) {
    out.write((char *)data_, width_ * height_ * bytes_per_pixel_);
    if (!out.good()) {
      std::cerr << "can't unload raw data\n";
      out.close();
      return false;
    }
  } else {
    if (!UnloadRleData(out)) {
      out.close();
      std::cerr << "can't unload rle data\n";
      return false;
    }
  }
  out.write((char *)developer_area_ref, sizeof(developer_area_ref));
  if (!out.good()) {
    std::cerr << "can't dump the tga file\n";
    out.close();
    return false;
  }
  out.write((char *)extension_area_ref, sizeof(extension_area_ref));
  if (!out.good()) {
    std::cerr << "can't dump the tga file\n";
    out.close();
    return false;
  }
  out.write((char *)footer, sizeof(footer));
  if (!out.good()) {
    std::cerr << "can't dump the tga file\n";
    out.close();
    return false;
  }
  out.close();
  return true;
}

// TODO: it is not necessary to break a raw chunk for two equal pixels (for the
// matter of the resulting size)
bool TGAImage::UnloadRleData(std::ofstream &out) {
  const unsigned char max_chunk_length = 128;
  unsigned long npixels = width_ * height_;
  unsigned long curpix = 0;
  while (curpix < npixels) {
    unsigned long chunkstart = curpix * bytes_per_pixel_;
    unsigned long curbyte = curpix * bytes_per_pixel_;
    unsigned char run_length = 1;
    bool raw = true;
    while (curpix + run_length < npixels && run_length < max_chunk_length) {
      bool succ_eq = true;
      for (int t = 0; succ_eq && t < bytes_per_pixel_; t++) {
        succ_eq = (data_[curbyte + t] == data_[curbyte + t + bytes_per_pixel_]);
      }
      curbyte += bytes_per_pixel_;
      if (1 == run_length) {
        raw = !succ_eq;
      }
      if (raw && succ_eq) {
        run_length--;
        break;
      }
      if (!raw && !succ_eq) {
        break;
      }
      run_length++;
    }
    curpix += run_length;
    out.put(raw ? run_length - 1 : run_length + 127);
    if (!out.good()) {
      std::cerr << "can't dump the tga file\n";
      return false;
    }
    out.write((char *)(data_ + chunkstart),
              (raw ? run_length * bytes_per_pixel_ : bytes_per_pixel_));
    if (!out.good()) {
      std::cerr << "can't dump the tga file\n";
      return false;
    }
  }
  return true;
}

TGAColor TGAImage::Get(int x, int y) const {
  if (!data_ || x < 0 || y < 0 || x >= width_ || y >= height_) {
    return TGAColor();
  }
  return TGAColor(data_ + (x + y * width_) * bytes_per_pixel_,
                  bytes_per_pixel_);
}

bool TGAImage::Set(int x, int y, const TGAColor &c) {
  if (!data_ || x < 0 || y < 0 || x >= width_ || y >= height_) {
    return false;
  }
  memcpy(data_ + (x + y * width_) * bytes_per_pixel_, c.raw, bytes_per_pixel_);
  return true;
}

bool TGAImage::FlipHorizontally() {
  if (!data_)
    return false;
  int half = width_ >> 1;
  for (int i = 0; i < half; i++) {
    for (int j = 0; j < height_; j++) {
      TGAColor c1 = Get(i, j);
      TGAColor c2 = Get(width_ - 1 - i, j);
      Set(i, j, c2);
      Set(width_ - 1 - i, j, c1);
    }
  }
  return true;
}

bool TGAImage::FlipVertically() {
  if (!data_)
    return false;
  unsigned long bytes_per_line = width_ * bytes_per_pixel_;
  unsigned char *line = new unsigned char[bytes_per_line];
  int half = height_ >> 1;
  for (int j = 0; j < half; j++) {
    unsigned long l1 = j * bytes_per_line;
    unsigned long l2 = (height_ - 1 - j) * bytes_per_line;
    memmove((void *)line, (void *)(data_ + l1), bytes_per_line);
    memmove((void *)(data_ + l1), (void *)(data_ + l2), bytes_per_line);
    memmove((void *)(data_ + l2), (void *)line, bytes_per_line);
  }
  delete[] line;
  return true;
}

void TGAImage::Clear() {
  memset((void *)data_, 0, width_ * height_ * bytes_per_pixel_);
}

bool TGAImage::Scale(int w, int h) {
  if (w <= 0 || h <= 0 || !data_)
    return false;
  unsigned char *tdata = new unsigned char[w * h * bytes_per_pixel_];
  int nscanline = 0;
  int oscanline = 0;
  int erry = 0;
  unsigned long nlinebytes = w * bytes_per_pixel_;
  unsigned long olinebytes = width_ * bytes_per_pixel_;
  for (int j = 0; j < height_; j++) {
    int errx = width_ - w;
    int nx = -bytes_per_pixel_;
    int ox = -bytes_per_pixel_;
    for (int i = 0; i < width_; i++) {
      ox += bytes_per_pixel_;
      errx += w;
      while (errx >= (int)width_) {
        errx -= width_;
        nx += bytes_per_pixel_;
        memcpy(tdata + nscanline + nx, data_ + oscanline + ox,
               bytes_per_pixel_);
      }
    }
    erry += h;
    oscanline += olinebytes;
    while (erry >= (int)height_) {
      if (erry >= (int)height_ << 1) // it means we jump over a scanline
        memcpy(tdata + nscanline + nlinebytes, tdata + nscanline, nlinebytes);
      erry -= height_;
      nscanline += nlinebytes;
    }
  }
  delete[] data_;
  data_ = tdata;
  width_ = w;
  height_ = h;
  return true;
}
