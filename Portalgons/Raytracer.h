#pragma once
#include "portalgon.h"
#include <CGAL/Ray_2.h>
#include <CGAL/Line_2.h>

typedef K::Ray_2			Ray;
typedef K::Intersect_2		Intersect;
typedef K::Direction_2		Direction;
class Raytracer
{
public:
	Point source;
	Raytracer(float x, float y);
	~Raytracer();


	Segment intersect(Ray ray, Segment segment);
	Segment intersect(Ray ray, Portalgon& p);

	Segment castRay(Portalgon& p, Direction direction);
};

