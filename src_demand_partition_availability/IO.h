#ifndef _IO_H_
#define _IO_H_
#include "basic_parameter.h"
#include "variables.h"

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <time.h>

using namespace std;

// function of inputting data
void input(initial_parameter initial, basic_parameter& basic, string root);
// function of generating demands
void demand_gen(initial_parameter initial, basic_parameter& basic);
// function of demand proportion
void demand_prop(initial_parameter initial, basic_parameter& basic, string root);
// function of demand
void demand_assign(initial_parameter initial, basic_parameter& basic, string root);
// function of output results
void output(initial_parameter initial, basic_parameter basic, primal_variable primal, string root);
// function of output service level
void output_service_nosharing(initial_parameter initial, basic_parameter basic, primal_variable primal, string root);
// function of output demand loss
void output_loss_nosharing(initial_parameter initial, basic_parameter basic, primal_variable primal, string root);
// function of output idle
void output_utility_nosharing(initial_parameter initial, basic_parameter basic, primal_variable primal, string root);
// function of output transfer
void output_transfer(initial_parameter initial, basic_parameter basic, primal_variable primal, string root);
// function of output summary for sharing problem
void output_summary(initial_parameter initial, basic_parameter basic, primal_variable primal, string root);
// function of obtian value
void get_val(initial_parameter initial, basic_parameter& basic, primal_variable primal, int firm_index);
// function of obtian value
void assign_historical_val(initial_parameter initial, basic_parameter& basic, primal_variable primal);
// function of checking converge
bool converge_check(initial_parameter initial, basic_parameter basic, int iter);
// start point
void start_point(initial_parameter initial, basic_parameter& basic, primal_variable primal);
// function of output results
void output_nonlinear(initial_parameter initial, basic_parameter basic, primal_variable primal, string root);
#endif;