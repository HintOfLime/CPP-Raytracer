#ifndef MATERIALS_H
#define MATERIALS_H

#include "vector.h"

class Material {
    public:
        virtual sf::Color getColor (Vector3) {return sf::Color (0,0,0,255);}
};

class Solid: public Material {
    public:
        Solid (sf::Color);
        sf::Color getColor (Vector3);
        sf::Color color;
};

class Texture: public Material {
    public:
        Texture (char*, float);
        float tiling;
        sf::Color getColor (Vector3);
        sf::Image image;
};

#endif