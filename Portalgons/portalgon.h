#pragma once

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Vector_2.h>
#include <CGAL/Segment_2.h>

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
	std::list<PortalSide> portals;

	Fragment() {
		p = Polygon();
	}

	void Fragment::createFragment();
	std::list < DrawableEdge > draw();
};



class Portalgon {
public:
	std::list <Fragment> fragments;

	Portalgon(std::list<Fragment> f) {
		fragments = f;
	}

	std::list < DrawableEdge > draw();
};


class PortalSide {
public:
	int id;
	bool orientation;
	bool flipped;
	Fragment * parent;
	PortalSide * exit;
	uint32_t color;

	PortalSide(int id, Fragment * parent, PortalSide * exit = NULL, bool orientation = false, bool flipped = false) {
		this->id = id;
		this->orientation = orientation;
		this->flipped = flipped;
		this->parent = parent;
		this->exit = exit;
		if (exit == NULL) {
			/*
			uint32_t red = (rand() % 255) << 16;
			uint16_t green = (rand() % 255) << 8;
			uint8_t  blue = (rand() % 255) << 0;
			*/
			color = 0x00ff00;
		}
		else {
			color = exit->color;
		}

	}
	//TODO: write proper destructor because we are creating "new" PortalSides in createFragment

	std::list <DrawableEdge> draw();
};

