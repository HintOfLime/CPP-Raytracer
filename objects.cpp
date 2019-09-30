#include <SFML/Graphics.hpp>
#include <math.h>
#include <algorithm>
using namespace std;

#include "objects.h"

void Primitive::init (Material* material, Material* normalMap, float reflectivity, float kd, float ks, float kt, float kr) {
    this->material = material;
    this->normalMap = normalMap;
    this->reflectivity = reflectivity;
    this->kd = kd; // Diffuse
    this->ks = ks; // Specular
    this->kt = kt; // Transparency
    this->kr = kr; // Refractive Index
}

Point::Point (Vector3 center, Material* material, Material* normalMap, float reflectivity, float kd, float ks, float kt, float kr) {
    this->center = center;
    this->init(material, normalMap, reflectivity, kd, ks, kt, kr);
}

Sphere::Sphere (Vector3 center, float radius, Material* material, Material* normalMap, float reflectivity, float kd, float ks, float kt, float kr) {
    this->center = center;
    this->radius = radius;
    this->init(material, normalMap, reflectivity, kd, ks, kt, kr);
}

float Sphere::intersect (Ray& ray) {
    float A = ray.direction.dot(ray.direction);
    Vector3 dist = ray.origin - this->center;
    float B = 2.0 * dist.dot(ray.direction);
    float C = dist.dot(dist) - (this->radius * this->radius);
    float discr = (B*B) - (4*A*C);

    if (discr < 0.0) {
        return 0;
    }

    float t0 = (-B - sqrt(discr))/(2*A);
    float t1 = (-B + sqrt(discr))/(2*A);
    float d;

    if (t0 < 0 && t1 < 0) {
        d = 0;
    }
    else if (t0 < 0) {
        d = t1;
    }
    else if (t1 < 0) {
        d = t0;
    }
    else {
        d = min(t0, t1);
    }

    return d;
}

Vector3 Sphere::getNormal (Vector3 intersect) {
    return (intersect-this->center).normalize();
}

Vector3 Sphere::getSurfCoords (Vector3 intersect) {
    Vector3 hitVec = intersect - this->center;
    float x = (atan2(hitVec.z, hitVec.x) / (2*M_PI))+0.5;
    float y = 1.0 - (acos(hitVec.y/this->radius) / M_PI);
    return Vector3(x,y,0);
}

Vector3 Sphere::getTangentAxis (Vector3 intersect) {
    return this->getNormal(intersect).cross(Vector3(0,1,0)).normalize();
}

Vector3 Sphere::getBitangentAxis (Vector3 intersect) {
    return this->getNormal(intersect).cross(getTangentAxis(intersect)).normalize();
}

Vector3 Sphere::getWorldCoord(Vector3 intersect, Vector3 offsetCoord) {
    return intersect + this->getTangentAxis(intersect).scale(offsetCoord.x) + this->getBitangentAxis(intersect).scale(offsetCoord.y);
}

Plane::Plane (Vector3 center, Vector3 normal, float width, float height, Material* material, Material* normalMap, float reflectivity, float kd, float ks, float kt, float kr) {
    this->center = center;
    this->normal = normal;
    this->width = width;
    this->height = height;
    this->init(material, normalMap, reflectivity, kd, ks, kt, kr);
}

float Plane::intersect (Ray& ray) {
    float denom = this->normal.dot(ray.direction);
    if (denom < 0.0001 and denom > -0.0001) {return 0;}

    Vector3 v = this->center - ray.origin;;
    float d = v.dot(this->normal) / denom;
    if (d < 0) {return 0;}

    Vector3 scaled = ray.direction.normalize().scale(d);
    Vector3 intersect = ray.origin + scaled;
    Vector3 coord = this->getSurfCoords(intersect);
    if (coord.x > 1.0 or coord.x < 0.0) {return 0;}
    if (coord.y > 1.0 or coord.y < 0.0) {return 0;}

    return d;
}

Vector3 Plane::getNormal (Vector3 intersect) {
    return this->normal.normalize();
}

Vector3 Plane::getSurfCoords(Vector3 intersect) {
    Vector3 x_axis  = this->normal.cross(Vector3(1,0,0));
    if (x_axis.magnitude() == 0) {
        x_axis  = this->normal.cross(Vector3(0,0,1));
    }
    Vector3 y_axis  = this->normal.cross(x_axis);
    Vector3 hitCoord (((intersect-this->center).dot(x_axis)/this->width)+0.5,
                      ((intersect-this->center).dot(y_axis)/this->height)+0.5,
                      0);
    return hitCoord;
}

Vector3 Plane::getTangentAxis (Vector3 intersect) {
    Vector3 x_axis  = this->getNormal(intersect).cross(Vector3(1,0,0));
    if (x_axis.magnitude() == 0) {
        x_axis  = this->getNormal(intersect).cross(Vector3(0,0,1));
    }
    return x_axis.normalize();
}

Vector3 Plane::getBitangentAxis (Vector3 intersect) {
    Vector3 y_axis  = this->getNormal(intersect).cross(this->getTangentAxis(intersect));
    return y_axis.normalize();
}

Vector3 Plane::getWorldCoord(Vector3 intersect, Vector3 offsetCoord) {
    return intersect + this->getTangentAxis(intersect).scale(offsetCoord.x*this->width) + this->getBitangentAxis(intersect).scale(offsetCoord.y*this->height);
}