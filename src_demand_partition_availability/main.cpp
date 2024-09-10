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


int main(int argc, char** argv)
{

	try {

		//****************************** Parameters Work ******************************

		initial_parameter initial;
		cout << "--initial parameters are defined--" << endl;
		
		initial.T = 3;
		initial.nb_points = 2;
		initial.nb_nodes = 3;
		initial.nb_plates = 60;
		initial.budget_A = 1500;
		initial.budget_B = 1500;
		initial.demand_level = 1;

		basic_parameter basic(initial.T, initial.nb_points, initial.nb_nodes, initial.nb_plates, initial.budget_A, initial.budget_B, initial.demand_level);

		basic.cost_purchase[0] = 0.16;
		basic.penalty[0] = 0.1;
		basic.rental_price[0] = 0.4;
		basic.rental_price[1] = 0.4;
		basic.h[0][1] = 0.12;
		basic.h[0][2] = 0.06;
		basic.trans_price[0] = 0.12;
		basic.trans_price[1] = 0.12;
		basic.alpha[0] = 0.25;
		basic.alpha[1] = 0.25;
		basic.start_hour = 6;
		basic.obj_option = 3;
		basic.sharing_option = 1;
		basic.service = 0.95;
		basic.competition_option = 1;
		basic.differ_ratio = 1;
		basic.model_option = 1; // 0: nonlinear, 1: linear

		basic.cost_purchase[1] = basic.cost_purchase[0] * basic.differ_ratio;
		basic.penalty[1] = basic.penalty[0] * basic.differ_ratio;
		basic.h[1][1] = basic.h[0][1] * basic.differ_ratio;
		basic.h[1][2] = basic.h[0][2] * basic.differ_ratio;

		cout << "--basic parameters are input--" << endl;

		string root = "the path where user stores the code folder/";
		input(initial, basic, root);
		cout << endl << "--data from csv is input--" << endl;
		demand_prop(initial, basic, root);
		demand_assign(initial, basic, root);
		cout << endl << "--demand data is input--" << endl;

		//****************************** Build Model ******************************

		primal_variable primal(initial, basic);
		cout << "--primal variables defined--" << endl;

		if (basic.competition_option == 1) {

			if (basic.model_option == 0) {
				int iter = 0;
				basic.cplex.setParam(IloCplex::EpGap, 0.0001);
				start_point(initial, basic, primal);

				std::chrono::time_point<std::chrono::system_clock>  time_start = std::chrono::system_clock::now();

				do {
					// assign value in the last iteration
					assign_historical_val(initial, basic, primal);
					// solve Firm A
					create_firmA_problem(initial, basic, primal);
					basic.cplex.extract(basic.model);

					if (!basic.cplex.solve()) {
						basic.env.error() << "Failed to optimize Firm A." << endl;
						throw(-1);
					}
					else {
						cout << "Solved Firm A Successfully!" << endl;
					}

					get_val(initial, basic, primal, 0);

					basic.cplex.clear();
					basic.model.remove(basic.myobj);
					basic.model.remove(basic.mycons);
					basic.mycons.clear();

					// solve Firm B
					create_firmB_problem(initial, basic, primal);
					basic.cplex.extract(basic.model);
					if (!basic.cplex.solve()) {
						basic.env.error() << "Failed to optimize Firm B." << endl;
						throw(-1);
					}
					else {
						cout << "Solved Firm B Successfully!" << endl;
					}

					get_val(initial, basic, primal, 1);

					basic.cplex.clear();
					basic.model.remove(basic.myobj);
					basic.model.remove(basic.mycons);
					basic.mycons.clear();

					iter++;

					cout << endl << "=========== ITERATION " << iter << " =========== " << endl;


				} while (converge_check(initial, basic, iter));

				std::chrono::time_point<std::chrono::system_clock>  time_end = std::chrono::system_clock::now();
				std::chrono::duration<double> time_length = time_end - time_start;
				basic.time = time_length.count();

				cout << endl << "time = " << basic.time << endl;

				output_nonlinear(initial, basic, primal, root);
			}
			else {
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

				//char model_lp[50];
				//sprintf(model_lp, "/hpc/puhome/20031616r/Bicycle_Sharing_new/results/problem.lp");
				//basic.cplex.exportModel(model_lp);
				cout << "--model is printed--" << endl;
				cout << "--now start to solve the model, pleases see the txt file--" << endl;

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

				if (basic.competition_option == 1 && basic.obj_option > 1) {
					resolve_problem(initial, basic, primal);
				}
				if (basic.sharing_option == 0) {
					output(initial, basic, primal, root);
				}
				else {
					//output_service_nosharing(initial, basic, primal, root);
					//output_loss_nosharing(initial, basic, primal, root);
					//output_utility_nosharing(initial, basic, primal, root);
					//output_transfer(initial, basic, primal, root);
					output_summary(initial, basic, primal, root);
				}
			}
			
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