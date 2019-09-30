#ifndef OBJECTS_H
#define OBJECTS_H

#include "vector.h"
#include "materials.h"

class Primitive {
    public:
        Material* material;
        Material* normalMap;
        float reflectivity;
        float kd;
        float ks;
        float kt;
        float kr;
        Primitive () {}
        virtual float intersect (Ray&) {return false;}
        virtual Vector3 getNormal (Vector3) {return Vector3 (0,0,0);}
        virtual Vector3 getCenter() {return Vector3 (0,0,0);}
        virtual Vector3 getSurfCoords(Vector3) {return Vector3 (0,0,0);}
        virtual Vector3 getTangentAxis(Vector3) {return Vector3 (0,0,0);}
        virtual Vector3 getBitangentAxis(Vector3) {return Vector3 (0,0,0);}
        virtual Vector3 getWorldCoord(Vector3, Vector3) {return Vector3 (0,0,0);}
        void init (Material*, Material*, float, float, float, float, float);
};

class Point: public Primitive {
    public:
        Vector3 center;
        Point (Vector3, Material*, Material*, float, float, float, float, float);
        float intersect (Ray&) {return 0;}
        Vector3 getCenter() {return this->center;}
        virtual Vector3 getTangentAxis(Vector3) {return Vector3 (0,0,0);}
        virtual Vector3 getBitangentAxis(Vector3) {return Vector3 (0,0,0);}
        virtual Vector3 getWorldCoord(Vector3, Vector3) {return this->center;}
};

class Sphere: public Primitive {
    public:
        Vector3 center;
        float radius;
        Sphere (Vector3, float, Material*, Material*, float, float, float, float, float);
        float intersect (Ray&);
        Vector3 getNormal (Vector3);
        Vector3 getCenter() {return this->center;}
        Vector3 getSurfCoords(Vector3);
        Vector3 getTangentAxis(Vector3);
        Vector3 getBitangentAxis(Vector3);
        Vector3 getWorldCoord(Vector3, Vector3);
};

class Plane: public Primitive {
    public:
        Vector3 center;
        Vector3 normal;
        float width;
        float height;
        Plane (Vector3, Vector3, float, float, Material*, Material*, float, float, float, float, float);
        float intersect (Ray&);
        Vector3 getNormal (Vector3);
        Vector3 getCenter() {return this->center;}
        Vector3 getSurfCoords(Vector3);
        Vector3 getTangentAxis(Vector3);
        Vector3 getBitangentAxis(Vector3);
        Vector3 getWorldCoord(Vector3, Vector3);
};

#endif