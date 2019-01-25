#include <SFML/Graphics.hpp>
#include <math.h>
#include <algorithm>
using namespace std;

#include "materials.h"

Solid::Solid (sf::Color color) {
    this->color = color;
}

sf::Color Solid::getColor (Vector3 coord) {
    return this->color;
}

Texture::Texture (char* filename, float tiling) {
    this->tiling = tiling;
    this->image.loadFromFile(filename);
}

sf::Color Texture::getColor (Vector3 coord) {
    sf::Color color = this->image.getPixel((unsigned int)(coord.x*this->image.getSize().x*tiling)%this->image.getSize().x, this->image.getSize().y-((unsigned int)(coord.y*this->image.getSize().y*tiling)%this->image.getSize().y)-1);
    return color;
}