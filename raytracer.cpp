#include <SFML/Graphics.hpp>
#include <iostream>
#include <limits>
#include <math.h>
#include <algorithm>
#include <ctime>
#include <bits/stdc++.h>
#include <omp.h>
using namespace std;

#include "vector.h"
#include "objects.h"
#include "materials.h"

const unsigned int WIDTH = 400;
const unsigned int HEIGHT = 400;
const unsigned int MAX_RECURSIONS = 3;
const unsigned int SHADOW_SAMPLE_GRID_SIZE = 4;
const unsigned int LIGHTING_SAMPLE_GRID_SIZE = 3;
sf::Color BACKGROUND (16,16,16,255);

static sf::Uint8* pixels = new sf::Uint8[WIDTH*HEIGHT*4];

Primitive* objects [] = {new Sphere (Vector3 (30,0,40), 50.0, new Solid(sf::Color(255,255,255,255)), new Texture("scratched.jpg", 1), 0.9, 1.0, 1.0),
                         new Sphere (Vector3 (-50,-20,25), 30.0, new Solid(sf::Color(0,255,0,255)), new Solid(sf::Color(127,127,127,255)), 0.2, 1.0, 0.4),
                         new Sphere (Vector3 (-15,-20,-25), 30.0, new Texture("earth.bmp", 1), new Texture("rough.png", 1), 0.0, 0.8, 0.5),
                         new Plane (Vector3 (0,-50,0), Vector3 (0,1,0), 200, 200, new Solid(sf::Color(255,255,255,255)), new Texture("plate.jpg", 3), 0.7, 1.0, 1.0)};
Primitive* lights  [] = {new Plane (Vector3 (0, 90, 0), Vector3 (0,-1,0), 100, 100, new Solid(sf::Color(255,255,255,255)), new Solid(sf::Color(127,127,127,255)), 0.0, 0.0, 0.0)};//,
                         //new Point (Vector3 (50, 20, -200), new Solid(sf::Color(255,255,255,255)), new Solid(sf::Color(127,127,127,255)), 0.0, 0.0, 0.0)};

Ray rays [WIDTH*HEIGHT];
sf::Color colors [WIDTH*HEIGHT];

Ray reflect (Vector3 incident, Vector3 intersect, Vector3 normal) {
    incident = incident.normalize();
    normal = normal.normalize();

    Vector3 reflected = incident - normal.scale(2.0*normal.dot(incident));
    Vector3 origin = intersect + reflected.scale(0.001);

    Ray out (origin, reflected.normalize());

    return out;
}

float clamp (float x, float upper, float lower)
{
    return min(upper, max(x, lower));
}

struct Intersect {
    double distance;
    Primitive* closest;
};

Intersect getFirstIntersect (Ray ray) {
    double distance = numeric_limits<double>::infinity();
    Primitive* closest;
    for( unsigned int i = 0; i < sizeof(objects)/sizeof(objects[0]); i += 1 ) {
        Primitive* o = objects[i];
        float d = o->intersect(ray);
        if (d != 0 && d < distance) {
            distance = d;
            closest = o;
        }
    }
    Intersect i;
    i.distance = distance;
    i.closest = closest;

    return i;
}

