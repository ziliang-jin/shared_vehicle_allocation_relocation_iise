#ifndef _PROBLEM_H_
#define _PROBLEM_H_
#include "basic_parameter.h"
#include "variables.h"

#include <stdlib.h>
#include <ilcplex/ilocplex.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

//typedef IloArray<IloNumArray>	NumMatrix;

//using namespace std;

using std::vector;

void create_primal_problem(initial_parameter initial, basic_parameter& basic, primal_variable primal);

void create_station_problem(initial_parameter initial, basic_parameter& basic, dual_variable dual);

void create_comp_problem(initial_parameter initial, basic_parameter& basic, primal_variable primal, dual_variable dual);

void create_obj(initial_parameter initial, basic_parameter& basic, primal_variable primal);

void resolve_problem(initial_parameter initial, basic_parameter& basic, primal_variable primal);

void create_no_competition(initial_parameter initial, basic_parameter& basic, primal_variable primal);

void create_firmA_problem(initial_parameter initial, basic_parameter& basic, primal_variable primal);

void create_firmB_problem(initial_parameter initial, basic_parameter& basic, primal_variable primal);

#endif;