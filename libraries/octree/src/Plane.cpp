//
//  Plane.h
//  hifi
//
//  Created by Brad Hefta-Gaub on 04/11/13.
//  Originally from lighthouse3d. Modified to utilize glm::vec3 and clean up to our coding standards
//
//  Simple plane class.
//

#include "Plane.h"

#include <stdio.h>

void Plane::set3Points(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3) {
    glm::vec3 linev1v2, linev1v3;

    linev1v2 = v2 - v1;
    linev1v3 = v3 - v1;

    // this will be perpendicular to both lines
    _normal = glm::cross(linev1v2,linev1v3);
    _normal = glm::normalize(_normal);

    // this is a point on the plane
    _point = v2;

    // the D coefficient from the form Ax+By+Cz=D
    _dCoefficient = -(glm::dot(_normal,_point));
}

void Plane::setNormalAndPoint(const glm::vec3 &normal, const glm::vec3 &point) {
    _point = point;
    _normal = normal;
    glm::normalize(_normal);

    // the D coefficient from the form Ax+By+Cz=D
    _dCoefficient = -(glm::dot(_normal,_point));
}

void Plane::setCoefficients(float a, float b, float c, float d) {
    // set the normal vector
    _normal = glm::vec3(a,b,c);

    //compute the lenght of the vector
    float l = glm::length(_normal);

    // normalize the vector
    _normal = glm::vec3(a/l,b/l,c/l);

    // and divide d by th length as well
    _dCoefficient = d/l;
}

float Plane::distance(const glm::vec3 &point) const {
    return (_dCoefficient + glm::dot(_normal,point));
}

void Plane::print() const {
    printf("Plane - point (x=%f y=%f z=%f) normal (x=%f y=%f z=%f) d=%f\n",
        _point.x, _point.y, _point.z, _normal.x, _normal.y, _normal.z, _dCoefficient);
}
