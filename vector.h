#ifndef VECTOR_H
#define VECTOR_H

class Vector3 {
    public:
        float x, y, z;
        Vector3 () {}
        Vector3 (float, float, float);
        Vector3 operator+(Vector3);
        Vector3 operator-(Vector3);
        float dot (Vector3);
        Vector3 cross (Vector3);
        float magnitude ();
        Vector3 scale (float);
        Vector3 normalize ();
        Vector3 rotate(Vector3);
};

class Ray {
    public:
        Vector3 origin, direction;
        Ray (Vector3, Vector3);
        Ray () {origin = Vector3 (0,0,0); direction = Vector3 (0,0,0);}
};

#endif