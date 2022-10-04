#include "portalgon.h"
#include <sstream>
using namespace std;

Portalgon createPortalgonFromGraphml(string file_name) {
	vector<vector<double>> path = parseGraphmlFile(file_name);
	Fragment *f = new Fragment();
	for (vector<double> point : path) {
		f->p.push_back(Point(point[0],point[1]));
	}
	f->portals.resize(f->p.edges().size());
	return Portalgon({ f });
}

Portalgon createPortalgonFromIpe(string file_name) {
	vector<vector<vector<double>>> paths;
	vector< pair < vector<vector<double>>, string> > portals;
	vector<vector<double>> path_transformations;
	vector<vector<double>> portal_transformations;
	parseIpeFile(file_name, &paths, &path_transformations, &portals, &portal_transformations);

	std::vector<Fragment *> fragments;
	int index = 0;
	for each (vector<vector<double>> path in paths)
	{
		Fragment * f = new Fragment();
		vector<double> transformation_vector = path_transformations[index];
		K::Aff_transformation_2 transformation_matrix = K::Aff_transformation_2(	K::RT(transformation_vector[0]),  //m00	scale/rotation
																					K::RT(transformation_vector[2]),  //m01
																					K::RT(transformation_vector[4]),  //m02
																					K::RT(transformation_vector[1]),  //m10
																					K::RT(transformation_vector[3]),  //m11	translation
																					K::RT(transformation_vector[5])); //m12
		for each (vector<double> point in path) {
			Point pnt = Point(point[0], point[1]);
			pnt = transformation_matrix(pnt);
			f->p.push_back(Point(pnt));
		}
		
		f->portals.resize(f->p.edges().size());
		
		if (f->p.is_simple()) {
			if (f->p.is_counterclockwise_oriented()) {
				f->p.reverse_orientation();
			}

			fragments.push_back(f);
		}
		index++;
	}
	map<string, uint32_t> colors = getColorsFromFile(file_name);
	map<uint32_t, vector<Segment>> portal_pairs;
	index = 0;
	for each (pair<vector<vector<double>>, string> portal in portals) {
		vector<double> transformation_vector = portal_transformations[index];
		K::Aff_transformation_2 transformation_matrix = K::Aff_transformation_2(K::RT(transformation_vector[0]),  //m00	scale/rotation
																				K::RT(transformation_vector[2]),  //m01
																				K::RT(transformation_vector[4]),  //m02
																				K::RT(transformation_vector[1]),  //m10
																				K::RT(transformation_vector[3]),  //m11	translation
																				K::RT(transformation_vector[5])); //m12

		Point source = Point(portal.first[0][0], portal.first[0][1]);
		Point target = Point(portal.first[1][0], portal.first[1][1]);

		source = transformation_matrix(source);
		target = transformation_matrix(target);

		Segment segment(source, target);
		portal_pairs[colors[portal.second]].push_back(segment);
		index++;
	}

	for (const auto&[key, value] : portal_pairs) {
		std::vector<PortalSide *> pair;
		if (value.size() != 2) {
			continue;
		}
		PortalSide * e1 = new PortalSide();
		PortalSide * e2 = new PortalSide();
		e1->exit = e2;
		e2->exit = e1;
		makePortalSide(value[0], key, fragments, e1);
		makePortalSide(value[1], key, fragments, e2);

	}

	return Portalgon(fragments);
}

void makePortalSide(Segment seg, uint32_t color, std::vector<Fragment *> fragments, PortalSide * e) {
	bool found = false;
	for (Fragment * f : fragments)
	{
		int id = 0;
		for (Segment edge : f->p.edges())
		{
			if ((edge.source() == seg.source() && edge.target() == seg.target()) ||
				(edge.source() == seg.target() && edge.target() == seg.source())) {
				bool direction_of_edge = true;
				if (edge.source() == seg.target() && edge.target() == seg.source()) {
					direction_of_edge = false;
				}
				e->id = id;
				e->parent = f;
				e->flipped = !direction_of_edge;
				e->color = color;
				f->portals[e->id] = *e;
				found = true;
			}
			id++;
			if (found) {
				break;
			}
		}
		if (found) {
			break;
		}
	}
	return;
}


#if 0
Portalgon createPortalgon(){
	Fragment * f =  new Fragment();
	//Put some points in the polygon
	//TODO: Add functions to Fragment to add polygon and automatically resize portals vector accordingly, set portalsides etc.
	f->p.push_back(Point(2, 2));
	f->p.push_back(Point(3, 1));
	f->p.push_back(Point(3, 10));
	f->p.push_back(Point(2, 8));

	f->portals.resize(f->p.edges().size());
	// create 2 portal edges connected to eachother:
	PortalSide * e1 = new PortalSide(1, f);
	PortalSide * e2 = new PortalSide(3, f, e1, true);
	e1->exit = e2;
	f->portals[e1->id] = *e1;
	f->portals[e2->id] = *e2;
	/*
	Fragment * f2 = new Fragment();
	//Put some points in the polygon
	f2->p.push_back(Point(9, 0));
	f2->p.push_back(Point(20, -5));
	f2->p.push_back(Point(20, 15));
	f2->p.push_back(Point(9, 6));
	f2->portals.resize(f2->p.edges().size());

	PortalSide * e3 = new PortalSide(3, f2, e1, true);
	PortalSide * e4 = new PortalSide(0, f2, e2, true);
	e1->exit = e3;
	e2->exit = e4;
	f->portals[e1->id] = *e1;
	f->portals[e2->id] = *e2;
	f2->portals[e3->id] = *e3;
	f2->portals[e4->id] = *e4;
	*/
	std::vector<Fragment> fragments;
	fragments.push_back(*f);
	//fragments.push_back(*f2);
	return Portalgon(fragments);
}
#endif
//void Portalgon::addPortal(std::vector<Point>) {

//}

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


std::vector < DrawableEdge > Fragment::drawCDT() {
	std::vector < DrawableEdge> toDraw;

	for (CDT::Finite_edges_iterator sit = cdt.finite_edges_begin(); sit != cdt.finite_edges_end(); ++sit)
	{
		Segment edge = cdt.segment(sit);
		toDraw.push_back({ edge, 0x0 });
	}
	return toDraw;
}



Vector rotate(Vector v, double angle) {
	Vector v2 = Vector(v.x() * cos(angle) - v.y() * sin(angle), v.x() * sin(angle) + v.y() * cos(angle));
	return v2;
}


std::vector <DrawableEdge> Portalgon::draw() {

	std::vector < DrawableEdge> toDraw;
	for each (Fragment * f in fragments)
	{
		std::vector < DrawableEdge> temp = f->draw();
		toDraw.insert(toDraw.end(), temp.begin(), temp.end());
	}
	return toDraw;
}

std::vector < DrawableEdge > Portalgon::drawCDTs() {

	std::vector < DrawableEdge> toDraw;
	for each (Fragment * f in fragments)
	{
		std::vector < DrawableEdge> temp = f->drawCDT();
		toDraw.insert(toDraw.end(), temp.begin(), temp.end());
	}
	return toDraw;
}
 