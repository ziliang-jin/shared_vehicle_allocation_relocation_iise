//STANDARD C++ LIBRARIES
#include<stdio.h>
#include<iostream>
#include<fstream>
#include<iosfwd>
#include<string>
#include <queue>		//For implementing branching
#include <deque>
#include <sstream>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include<algorithm>
#include <set>
#include <chrono>
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#include <typeinfo>
#include <limits.h>		//For using INT_MAX

//CONCERT TECHNOLOGY LIBRARIES
#include <ilcplex/ilocplex.h>
#include <ilconcert/ilosys.h>
#include <ilconcert/ilocsvreader.h>

//User's Head Files
#include "basic_parameter.h"
#include "IO.h"
#include "problem.h"
#include "variables.h"

ILOSTLBEGIN


using namespace std;


int main(int argc, char **argv)
{
	
	try{

		//****************************** Parameters Work ******************************

		initial_parameter initial;
		cout << "--initial parameters are defined--" << endl;

		initial.T = 48; // time periods
		initial.nb_points = 6; // number of regions
		initial.nb_nodes = 3; // number of scenarios
		initial.nb_plates = 60; // used for calculating the total allocation capacity in all regions = initial.nb_plates * initial.nb_points
		initial.budget_A = 1500; // \hat{x}^A
		initial.budget_B = 1500; // \hat{x}^A
		initial.demand_level = 2; // demand level


		basic_parameter basic(initial.T, initial.nb_points, initial.nb_nodes, initial.nb_plates, initial.budget_A, initial.budget_B, initial.demand_level);

		basic.cost_purchase[0] = 0.16; // vehicle allocation cost c^A and c^b
		basic.penalty[0] = 0.1; // penalty cost h_P^A ad h_P^B
		basic.rental_price[0] = 0.4; // revenue per trip of firm A h_R^A
		basic.rental_price[1] = 0.4; // revenue per trip of firm B h_R^B
		basic.h[0][1] = 0.12; // relocation cost h_L^A and h_L^B
		basic.h[0][2] = 0.06; // idle cost h_I^A and h_I^B
		basic.trans_price[0] = 0.12; // transfer cost h_T
		basic.trans_price[1] = 0.12; // transfer cost h_T
		basic.alpha[0] = 0.1; // alpha_A, the percentage of consumers who are loyal to firm A
		basic.alpha[1] = 0.45; // alpha_B, the percentage of consumers who are loyal to firm B
		basic.start_hour = 6; // the operational horizon starts from 6:00
		basic.obj_option = 3; // choose the equilibrium strategy, 3: minimizes the expected total demand loss, i.e., (9) in the paper; 4: minimizes the total number of allocated vehicles, i.e., (10)
		basic.sharing_option = 1; // choose if we consider the capacity sharing, 1: yes; 0: no
		basic.service = 0.95; // required service level
		basic.competition_option = 1; // choose if we consider the competition between two firms, 1: yes; 0: no
		basic.differ_ratio = 1.25; // u

		basic.cost_purchase[1] = basic.cost_purchase[0] * basic.differ_ratio;
		basic.penalty[1] = basic.penalty[0] * basic.differ_ratio;
		basic.h[1][1] = basic.h[0][1] * basic.differ_ratio;
		basic.h[1][2] = basic.h[0][2] * basic.differ_ratio;

		cout << "--basic parameters are input--" << endl;

		string root = "the path where user stores the code folder";
		input(initial, basic, root);
		cout << endl << "--data from csv is input--" << endl;
		demand_prop(initial, basic, root);
		demand_assign(initial, basic, root);
		cout << endl << "--demand data is input--" << endl;

		//****************************** Build Model ******************************

		primal_variable primal(initial, basic);
		cout << "--primal variables defined--" << endl;

		if (basic.competition_option == 1) {
			create_obj(initial, basic, primal);
			cout << "--obj function is added--" << endl;
			create_primal_problem(initial, basic, primal);
			cout << "--primal cons are added--" << endl;

			if (basic.obj_option > 1) {
				dual_variable dual(initial, basic);
				cout << "--dual variables defined--" << endl;
				create_station_problem(initial, basic, dual);
				cout << "--stationarity cons are added--" << endl;
				create_comp_problem(initial, basic, primal, dual);
				cout << "--complementary cons are added--" << endl;
			}

			basic.cplex.extract(basic.model);
			cout << "--model is extracted--" << endl;

			//****************************** Solve Model ******************************

			basic.cplex.setParam(IloCplex::TiLim, 36000);
			basic.cplex.setParam(IloCplex::EpGap, 0.001);

			std::chrono::time_point<std::chrono::system_clock>  time_start = std::chrono::system_clock::now();
			if (!basic.cplex.solve()) {
				basic.env.error() << "Failed to optimize LP firm1." << endl;
				throw(-1);
			}
			else {
				cout << "Solved Successfully!" << endl;
			}
			std::chrono::time_point<std::chrono::system_clock>  time_end = std::chrono::system_clock::now();
			std::chrono::duration<double> time_length = time_end - time_start;
			basic.time = time_length.count();
		}
		else {
			create_no_competition(initial, basic, primal);
			basic.cplex.extract(basic.model_independent);
			if (!basic.cplex.solve()) {
				basic.env.error() << "Failed to optimize LP firm1." << endl;
					throw(-1);
			}
			else {
				cout << "Solved Successfully!" << endl;
			}
		}

		//****************************** Output Results ******************************
		
		if (basic.competition_option == 1 && basic.obj_option > 1) {
			resolve_problem(initial, basic, primal);
		}
		if (basic.sharing_option == 0) {
			output(initial, basic, primal, root);
		}
		else {
			output_service_nosharing(initial, basic, primal, root);
			output_loss_nosharing(initial, basic, primal, root);
			output_utility_nosharing(initial, basic, primal, root);
			output_transfer(initial, basic, primal, root);
			output_summary(initial, basic, primal, root);
		}
		
	}

	catch (IloException& e) {

		cerr << "Concert exception caught: " << e << endl;
	}
	catch (...) {
		cerr << "Unknown exception caught" << endl;
	}
	//system("pause");
	return 0;
} // END main