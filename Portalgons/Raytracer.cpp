#include "Raytracer.h"
#include <iostream>
constexpr bool GLOBAL_DISTANCE = true;

Raytracer::Raytracer(double x, double y, double initialspeed, double stepsize)
{
	source = Point(x, y);
	this->initialspeed = initialspeed;
	this->stepsize = stepsize;
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
	for each (Fragment * frag in portalgon.fragments)
	{
		for (Polygon::Edge_const_iterator sit = frag->p.edges_begin(); sit != frag->p.edges_end(); ++sit)
		{
			Segment s = intersect(ray, *sit);
			if (s.squared_length() != 0 && s.squared_length() < result.squared_length() ){
				result = s;
			}
		}
	}
	return result;
}



Segment Raytracer::intersect(PathSegment ray_path_seg, Segment *seg) {
	if (ray_path_seg.intersected == *seg) {
		return ray_path_seg.segment;
	}
	const auto result = intersection(ray_path_seg.segment, *seg);
	if (result) {
		if (const Segment* s = boost::get<Segment>(&*result)) {
			return Segment(ray_path_seg.segment.source(), s->source());
		}
		else {
			const Point* p = boost::get<Point >(&*result);
			return Segment(ray_path_seg.segment.source(), *p);
		}
	}
	return ray_path_seg.segment;
}


std::vector<PathSegment> Raytracer::expandRay(PathSegment rayseg, Portalgon& portalgon) {
	if (rayseg.closed) {
		return { rayseg };
	}
	if (rayseg.segment.target().x() != rayseg.segment.target().x()) {
		int hetgaatmis = 1;
	}
	rayseg.age += 1;
	//First lengthen the rayseg with length of stepsize / speed:
	Segment newseg = Segment(rayseg.segment.source(), rayseg.segment.target() + (rayseg.segment.to_vector() / (sqrt(rayseg.segment.squared_length()))) * stepsize * rayseg.speed);
	bool closed = false;
	std::vector<PathSegment> result = { {newseg, rayseg.speed, closed, rayseg.intersected} };
	result[0].age = rayseg.age;
	for each (Fragment * frag in portalgon.fragments)
	{
		int index = 0;
		for each(Segment edge in frag->p.edges())
		{
			Segment s = intersect(result[0], &edge);
			if (s.squared_length() < result[0].segment.squared_length()) {
				result = { {s, rayseg.speed, true, rayseg.intersected} };
				result[0].age = rayseg.age;

				//check if it was a portalside, if so, create new segment as extension on other side of portal:
				if (frag->portals[index]) {
					PortalSide portalside = boost::get<PortalSide>(frag->portals[index]);
					result = { {s, rayseg.speed, true, rayseg.intersected}, extendThroughPortal({s, rayseg.speed, closed}, newseg, portalside) };
					result[0].age = rayseg.age;
					result[1].age = rayseg.age;
				}
			}
			
			index++;
		}
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

	double speed = intersected.speed;
	double old_speed = speed;
	RT portal_ratio = sqrt(exit.squared_length() / entry.squared_length());
	RT portal_angle = atan2(exit_vec.y(), exit_vec.x()) - atan2(entry_vec.y(), entry_vec.x());
	//RT portal_angle = acos((entry_vec * exit_vec) / sqrt(entry.squared_length() * exit.squared_length()));

	
	Segment just_the_tip = Segment();
	Transformation rotate(CGAL::ROTATION, sin(portal_angle), cos(portal_angle));

	Vector tippy = original.to_vector() - intersected.segment.to_vector();	

	tippy = rotate(tippy);

	if (!GLOBAL_DISTANCE) {
		speed = intersected.speed * portal_ratio;
	}
	else {
		Vector normal = Vector(entry_vec.y(), -entry_vec.x()); //normal vector pointing towards the portal
		if (original.to_vector() * normal < 0) {
			normal *= -1;
		}
		/*if (portal.flipped == portal.exit->flipped) {
			normal *= -1;
		}*/
		RT normal_angle = atan2(normal.y(), normal.x());
		
		RT ray_angle = atan2(original.to_vector().y(), original.to_vector().x());
		
		RT incoming_angle = normal_angle - ray_angle; //angle with entry portal normal
		
		//std::cout << "Incoming angle before: " << std::endl;
		//std::cout << incoming_angle << std::endl;
		
		/*while (incoming_angle <= -PI) {
			incoming_angle += PI;
		}
		while (incoming_angle >= PI) {
			incoming_angle -= PI;
		}*/
		
		RT sin_outgoing = sin(incoming_angle) / portal_ratio;
		if (sin_outgoing >= 1 || sin_outgoing <= -1) {
			return { Segment(), speed, true};
		}
		RT outgoing_angle = asin(sin_outgoing);
		RT rotate_angle = -(outgoing_angle - incoming_angle);

		Transformation rotate_global(CGAL::ROTATION, sin(rotate_angle), cos(rotate_angle));
		tippy = rotate_global(tippy);	
		if (tippy.x() != tippy.x()) {
			int hetgaatmis = 1;
		}
	}

	if (!(portal.flipped ^ portal.exit->flipped)) {
		Transformation reflect(CGAL::REFLECTION, Line(Point(0, 0), exit_vec));
		tippy = reflect(tippy);
	}
	just_the_tip = Segment(exit_point, exit_point + tippy * (speed / old_speed) * stepsize);
	if (just_the_tip.target().x() != just_the_tip.target().x()) {
		int hetgaatmis = 1;
	}


	return { just_the_tip, speed, false, exit };
}

// Very rough way to visualize wavefront collision?
void Raytracer::collideRaySegs(PathSegment* a, PathSegment* b) {
	if (a->closed) {
		return;
	}
	const auto result = intersection(a->segment, b->segment);
	if (result) {
		if (const Segment* s = boost::get<Segment>(&*result)) {
			return;
		}
		else {
			const Point* p = boost::get<Point >(&*result);
			if (*p == a->segment.source()) {
				return;
			}
		}
		a->closed = true;
		b->closed = true;
	}
	return;
}


Segment Raytracer::castRay(Portalgon& p, Direction direction) {
	Ray ray = Ray(source, direction);
	return intersect(ray, p);
}

/**
	Begin expanding a ray segment in the portalgon with stepsize

	@param direction Assume normalized direction 
*/
PathSegment Raytracer::castRaySegment(Portalgon& p, Direction direction) {
	Segment rayseg = Segment(source, source + (direction.to_vector() * stepsize * initialspeed));
	return { rayseg, initialspeed, false };
}


Raytracer::~Raytracer()
{
}
