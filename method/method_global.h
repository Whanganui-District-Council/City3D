#ifndef _METHOD_GLOBAL_H_
#define _METHOD_GLOBAL_H_


#include "method_common.h"
#include "../math/linear_program_solver.h"

#include <string>


namespace Method {

	extern METHOD_API double lambda_data_fitting;
	extern METHOD_API double lambda_model_height;
	extern METHOD_API double lambda_model_complexity;

    extern METHOD_API double number_region_growing;
    extern METHOD_API double point_density;

	// - two points considered as coincident; 
	// - a point considered to be on a plane, etc.
	extern METHOD_API double coincident_threshold;


	extern METHOD_API LinearProgramSolver::SolverName  solver_name;

	//________________ names for various quality measures ____________________

	extern METHOD_API std::string facet_attrib_supporting_vertex_group;
	extern METHOD_API std::string facet_attrib_supporting_point_num;
	extern METHOD_API std::string facet_attrib_facet_area;
	extern METHOD_API std::string facet_attrib_covered_area;

}


#endif