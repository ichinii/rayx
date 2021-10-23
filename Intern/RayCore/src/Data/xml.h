#pragma once

#include "rapidxml.hpp"
#include <glm.hpp>
#include <vector>

namespace RAYX {
    namespace xml {
        // these functions return whether they were successfull, and return their output in the argument `out`
        bool param(rapidxml::xml_node<>* node, const char* paramname, rapidxml::xml_node<>** out);
        bool paramDouble(rapidxml::xml_node<>* node, const char* paramname, double* out);
        bool paramInt(rapidxml::xml_node<>* node, const char* paramname, int* out);
        bool paramStr(rapidxml::xml_node<>* node, const char* paramname, const char** out);
        bool paramDvec3(rapidxml::xml_node<>* node, const char* paramname, glm::dvec3* out);
        bool paramMisalignment(rapidxml::xml_node<>* node, std::vector<double>* out);
    }
}