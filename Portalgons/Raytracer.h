#pragma once
#include "portalgon.h"
#include <CGAL/Ray_2.h>
#include <CGAL/Line_2.h>
#include <CGAL/Cartesian.h>


typedef K::Ray_2			Ray;
typedef K::Line_2			Line;
typedef K::Intersect_2		Intersect;
typedef K::Direction_2		Direction;
typedef K::RT				RT;
typedef CGAL::Quotient<RT>	Quotient;
typedef K::Aff_transformation_2 Transformation;

const double PI = 3.1415926535897931;

struct PathSegment {
	Segment segment;
	double speed;
	bool closed;
	Segment intersected = Segment();
};

class Raytracer
{
public:
	Point source;
	double initialspeed;
	Raytracer(double x, double y, double initialspeed);
	~Raytracer();


	Segment intersect(Ray ray, Segment segment);
	Segment intersect(Ray ray, Portalgon& p);
	std::vector<PathSegment> expandRay(PathSegment rayseg, Portalgon& portalgon, double stepsize);
	Segment intersect(PathSegment rayseg, Segment *seg);

	Segment castRay(Portalgon& p, Direction direction);
	PathSegment castRaySegment(Portalgon& p, Direction direction, double stepsize);

	PathSegment extendThroughPortal(PathSegment ps, Segment s, PortalSide portal);
};