sf::Color trace (Ray& ray, int recursion_depth) {
    Intersect first = getFirstIntersect(ray);
    double distance = first.distance;
    Primitive* closest = first.closest;

    for( unsigned int i = 0; i < sizeof(lights)/sizeof(lights[0]); i += 1 ) {
        Primitive* l = lights[i];
        if ((l->intersect(ray) > 0) and (l->intersect(ray) <= distance)) {return l->material->getColor(l->getSurfCoords(ray.origin+(ray.direction.normalize().scale(distance))));}
    }

    if (distance >= numeric_limits<double>::infinity()) {
        return BACKGROUND;
    }

    Vector3 scaled = ray.direction.normalize().scale(distance);
    Vector3 intersect = ray.origin + scaled;
    Vector3 normal = closest->getNormal(intersect).normalize();

    if (normal.dot(ray.direction) > 0) {
        normal = normal.scale(1);
    }

    normal = normal+closest->getTangentAxis(intersect).scale((closest->normalMap->getColor(closest->getSurfCoords(intersect)).r/-255.0)+0.5)+closest->getBitangentAxis(intersect).scale((closest->normalMap->getColor(closest->getSurfCoords(intersect)).g/-255.0)+0.5).scale(0.1);

    float diffuse = 0;
    float specular = 0;
    for( unsigned int i = 0; i < sizeof(lights)/sizeof(lights[0]); i += 1 ) {
        Primitive* l = lights[i];
        for (unsigned int y = 0; y < LIGHTING_SAMPLE_GRID_SIZE; y += 1){
            for (unsigned int x = 0; x < LIGHTING_SAMPLE_GRID_SIZE; x += 1){
                float x2 = ((float)x/(float)LIGHTING_SAMPLE_GRID_SIZE);
                float y2 = ((float)y/(float)LIGHTING_SAMPLE_GRID_SIZE);
                x2 += ((float)rand()/(float)RAND_MAX)-0.5;
                y2 += ((float)rand()/(float)RAND_MAX)-0.5;
                diffuse += (l->getWorldCoord(l->getCenter(), Vector3 (x2,y2,0)) - intersect).normalize().dot(normal);
                specular += (l->getWorldCoord(l->getCenter(), Vector3 (x2,y2,0)) - intersect).normalize().dot(reflect(ray.direction, intersect, normal).direction);
            }
        }
    }

    diffuse /= (float)((float)(sizeof(lights)/sizeof(lights[0]))*(float)(LIGHTING_SAMPLE_GRID_SIZE*LIGHTING_SAMPLE_GRID_SIZE));
    specular /= (float)((float)(sizeof(lights)/sizeof(lights[0]))*(float)(LIGHTING_SAMPLE_GRID_SIZE*LIGHTING_SAMPLE_GRID_SIZE));

    if (diffuse < 0) {diffuse = 0;}
    if (specular < 0) {specular = 0;}
    specular = pow(specular, 40);

    float shadowIntensity = 0;
    for( unsigned int i = 0; i < sizeof(lights)/sizeof(lights[0]); i += 1 ) {
        Primitive* l = lights[i];
        for (unsigned int y = 0; y < SHADOW_SAMPLE_GRID_SIZE; y += 1){
            for (unsigned int x = 0; x < SHADOW_SAMPLE_GRID_SIZE; x += 1){
                float x2 = ((float)x/(float)SHADOW_SAMPLE_GRID_SIZE);
                float y2 = ((float)y/(float)SHADOW_SAMPLE_GRID_SIZE);
                x2 += ((float)rand()/(float)RAND_MAX)-0.5;
                y2 += ((float)rand()/(float)RAND_MAX)-0.5;
                Ray shadowRay (intersect+normal.scale(0.001), (l->getWorldCoord(l->getCenter(), Vector3 (x2,y2,0))-intersect).normalize());
                Intersect obstruction = getFirstIntersect(shadowRay);
                if (obstruction.distance < (l->getWorldCoord(l->getCenter(), Vector3 (x2,y2,0))-intersect).magnitude()) {
                    shadowIntensity += 1.0/(float)(SHADOW_SAMPLE_GRID_SIZE*SHADOW_SAMPLE_GRID_SIZE);
                }
            }
        }
    }
    shadowIntensity /= (float)(sizeof(lights)/sizeof(lights[0]));

    sf::Color out = closest->material->getColor(closest->getSurfCoords(intersect));
    out.r = clamp(((out.r*diffuse*closest->kd)+(255*specular*closest->ks))*(1.0-shadowIntensity), 255, 0);
    out.g = clamp(((out.g*diffuse*closest->kd)+(255*specular*closest->ks))*(1.0-shadowIntensity), 255, 0);
    out.b = clamp(((out.b*diffuse*closest->kd)+(255*specular*closest->ks))*(1.0-shadowIntensity), 255, 0);

    if ((recursion_depth+1 < MAX_RECURSIONS) and (closest->reflectivity > 0)) {
        Ray reflection = reflect(ray.direction, intersect, normal);
        sf::Color reflected_color;
        reflected_color = trace(reflection, recursion_depth+1);
        out.r = clamp((out.r*(1.0-closest->reflectivity))+(reflected_color.r*closest->reflectivity*(1.0-shadowIntensity)), 255, 0);
        out.g = clamp((out.g*(1.0-closest->reflectivity))+(reflected_color.g*closest->reflectivity*(1.0-shadowIntensity)), 255, 0);
        out.b = clamp((out.b*(1.0-closest->reflectivity))+(reflected_color.b*closest->reflectivity*(1.0-shadowIntensity)), 255, 0);
    } else {
        out.r = clamp((out.r*(1.0-closest->reflectivity)), 255, 0);
        out.g = clamp((out.g*(1.0-closest->reflectivity)), 255, 0);
        out.b = clamp((out.b*(1.0-closest->reflectivity)), 255, 0);
    }

    return out;
}

int main () {
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Raytracer");
    sf::Texture texture;
    texture.create(WIDTH, HEIGHT);
    sf::Sprite sprite(texture);

    #ifdef _OPENMP
    printf("OpenMP enabled\n");
    omp_set_num_threads(4);
    #endif

    int frame = 0;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Black);

        float theta = frame*((2.0*M_PI)/400.0);
        Vector3 cameraRot (0,-theta,0);
        Vector3 cameraPos (150.0*sin(theta), 0.0, 150.0*-cos(theta));

        for(int y = 0; y < WIDTH; y++) {
            for(int x = 0; x < HEIGHT; x++) {
                    Vector3 direction = Vector3(((float)x/(float)WIDTH)-0.5, ((float)-y/(float)HEIGHT)+0.5, 1.0);
                    direction = direction.rotate(cameraRot);
                    Ray ray (cameraPos, direction);
                    int val = ((y*WIDTH)+x);
                    rays[val] = ray;
            }
        }

        clock_t time = clock();

        for(int y = 0; y < HEIGHT; y++) {
            int x;
            int colors_temp [WIDTH*3];
            #pragma omp parallel for private(x)
            for (x = 0; x < WIDTH; x++) {
                int val = ((y*WIDTH)+x);
                sf::Color c = trace(rays[val], 0);
                colors[val] = c;
            }
        }

        printf("%f seconds\n", clock() - time);

        for(int y = 0; y < WIDTH; y++) {
            for(int x = 0; x < HEIGHT; x++) {
                int val = ((y*WIDTH)+x);
                pixels[(val*4)+0] = colors[val].r;
                pixels[(val*4)+1] = colors[val].g;
                pixels[(val*4)+2] = colors[val].b;
                pixels[(val*4)+3] = 255;
            }
        }

        texture.update(pixels);
        window.draw(sprite);
        window.display();

        frame += 1;
    }

    return 0;
}