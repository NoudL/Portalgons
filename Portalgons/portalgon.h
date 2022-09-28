#pragma once

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Vector_2.h>
#include <CGAL/Segment_2.h>
#include "ipe_parser.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel	K;
typedef CGAL::Polygon_2<K>                                  Polygon;
typedef CGAL::Point_2<K>                                    Point;
typedef CGAL::Vector_2<K>									Vector;
typedef CGAL::Segment_2<K>									Segment;

class Portalgon;
class Fragment;
class PortalSide;


struct DrawableEdge {
	Segment edge;
	uint32_t color;
};

class Fragment {
public:
	Polygon p;
	std::vector<boost::optional<PortalSide>> portals;

	Fragment() {
		p = Polygon();
	}

	std::vector < DrawableEdge > draw();
};


class Portalgon {
public:
	std::vector <Fragment *> fragments;
	
	Portalgon(std::vector<Fragment *> f) {
		fragments = f;
	}


	std::vector < DrawableEdge > draw();
	//void addPortal(std::vector<Point> line, color);
};

Portalgon createPortalgon();
Portalgon createPortalgonFromIpe(std::string file_name);


class PortalSide {
public:
	int id;
	bool flipped; //if true: arrow points anti clockwise (like the fragment is defined)
	Fragment * parent;
	PortalSide * exit;
	uint32_t color;

	PortalSide(int id, Fragment * parent, PortalSide * exit = NULL, bool flipped = false) {
		this->id = id;
		this->flipped = flipped;
		this->parent = parent;
		this->exit = exit;
		if (exit == NULL) {
			
			uint32_t red = (rand() % 255) << 16;
			uint16_t green = (rand() % 255) << 8;
			uint8_t  blue = (rand() % 255) << 0;
			
			color = red + green + blue;
		}
		else {
			color = exit->color;
		}

	}

	PortalSide() {

	};
	//TODO: write proper destructor because we are creating "new" PortalSides in createFragment

	Segment getSegment() {
		return parent->p.edge(id);
	}


	std::vector <DrawableEdge> draw();
};

Vector rotate(Vector v, double angle);

void makePortalSide(Segment seg, uint32_t color, std::vector<Fragment *> fragments, PortalSide * e);
