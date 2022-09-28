#pragma once
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include <map>

void parseIpeFile(std::string file_name,
	std::vector<std::vector<std::vector<double>>> * paths,
	std::vector < std::vector<double>> * transformations,
	std::vector< std::pair< std::vector<std::vector<double>>, std::string>  > * portals,
	std::vector<std::vector<double>> * portal_transformations);


std::map<std::string, uint32_t> getColorsFromFile(std::string file_name);
uint32_t RGB_to_BGR_hex(double _R, double _G, double _B);