#pragma once

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Vector_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Constrained_triangulation_plus_2.h>
#include <CGAL/draw_triangulation_2.h>
#include "parser.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel	K;
typedef CGAL::Polygon_2<K>                                  Polygon;
typedef CGAL::Point_2<K>                                    Point;
typedef CGAL::Vector_2<K>									Vector;
typedef CGAL::Segment_2<K>									Segment;

typedef CGAL::Exact_intersections_tag                                     Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, CGAL::Default, Itag> CDT;
typedef CGAL::Constrained_triangulation_plus_2<CDT>                       CDTP;

typedef CDTP::Point                                                       CDTPPoint;
typedef CDTP::Constraint_id                                               Cid;
typedef CDTP::Vertex_handle                                               Vertex_handle;

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

	//Insert the polygons into a constrained triangulation
	CDT cdt;

	Fragment() {
		p = Polygon();
	}

	std::vector < DrawableEdge > draw();
	void generate_CDT() {
		cdt.insert_constraint(p.vertices_begin(), p.vertices_end(), true);
	}
	std::vector < DrawableEdge > drawCDT();
};


class Portalgon {
public:
	std::vector <Fragment *> fragments;
	
	Portalgon(std::vector<Fragment *> f) {
		fragments = f;
		for (Fragment * f : fragments) {
			f->generate_CDT();
		}
	}


	std::vector < DrawableEdge > draw();
	std::vector < DrawableEdge > drawCDTs();
	//void addPortal(std::vector<Point> line, color);
};

Portalgon createPortalgon();
Portalgon createPortalgonFromIpe(std::string file_name);
Portalgon createPortalgonFromGraphml(std::string file_name);


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
