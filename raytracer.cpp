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

const unsigned int WIDTH = 300;
const unsigned int HEIGHT = 300;
const unsigned int MAX_RECURSIONS = 5;
const float AMBIENT = 0.1;
const unsigned int SHADOW_SAMPLE_GRID_SIZE = 3;
const unsigned int LIGHTING_SAMPLE_GRID_SIZE = 3;
const float BIAS = 0.01;
//sf::Color BACKGROUND (24,24,24,255);

static sf::Uint8* pixels = new sf::Uint8[WIDTH*HEIGHT*4];

Primitive* objects [] = {new Sphere (Vector3 (30,0,40), 50.0, new Solid(sf::Color(255,255,255,255)), new Texture("scratched.jpg", 2), 0.9, 1.0, 1.0, 0, 1.0),
                         new Sphere (Vector3 (-47.5,-25,25), 25.0, new Texture("earth.bmp", 1), new Texture("rough.png", 2), 0.0, 1.0, 0.4, 0.0, 1.0),
                         new Sphere (Vector3 (-17.5,-20,-25), 30.0, new Solid(sf::Color(255,255,255,255)), new Solid(sf::Color(127,127,127,255)), 0.0, 1.0, 0.6, 1.0, 0.9),
                         new Plane (Vector3 (0,-50,0), Vector3 (0,1,0), 300, 300, new Solid(sf::Color(255,255,255,255)), new Texture("plate.jpg", 6), 0.8, 0.7, 0.9, 0.0, 1.0)//,
                         //new Plane (Vector3 (0,100,0), Vector3 (0,-1,0), 300, 300, new Solid(sf::Color(255,255,255,255)), new Texture("rough.png", 6), 0.0, 0.8, 0.2, 0.0, 0.0),
                         //new Plane (Vector3 (-150,-50+125,0), Vector3 (1,0,0), 300, 300, new Solid(sf::Color(255,0,0,255)), new Solid(sf::Color(127,127,127,255)), 0.05, 0.5, 0.2, 0.0, 0.0),
                         //new Plane (Vector3 (150,-50+125,0), Vector3 (-1,0,0), 300, 300, new Solid(sf::Color(0,255,0,255)), new Solid(sf::Color(127,127,127,255)), 0.05, 0.5, 0.2, 0.0, 0.0),
                         //new Plane (Vector3 (0,-50+125,150), Vector3 (0,0,-1), 300, 300, new Solid(sf::Color(0,0,255,255)), new Solid(sf::Color(127,127,127,255)), 0.05, 0.5, 0.2, 0.0, 0.0),
                         //new Plane (Vector3 (0,-50+125,-150), Vector3 (0,0,1), 300, 300, new Solid(sf::Color(255,255,0,255)), new Solid(sf::Color(127,127,127,255)), 0.05, 0.5, 0.2, 0.0, 0.0)
                        };

Primitive* lights  [] = {new Plane (Vector3 (0, 95, 0), Vector3 (0,-1,0), 175, 175, new Solid(sf::Color(255,255,255,255)), new Solid(sf::Color(127,127,127,255)), 0.0, 0.0, 0.0, 0.0, 0.0),
                         new Plane (Vector3 (80, 30, -80), Vector3 (-1,-0.5,1), 50, 50, new Solid(sf::Color(255,255,255,255)), new Solid(sf::Color(127,127,127,255)), 0.0, 0.0, 0.0, 0.0, 0.0)
                         //new Sphere (Vector3 (-100, 50, -100), 20.0, new Solid(sf::Color(255,255,255,255)), new Solid(sf::Color(127,127,127,255)), 0.0, 0.0, 0.0, 0.0, 0.0)
                        };

Material* skybox = new Texture("skybox.png", 1);

Ray rays [WIDTH*HEIGHT];
sf::Color colors [WIDTH*HEIGHT];

Ray reflect (Vector3 incident, Vector3 intersect, Vector3 normal) {
    incident = incident.normalize();
    normal = normal.normalize();

    Vector3 reflected = incident - normal.scale(2.0*normal.dot(incident));
    Vector3 origin = intersect + reflected.scale(BIAS);

    Ray out (origin, reflected.normalize());

    return out;
}

