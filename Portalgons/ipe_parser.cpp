#include "ipe_parser.h"
#include <sstream>
#include <iostream>

void parseIpeFile(std::string file_name, 
				std::vector<std::vector<std::vector<double>>> * paths,
				std::vector<std::vector<double>> * path_transformations,
				std::vector< std::pair< std::vector<std::vector<double>>, std::string>  > * portals,
				std::vector<std::vector<double>> * portal_transformations ){

	rapidxml::file<> f_doc(file_name.c_str());
	rapidxml::file<> xmlFile(f_doc);
	rapidxml::xml_document<> doc;
	doc.parse<0>(xmlFile.data());
	

	rapidxml::xml_node<>* node = doc.first_node("ipe")->first_node("page");

	node = node->first_node("path");
	while (node != NULL)
	{
		std::stringstream ss(node->value());
		std::string to;
		
		if (node->value() != NULL)
		{
			std::vector<double> transformation;
			if (node->first_attribute("matrix") != nullptr) {
				std::stringstream matrix(node->first_attribute("matrix")->value());
				std::string m;
				while (std::getline(matrix, m, ' ')) {
					if (m != "") {
						transformation.push_back(std::stod(m));
					}
				}
			}
			else {
				transformation = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
			}

			std::vector<std::vector<double>> path;
			while (std::getline(ss, to, '\n')) {
				if (to != "" && to != "h") {
					std::stringstream line(to);
					std::string coordinate;
					std::vector<double> point;
					while (std::getline(line, coordinate, ' ')) {
						if (coordinate != "m" && coordinate != "l"){
							point.push_back(std::stod(coordinate));
						}
					}
					path.push_back(point);
				}
			}
			if (path.size() == 2 && node->first_attribute("stroke") != nullptr) {
				std::string portalcolor = node->first_attribute("stroke")->value();
				std::pair<std::vector<std::vector<double>>, std::string> portal(path, portalcolor);
				portals->push_back(portal);
				portal_transformations->push_back(transformation);
			}
			else {
				paths->push_back(path);
				path_transformations->push_back(transformation);
			}
		}
		node = node->next_sibling("path");
	}
	return;
}


std::map<std::string, uint32_t> getColorsFromFile(std::string file_name) {
	rapidxml::file<> f_doc(file_name.c_str());
	rapidxml::file<> xmlFile(f_doc);
	rapidxml::xml_document<> doc;
	doc.parse<0>(xmlFile.data());


	rapidxml::xml_node<>* node = doc.first_node("ipe")->first_node("ipestyle");

	node = node->first_node("color");
	std::map<std::string, uint32_t> colors;
	while (node != NULL)
	{
		std::string name = node->first_attribute("name")->value();

		std::stringstream colortext(node->first_attribute("value")->value());
		std::string component;

		if (node->value() != NULL)
		{
			std::vector<double> color;
			while (std::getline(colortext, component, ' ')) {
				color.push_back(std::stod(component));
			}
			if (color.size() == 1) {
				colors.insert({ name, RGB_to_BGR_hex(color[0],color[0],color[0]) });
			}
			else {
				colors.insert({ name, RGB_to_BGR_hex(color[0],color[1],color[2]) });
			}
		}
		node = node->next_sibling("color");
	}
	return colors;
}

uint32_t RGB_to_BGR_hex(double _R, double _G, double _B)
{
	uint32_t R = uint32_t(_R * 255);
	uint32_t G = uint32_t(_G * 255);
	uint32_t B = uint32_t(_B * 255);
	uint32_t result = (B << 16) | (G << 8) | R;
	return result;
}
