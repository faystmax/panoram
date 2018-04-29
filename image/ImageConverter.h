#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

#include "Image.h"
#include "Kernel.h"

class ImageConverter{
public:
  static Image halfReduce(const Image &image);
  static Image bilinearHalfReduce(const Image &image);
  static Image convolution(const Image &image, const Kernel &core);
};

#endif // IMAGECONVERTER_H