Ray refract(Vector3 incident, Vector3 intersect, Vector3 normal, float refractiveIndex) {
    Vector3 refNorm = normal;
    float n1 = 1.0;
    float n2 = refractiveIndex;
    float idn = incident.dot(normal);
    if (idn < 0.0) {
        idn = -idn;
    } else {
        //normal = normal.scale(-1.0);
        //n1 = refractiveIndex;
        //n2 = 1.0;
    }
    float n = n2/n1;
    float k = 1.0 - (pow(n, 2) * (1.0-pow(idn, 2))); 

    if (k < 0.0) { return reflect(incident, intersect, normal); }
    Vector3 direction = ((incident + refNorm.scale(idn)).scale(n) - refNorm.scale(sqrt(k))).normalize();
    Ray out = Ray(intersect+direction.scale(BIAS), direction);

    return out;
}

float clamp (float x, float upper, float lower) {
    return min(upper, max(x, lower));
}

sf::Color clamp (sf::Color color) {
    color.r = clamp(color.r, 255, 0);
    color.g = clamp(color.g, 255, 0);
    color.b = clamp(color.b, 255, 0);
    return color;
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

sf::Color getSkybox (Vector3 direction) {
    direction = direction.normalize();
    float x = (atan2(direction.z, direction.x) / (2*M_PI))+0.5;
    float y = 1.0 - (acos(direction.y/1.0) / M_PI);
    return skybox->getColor(Vector3(-x,y,0));
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
        return getSkybox(ray.direction);
        //return BACKGROUND;
    }

    Vector3 scaled = ray.direction.normalize().scale(distance);
    Vector3 intersect = ray.origin + scaled;
    Vector3 normal = closest->getNormal(intersect).normalize();

    if (normal.dot(ray.direction) > 0) {
        normal = normal.scale(1);
    }

    normal = normal+closest->getTangentAxis(intersect).scale(((float)closest->normalMap->getColor(closest->getSurfCoords(intersect)).r/255.0)-0.5).scale(1.0)+closest->getBitangentAxis(intersect).scale(((float)closest->normalMap->getColor(closest->getSurfCoords(intersect)).g/255.0)-0.5).scale(1.0);

    float diffuse = 0;
    float specular = 0;
    for( unsigned int i = 0; i < sizeof(lights)/sizeof(lights[0]); i += 1 ) {
        Primitive* l = lights[i];
        for (unsigned int y = 0; y < LIGHTING_SAMPLE_GRID_SIZE; y += 1){
            for (unsigned int x = 0; x < LIGHTING_SAMPLE_GRID_SIZE; x += 1){
                float x2 = ((float)x/(float)LIGHTING_SAMPLE_GRID_SIZE);
                float y2 = ((float)y/(float)LIGHTING_SAMPLE_GRID_SIZE);
                if (LIGHTING_SAMPLE_GRID_SIZE > 1) {
                    // Maybe this should be done regularly not randomly
                    //x2 += ((float)rand()/(float)RAND_MAX)-0.5;
                    //y2 += ((float)rand()/(float)RAND_MAX)-0.5;
                }
                diffuse += (l->getWorldCoord(l->getCenter(), Vector3 (x2,y2,0)) - intersect).normalize().dot(normal);
                specular += (l->getWorldCoord(l->getCenter(), Vector3 (x2,y2,0)) - intersect).normalize().dot(reflect(ray.direction, intersect, normal).direction);
            }
        }
    }

    diffuse /= (float)(LIGHTING_SAMPLE_GRID_SIZE*LIGHTING_SAMPLE_GRID_SIZE);
    specular /= (float)(LIGHTING_SAMPLE_GRID_SIZE*LIGHTING_SAMPLE_GRID_SIZE);

    diffuse = clamp(diffuse, 1.0, 0.0);
    specular = clamp(specular, 1.0, 0.0);

    if (diffuse < AMBIENT) {diffuse = AMBIENT;}
    specular = pow(specular, 40);

    float shadowIntensity = 0;
    for( unsigned int i = 0; i < sizeof(lights)/sizeof(lights[0]); i += 1 ) {
        Primitive* l = lights[i];
        for (unsigned int y = 0; y < SHADOW_SAMPLE_GRID_SIZE; y += 1){
            for (unsigned int x = 0; x < SHADOW_SAMPLE_GRID_SIZE; x += 1){
                float x2 = ((float)x/(float)SHADOW_SAMPLE_GRID_SIZE);
                float y2 = ((float)y/(float)SHADOW_SAMPLE_GRID_SIZE);
                if (SHADOW_SAMPLE_GRID_SIZE > 1) {
                    x2 += ((float)rand()/(float)RAND_MAX)-0.5;
                    y2 += ((float)rand()/(float)RAND_MAX)-0.5;
                }
                Ray shadowRay (intersect+normal.scale(0.001), (l->getWorldCoord(l->getCenter(), Vector3 (x2,y2,0))-intersect).normalize());
                Intersect obstruction = getFirstIntersect(shadowRay);
                if (obstruction.distance < (l->getWorldCoord(l->getCenter(), Vector3 (x2,y2,0))-intersect).magnitude()) {
                    shadowIntensity += 1.0/(float)(SHADOW_SAMPLE_GRID_SIZE*SHADOW_SAMPLE_GRID_SIZE);
                }
            }
        }
    }
    shadowIntensity /= (float)(sizeof(lights)/sizeof(lights[0]));

    if (shadowIntensity > 1.0-AMBIENT) { shadowIntensity = 1.0-AMBIENT; }

    sf::Color out = closest->material->getColor(closest->getSurfCoords(intersect));
    out.r = ((out.r*diffuse*closest->kd)+(255*specular*closest->ks));
    out.g = ((out.g*diffuse*closest->kd)+(255*specular*closest->ks));
    out.b = ((out.b*diffuse*closest->kd)+(255*specular*closest->ks));
    clamp(out);

    if ((recursion_depth+1 < MAX_RECURSIONS) and (closest->kt < 1.0) and (closest->reflectivity > 0.0)) {
        Ray reflection = reflect(ray.direction, intersect, normal);
        sf::Color reflected_color;
        reflected_color = trace(reflection, recursion_depth+1);
        out.r = (out.r*(1.0-closest->reflectivity))+(reflected_color.r*closest->reflectivity);
        out.g = (out.g*(1.0-closest->reflectivity))+(reflected_color.g*closest->reflectivity);
        out.b = (out.b*(1.0-closest->reflectivity))+(reflected_color.b*closest->reflectivity);
    } else {
        out.r = (out.r*(1.0-closest->reflectivity));
        out.g = (out.g*(1.0-closest->reflectivity));
        out.b = (out.b*(1.0-closest->reflectivity));
    }

    if ((recursion_depth+1 < MAX_RECURSIONS) and (closest->kt > 0.0)) {
        Ray refracted = refract(ray.direction, intersect, normal, closest->kr);
        Vector3 exitPoint = intersect + refracted.direction.normalize().scale(getFirstIntersect(refracted).distance);
        refracted = refract(refracted.direction, exitPoint, closest->getNormal(exitPoint).normalize(), 1.0/closest->kr);
        sf::Color refracted_color;
        refracted_color = trace(refracted, recursion_depth+1);
        out.r = (out.r*(1.0-closest->kt))+(refracted_color.r*closest->kt);
        out.g = (out.g*(1.0-closest->kt))+(refracted_color.g*closest->kt);
        out.b = (out.b*(1.0-closest->kt))+(refracted_color.b*closest->kt);
    } else {
        out.r = (out.r*(1.0-closest->kt));
        out.g = (out.g*(1.0-closest->kt));
        out.b = (out.b*(1.0-closest->kt));
    }

    out.r = (out.r*(1.0-shadowIntensity));
    out.g = (out.g*(1.0-shadowIntensity));
    out.b = (out.b*(1.0-shadowIntensity));
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

    float aspectRatio = (float)WIDTH/(float)HEIGHT;

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
        sprite.setScale(window.getView().getSize().x / sprite.getLocalBounds().width, window.getView().getSize().y / sprite.getLocalBounds().height);

        float theta = frame*((2.0*M_PI)/400.0);
        Vector3 cameraRot (0,-theta,0);
        Vector3 cameraPos (150.0*sin(theta), 0.0, 150.0*-cos(theta));

        for(int y = 0; y < HEIGHT; y++) {
            for(int x = 0; x < WIDTH; x++) {
                    Vector3 direction = Vector3(aspectRatio*(((float)x/(float)WIDTH)-0.5), ((float)-y/(float)HEIGHT)+0.5, 1.0);
                    direction = direction.rotate(cameraRot);
                    Ray ray (cameraPos, direction);
                    int val = ((y*WIDTH)+x);
                    rays[val] = ray;
            }
        }

        #ifdef _OPENMP
        auto time = omp_get_wtime ();
        #endif

	    int y;
        #pragma omp parallel for private(y)
        for(y = 0; y < HEIGHT; y++) {
            int x;
            int colors_temp [WIDTH*3];
            for (x = 0; x < WIDTH; x++) {
                int val = ((y*WIDTH)+x);
                sf::Color c = trace(rays[val], 0);
                colors[val] = c;
            }
        }

        #ifdef _OPENMP
        printf("%f seconds\n", omp_get_wtime() - time);
        printf("%f FPS\n", 1.0/(omp_get_wtime() - time));
        #endif

        for(int y = 0; y < HEIGHT; y++) {
            for(int x = 0; x < WIDTH; x++) {
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
