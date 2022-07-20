#include "portalgon.h"


void Fragment::createFragment(){
	//Put some points in the polygon
	p.push_back(Point(0, 0));
	p.push_back(Point(4, 0));
	p.push_back(Point(4, 4));
	p.push_back(Point(2, 2));
	p.push_back(Point(0, 4));
	
	// create 2 portal edges connected to eachother:
	PortalSide * e1 = new PortalSide(0, this);
	PortalSide * e2 = new PortalSide(3, this, e1);
	e1->exit = e2;

	portals.push_back(*e1);
	portals.push_back(*e2);
}


std::list < DrawableEdge> PortalSide::draw() {
	std::list < DrawableEdge> toDraw;
	
	Fragment temp1 = * parent;
	Polygon temp2 = parent->p;
	Segment edge = parent->p.edge(id);
	//draw arrow
	toDraw.push_back({ edge, color });
	toDraw.push_back({ Segment(edge.target(), edge.target() + edge.to_vector()), color });
	return toDraw;
	
}

std::list <DrawableEdge> Fragment::draw() {
	std::list < DrawableEdge> toDraw;

	for (Polygon::Edge_const_iterator sit = p.edges_begin(); sit != p.edges_end(); ++sit)
	{
		toDraw.push_back({ *sit, 0x0 });
	}
	
	for each (PortalSide portalside in portals)
	{
		toDraw.splice(toDraw.end(), portalside.draw());
	}
	return toDraw;
}