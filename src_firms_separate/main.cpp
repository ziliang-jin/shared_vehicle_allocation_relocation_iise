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

		initial.T = 48;
		initial.nb_points = 6;
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
		basic.alpha[0] = 0.1;
		basic.alpha[1] = 0.45;
		basic.start_hour = 6;
		basic.obj_option = 3;
		basic.sharing_option = 1;
		basic.service = 0.95;
		basic.competition_option = 1;
		basic.differ_ratio = 1.25;
		bool loyal_option = 1; // 1: loyal model; 0: disloyal model

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

		vector<double> ratio(initial.nb_points);
		ratio.at(0) = 0.14; ratio.at(1) = 0.19; ratio.at(2) = 0.22; ratio.at(3) = 0.17; ratio.at(4) = 0.16; ratio.at(5) = 0.12;

		//double ratio_adjust = 0.08;
		double ratio_adjust = 0.08;

		if (loyal_option == 1) {

			//================= loyal model =================
			for (int i = 0; i < initial.nb_points; i++) {
				for (int ii = 0; ii < initial.nb_points; ii++) {
					for (int t = 1; t <= initial.T - 1; t++) {
						if (t + basic.period[i][ii] <= initial.T) {
							for (int s = 0; s < initial.nb_nodes; s++) {
								basic.Y1[i][ii][t][t + basic.period[i][ii]][s] = 0;
							}
						}
					}
				}
			}

			int sum_temp = 0;
			for (int i = 0; i < initial.nb_points - 1; i++) {
				basic.totallots[i] = static_cast<int>((basic.alpha[0] + basic.alpha[1] + ratio_adjust) * initial.nb_plates * initial.nb_points * initial.demand_level * ratio.at(i));
				sum_temp += basic.totallots[i];
			}

			basic.totallots.at(initial.nb_points - 1) = (basic.alpha[0] + basic.alpha[1] + ratio_adjust) * initial.nb_plates * initial.nb_points * initial.demand_level - sum_temp;

			create_obj(initial, basic, primal);
			cout << "--obj function is added--" << endl;
			create_primal_problem(initial, basic, primal);
			cout << "--primal cons are added--" << endl;
			dual_variable dual(initial, basic);
			cout << "--dual variables defined--" << endl;
			create_station_problem(initial, basic, dual);
			cout << "--stationarity cons are added--" << endl;
			create_comp_problem(initial, basic, primal, dual);
			cout << "--complementary cons are added--" << endl;

			basic.cplex.extract(basic.model);
			cout << "--model is extracted--" << endl;

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
			//================= disloyal model =================
			
			for (int i = 0; i < initial.nb_points; i++) {
				for (int ii = 0; ii < initial.nb_points; ii++) {
					for (int t = 1; t <= initial.T - 1; t++) {
						if (t + basic.period[i][ii] <= initial.T) {
							for (int k = 0; k < basic.nb_firms; k++) {
								for (int s = 0; s < initial.nb_nodes; s++) {
									basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] = 0;
								}
							}
						}
					}
				}
			}

			int sum_temp = 0;
			for (int i = 0; i < initial.nb_points - 1; i++) {
				basic.totallots[i] = static_cast<int>( (1-basic.alpha[0]-basic.alpha[1]-ratio_adjust) * initial.nb_plates * initial.nb_points * initial.demand_level * ratio.at(i));
				sum_temp += basic.totallots[i];
			}

			basic.totallots.at(initial.nb_points - 1) = (1-basic.alpha[0]-basic.alpha[1]-ratio_adjust) * initial.nb_plates * initial.nb_points * initial.demand_level - sum_temp;
			
			create_obj(initial, basic, primal);
			cout << "--obj function is added--" << endl;
			create_primal_problem(initial, basic, primal);
			cout << "--primal cons are added--" << endl;
			dual_variable dual(initial, basic);
			cout << "--dual variables defined--" << endl;
			create_station_problem(initial, basic, dual);
			cout << "--stationarity cons are added--" << endl;
			create_comp_problem(initial, basic, primal, dual);
			cout << "--complementary cons are added--" << endl;

			basic.cplex.extract(basic.model);
			cout << "--model is extracted--" << endl;

			basic.cplex.setParam(IloCplex::TiLim, 540);
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

		resolve_problem(initial, basic, primal);

		output_summary(initial, basic, primal, root);
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