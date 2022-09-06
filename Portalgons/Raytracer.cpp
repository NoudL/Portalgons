#include "Raytracer.h"



Raytracer::Raytracer(double x, double y, double initialspeed)
{
	source = Point(x, y);
	this->initialspeed = initialspeed;
}

Segment Raytracer::intersect(Ray ray, Segment seg) {

	const auto result = intersection(ray, seg);
	if (result) {
		if (const Segment* s = boost::get<Segment>(&*result)) {
			return Segment(ray.source(), s->source());
		}
		else {
			const Point* p = boost::get<Point >(&*result);
			if (*p == ray.source()) {
				return Segment();
			}
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



Segment Raytracer::intersect(Segment rayseg, Segment seg) {

	const auto result = intersection(rayseg, seg);
	if (result) {
		if (const Segment* s = boost::get<Segment>(&*result)) {
			return Segment(rayseg.source(), s->source());
		}
		else {
			const Point* p = boost::get<Point >(&*result);
			if (*p == rayseg.source()) {
				return Segment();
			}
			return Segment(rayseg.source(), *p);
		}
	}
	return rayseg;
}


std::vector<PathSegment> Raytracer::expandRay(PathSegment rayseg, Portalgon& portalgon, double stepsize) {
	if (rayseg.closed) {
		return { rayseg };
	}
	//First lengthen the rayseg with length of stepsize / speed:
	Segment newseg = Segment(rayseg.segment.source(), rayseg.segment.target() + (rayseg.segment.to_vector() / (sqrt(rayseg.segment.squared_length()))) * stepsize); //TODO: speed
	bool closed = false;
	std::vector<PathSegment> result = { {newseg, rayseg.speed, closed} };
	for each (Fragment frag in portalgon.fragments)
	{
		int index = 0;
		for each(Segment edge in frag.p.edges())
		{
			Segment s = intersect(newseg, edge);
			if (s.squared_length() < result[0].segment.squared_length()) {
				result = { {s, rayseg.speed, true} };

				//check if it was a portalside, if so, create new segment as extension on other side of portal:
				if (frag.portals[index]) {
					PortalSide portalside = boost::get<PortalSide>(frag.portals[index]);
					result = { {s, rayseg.speed, true}, extendThroughPortal({s, rayseg.speed, closed}, newseg, portalside) };
				}
			}
			
			index++;
		}
	}

	if (closed) {
		int test = 1;
	}
	return result;
}

PathSegment Raytracer::extendThroughPortal(PathSegment intersected, Segment original, PortalSide portal) {
	// bereken uitkomstpunt portal op basis van target() van intersected
	// maak nieuw segment met correcte hoek en speed op basis van verschil in lengte tussen original en intersected, en portals
	// return nieuw pathsegment met evt nieuwe speed
	
	//vector from source on entry portal
	Segment entry = portal.getSegment();
	Vector entry_vec = entry.to_vector();
	Vector interval;
	if (!portal.flipped) {
		interval = Segment(entry.source(), intersected.segment.target()).to_vector(); //TODO: could be a projected point on the portal instead of using the intersection point?
	}
	else {
		interval = Segment(entry.target(), intersected.segment.target()).to_vector();
		entry_vec *= -1;
	}
	RT entry_fraction = sqrt(interval.squared_length() / entry.squared_length()); //TODO: see if u can get this to be exact instead of double? (also can sqrt be a cgal sqrt?)
	
	Segment exit = portal.exit->getSegment();
	Vector exit_vec = exit.to_vector();
	RT exit_fraction = (sqrt(exit.squared_length()) * entry_fraction) / sqrt(exit.squared_length());

	Point exit_point;
	if (!portal.exit->flipped){
		exit_point = exit.source() + (exit_vec * exit_fraction);
	}
	else {
		exit_vec *= -1;
		exit_point = exit.target() + (exit_vec * exit_fraction);
	}

	RT portal_ratio = sqrt(entry.squared_length() / exit.squared_length());

	//RT portal_angle = acos((entry_vec * exit_vec) / sqrt(entry.squared_length() * exit.squared_length()));

	RT portal_angle = atan2(exit_vec.y(), exit_vec.x()) - atan2(entry_vec.y(), entry_vec.x());
	Segment just_the_tip = Segment();
	
	Transformation rotate(CGAL::ROTATION, sin(portal_angle), cos(portal_angle));
	Transformation rotate_pi(CGAL::ROTATION, sin(PI), sin(PI));

	Vector tippy = original.to_vector() - intersected.segment.to_vector();
	tippy = rotate(tippy);
	just_the_tip = Segment(exit_point, exit_point + tippy);



	return { just_the_tip, intersected.speed, false };
}


Segment Raytracer::castRay(Portalgon& p, Direction direction) {
	Ray ray = Ray(source, direction);
	return intersect(ray, p);
}

/**
	Begin expanding a ray segment in the portalgon with stepsize

	@param direction Assume normalized direction 
*/
PathSegment Raytracer::castRaySegment(Portalgon& p, Direction direction, double stepsize) {
	Segment rayseg = Segment(source, source + (direction.to_vector() * stepsize));
	return { rayseg, initialspeed, false };
}


Raytracer::~Raytracer()
{
}
