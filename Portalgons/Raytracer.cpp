#include "Raytracer.h"



Raytracer::Raytracer(float x, float y)
{
	source = Point(x, y);

}

Segment Raytracer::intersect(Ray ray, Segment seg) {

	const auto result = intersection(ray, seg);
	if (result) {
		if (const Segment* s = boost::get<Segment>(&*result)) {
			return Segment(ray.source(), s->source());
		}
		else {
			const Point* p = boost::get<Point >(&*result);
			return Segment(ray.source(), *p);
		}
	}
	return Segment();
}


Segment Raytracer::intersect(Ray ray, Portalgon& portalgon) {
	Segment result(Point(0,0), Point(100000,100000)); //TODO: more elegant
	for each (Fragment frag in portalgon.fragments)
	{
		for (Polygon::Edge_const_iterator sit = frag.p.edges_begin(); sit != frag.p.edges_end(); ++sit)
		{
			Segment s = intersect(ray, *sit);
			if (s.squared_length() != 0 && s.squared_length() < result.squared_length() ){
				result = s;
			}
		}
	}
	return result;
}


Segment Raytracer::castRay(Portalgon& p, Direction direction) {
	Ray ray = Ray(source, direction);
	return intersect(ray, p);
}

Raytracer::~Raytracer()
{
}
