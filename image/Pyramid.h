#ifndef PYRAMID_H
#define PYRAMID_H

#include "Image.h"
#include "Kernel.h"

struct Item {
        int octave;
        int scale;
        double sigmaScale;
        double sigmaEffect;
        Image image;
        Image trueImage;
        Item(Image image, int octave, double scale, double sigmaScale, double sigmaEffect){
            this->octave = octave;
            this->scale = scale;
            this->sigmaScale = sigmaScale;
            this->sigmaEffect = sigmaEffect;
            this->image = std::move(image);
        }
        Item(Image trueImage,Image image, int octave, double scale, double sigmaScale, double sigmaEffect){
            this->octave = octave;
            this->scale = scale;
            this->sigmaScale = sigmaScale;
            this->sigmaEffect = sigmaEffect;
            this->image = std::move(image);
            this->trueImage = std::move(trueImage);
        }
};

class Pyramid {
public:
    Pyramid() = default;
    Pyramid(const Image &image, const int scales = 7, double sigma = 1.6, double sigmaStart = 1);

    int getItemsSize() const {return items.size();}
    Item getItem(int index) const {return items[index];}
    int getDogsSize() const {return dogs.size();}
    Item& getDog(int index) {return dogs[index];}

private:
    vector<Item> items;
    vector<Item> dogs;

    Image&  getLastImage();
    double getDeltaSigma(double sigmaPrev, double sigmaNext) const;
    Image convultionSeparab(const Image &image, Kernel &&gaussLine);

};

#endif // PYRAMID_H
