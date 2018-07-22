#include <math.h>
#include "vector.h"

Vector3::Vector3 (float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

Vector3 Vector3::operator+ (Vector3 b) {
    Vector3 out (this->x + b.x,
                 this->y + b.y,
                 this->z + b.z);
    return out;
}

Vector3 Vector3::operator- (Vector3 b) {
    Vector3 out (this->x - b.x,
                 this->y - b.y,
                 this->z - b.z);
    return out;
}

float Vector3::dot (Vector3 b) {
    return (this->x*b.x) + (this->y*b.y) + (this->z*b.z);
}

Vector3 Vector3::cross (Vector3 b) {
    return Vector3 ((this->y*b.z) - (this->z*b.y),
                    (this->z*b.x) - (this->x*b.z),
                    (this->x*b.y) - (this->y*b.x));
}

float Vector3::magnitude () {
    return sqrt((this->x*this->x) + (this->y*this->y) + (this->z*this->z));
}

Vector3 Vector3::scale (float k) {
    Vector3 out (this->x*k, this->y*k, this->z*k);
    return out;
}

Vector3 Vector3::normalize () {
    float d = this->magnitude();
    if (d == 0) {
        return Vector3 (0,0,0);
    }
    return Vector3 (this->x/d, this->y/d, this->z/d);
}

Vector3 Vector3::rotate (Vector3 theta) {
    Vector3 a = this->normalize();

    //[cos,  0,  sin ]   [x]   [cos*x + 0*y + sin*z]
    //[0,    1,  0   ] * [y] = [0*x   + 1*y + 0*z  ]
    //[-sin, 0,  cos ]   [z]   [-sin*x + 0*y + cos*z]

    // Rotate around y axis
    if (theta.y != 0) {
        Vector3 b   ((a.x*cos(theta.y))+(a.z*sin(theta.y)),
                    a.y,
                    (a.x*-sin(theta.y))+(a.z*cos(theta.y)));
        a = b.normalize();
    }

    // Rotate around x axis
    if (theta.x != 0) {
        Vector3 c   (a.x,
                    (a.y*cos(theta.x))+(a.z*-sin(theta.x)),
                    (a.y*sin(theta.x))+(a.z*cos(theta.x)));
        a = c.normalize();
    }

    // Rotate around z axis
    if (theta.z != 0) {
        Vector3 d   ((a.x*cos(theta.z))+(a.y*-sin(theta.z)),
                    (a.x*sin(theta.z))+(a.y*cos(theta.z)),
                    a.z);
        a = d.normalize();
    }

    return a;
}

Ray::Ray (Vector3 origin, Vector3 direction) {
    this->origin = origin;
    this->direction = direction;
}