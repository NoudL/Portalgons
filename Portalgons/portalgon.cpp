#include "portalgon.h"


Portalgon createPortalgon(){
	Fragment * f =  new Fragment();
	//Put some points in the polygon
	//TODO: Add functions to Fragment to add polygon and automatically resize portals vector accordingly, set portalsides etc.
	f->p.push_back(Point(-1, 0));
	f->p.push_back(Point(4, 0));
	f->p.push_back(Point(4, 4));
	f->p.push_back(Point(0, 4));

	f->portals.resize(f->p.edges().size());
	// create 2 portal edges connected to eachother:
	PortalSide * e1 = new PortalSide(1, f);
	PortalSide * e2 = new PortalSide(3, f);
	 

	Fragment * f2 = new Fragment();
	//Put some points in the polygon
	f2->p.push_back(Point(7, 0));
	f2->p.push_back(Point(10, 0));
	f2->p.push_back(Point(10, 5));
	f2->p.push_back(Point(9, 5));
	f2->portals.resize(f2->p.edges().size());

	PortalSide * e3 = new PortalSide(3, f2, e1, false, true);
	PortalSide * e4 = new PortalSide(0, f2, e2, false, true);
	e1->exit = e3;
	e2->exit = e4;
	f->portals[e1->id] = *e1;
	f->portals[e2->id] = *e2;
	f2->portals[e3->id] = *e3;
	f2->portals[e4->id] = *e4;
	std::vector<Fragment> fragments;
	fragments.push_back(*f);
	fragments.push_back(*f2);
	return Portalgon(fragments);
}


std::vector < DrawableEdge> PortalSide::draw() {
	std::vector < DrawableEdge> toDraw;
	
	Fragment temp1 = * parent;
	Polygon temp2 = parent->p;
	Segment edge = parent->p.edge(id);
	//draw arrow
	toDraw.push_back({ edge, color });
	if (!flipped) {
		toDraw.push_back({ Segment(edge.target(), edge.target() - (0.05 * rotate(edge.to_vector(), 0.3))), color });
		toDraw.push_back({ Segment(edge.target(), edge.target() - (0.05 * rotate(edge.to_vector(), -0.3))), color });
	}
	else {
		toDraw.push_back({ Segment(edge.source(), edge.source() + (0.05 * rotate(edge.to_vector(), 0.3))), color });
		toDraw.push_back({ Segment(edge.source(), edge.source() + (0.05 * rotate(edge.to_vector(), -0.3))), color });
	}
	return toDraw;
	
}

std::vector <DrawableEdge> Fragment::draw() {
	std::vector < DrawableEdge> toDraw;

	for (Polygon::Edge_const_iterator sit = p.edges_begin(); sit != p.edges_end(); ++sit)
	{
		toDraw.push_back({ *sit, 0x0 });
	}
	for each (boost::optional<PortalSide> ps in portals) {
		if (ps){
			PortalSide portalside = boost::get<PortalSide>(ps);
			std::vector < DrawableEdge> temp = portalside.draw();
			toDraw.insert(toDraw.end(), temp.begin(), temp.end());
		}
	}
	return toDraw;
}

Vector rotate(Vector v, double angle) {
	Vector v2 = Vector(v.x() * cos(angle) - v.y() * sin(angle), v.x() * sin(angle) + v.y() * cos(angle));
	return v2;
}


std::vector <DrawableEdge> Portalgon::draw() {

	std::vector < DrawableEdge> toDraw;
	for each (Fragment f in fragments)
	{
		std::vector < DrawableEdge> temp = f.draw();
		toDraw.insert(toDraw.end(), temp.begin(), temp.end());
	}
	return toDraw;
}
 