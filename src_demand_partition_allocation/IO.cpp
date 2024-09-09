#include "IO.h"
using namespace std;

void input(initial_parameter initial, basic_parameter& basic, string root){
	
	//======================= Demand =======================

	if (initial.nb_points > 6) {
		cerr << "the number of regions can not exceed 6!" << endl;
	}

	string value;

/*	string demand_loc = root + "data/base_demand.csv";
	ifstream demand_file(demand_loc);
	if (!demand_file.good()){
		cerr << "can not open " << demand_loc << endl;
	}
	else{
		for (int i = 0; i <= initial.nb_points; i++){
			for (int j = 0; j <= initial.nb_points; j++)
			{
				if (j != initial.nb_points){
					getline(demand_file, value, ',');
				}
				else{
					getline(demand_file, value, '\n');
				}
				if (i >= 1 && j >= 1){
					basic.demand_value[i - 1][j - 1] = atof(value.c_str());
				}
			}
		}
	} */

	string demand_loc = root + "data/demand.csv";
	ifstream demand_file(demand_loc);
	if (!demand_file.good()) {
		cerr << "can not open " << demand_loc << endl;
	}
	else {
		for (int i = 0; i <= 4346; i++) {
			for (int j = 0; j <= 3; j++){
				if (j != 3) {
					getline(demand_file, value, ',');
				}
				else {
					getline(demand_file, value, '\n');
				}
				if (i >= 1 && j >= 0) {
					basic.demand_value[i - 1][j] = atof(value.c_str());
				}
			}
		}
	}

	//======================= Period =======================
	string period_loc = root + "data/period.csv";
	ifstream period_file(period_loc);
	if (!period_file.good()){
		cerr << "can not open " << period_loc << endl;
	}
	else{
		for (int i = 0; i <= initial.nb_points; i++){
			for (int j = 0; j <= initial.nb_points; j++){
				if (j != initial.nb_points){
					getline(period_file, value, ',');
				}
				else{
					getline(period_file, value, '\n');
				}
				if (i >= 1 && j >= 1){
					basic.period[i - 1][j - 1] = atof(value.c_str());
				}
			}
		}
	}

/*	//======================= Time Multiplier =======================
	string multiplier_loc = root + "data/time_multiplier.csv";
	ifstream multiplier_file(multiplier_loc);
	if (!multiplier_file.good()){
		cerr << "can not open " << multiplier_loc << endl;
	}
	else{
		for (int t = 0; t <= 24; t++){
			getline(multiplier_file, value, ',');
			basic.time_multiplier[t] = atof(value.c_str());
		}
	}*/
} // the end of input

void demand_gen(initial_parameter initial, basic_parameter& basic){

	int seed = 7;

	vector< vector< vector<double> > > demand_24h(6, vector<vector<double>>(6, vector<double>(145, 0)));
	
	// assign demands to each trip in 24h (start, end, time)
	for (int i = 0; i <= 4345; i++) {
		demand_24h[basic.demand_value[i][0] - 1][basic.demand_value[i][1] - 1][basic.demand_value[i][2]] = basic.demand_value[i][3];
	}

	// assign demands to each trip in 48 time periods
	int start = basic.start_hour * 6;

	for (int i = 0; i < initial.nb_points; i++) {
		for (int j = 0; j < initial.nb_points; j++) {
			for (int t = 0; t <= initial.T; t++) {
				basic.base_demand[i][j][t] = demand_24h[i][j][start + t];
			}
		}
	}

	int nb_seed = 1;
	if (abs(basic.alpha[1] - basic.alpha[0]) <= 0.05 && basic.alpha[1] - basic.alpha[0] != 0) {
		nb_seed = 100;
	}

	// Normal Distribution
	
	double temp_gen = 0;
	
	std::default_random_engine gen(seed);

	for (int i = 0; i < initial.nb_points; i++) {
		for (int ii = 0; ii < initial.nb_points; ii++) {
			for (int t = 1; t <= initial.T - 1; t++) {
				if (t + basic.period[i][ii] <= initial.T) {
					double mean = (1 - basic.alpha[0] - basic.alpha[1]) * basic.base_demand[i][ii][t];
					double sigma = 0.2 * mean;
					// obtain Y1
					std::normal_distribution<double> dis(mean, sigma);						
					for (int s = 0; s < initial.nb_nodes; s++) {
						temp_gen = dis(gen);
						if (temp_gen < 0) {
							temp_gen = 0;
						}
						basic.Y1[i][ii][t][t + basic.period[i][ii]][s] = temp_gen;
					}
					// obtain Y
					for (int k = 0; k < basic.nb_firms; k++) {
						if (basic.alpha[k] == 0) {
							for (int s = 0; s < initial.nb_nodes; s++) {
								basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] = 0;
							}
						}
						else {
							double mean_e = basic.alpha[k] * basic.base_demand[i][ii][t];
							double sigma_e = (1 + basic.alpha[1 - k] / basic.alpha[k]) * sigma;
							std::normal_distribution<double> dis_e(mean_e, sigma_e);
							for (int s = 0; s < initial.nb_nodes; s++) {
								temp_gen = dis_e(gen);
								if (temp_gen < 0) {
									temp_gen = 0;
								}
								basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] = temp_gen;
							}
						}
					}
				}
			}
		}
	}

	for (int nb = 1; nb < nb_seed; nb++) {

		std::default_random_engine gen(seed + nb);

		for (int i = 0; i < initial.nb_points; i++) {
			for (int ii = 0; ii < initial.nb_points; ii++) {
				for (int t = 1; t <= initial.T - 1; t++) {
					if (t + basic.period[i][ii] <= initial.T) {
						double mean = (1 - basic.alpha[0] - basic.alpha[1]) * basic.base_demand[i][ii][t];
						double sigma = 0.2 * mean;
						// obtain Y
						for (int k = 0; k < basic.nb_firms; k++) {
							double mean_e = basic.alpha[k] * basic.base_demand[i][ii][t];
							double sigma_e = (1 + basic.alpha[1 - k] / basic.alpha[k]) * sigma;
							std::normal_distribution<double> dis_e(mean_e, sigma_e);
							for (int s = 0; s < initial.nb_nodes; s++) {
								temp_gen = dis_e(gen);
								if (temp_gen < 0) {
									temp_gen = 0;
								}
								basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] += temp_gen;
							}
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < initial.nb_points; i++) {
		for (int ii = 0; ii < initial.nb_points; ii++) {
			for (int t = 1; t <= initial.T - 1; t++) {
				if (t + basic.period[i][ii] <= initial.T) {				
					// obtain Y
					for (int k = 0; k < basic.nb_firms; k++) {
						for (int s = 0; s < initial.nb_nodes; s++) {
							basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] = basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] / nb_seed;
						}
					}
				}
			}
		}
	}
	

/*	// Uniform Distribution

	for (int nb = 1; nb < 51; nb++) {
		srand(seed + nb);
		for (int i = 0; i < initial.nb_points; i++) {
			for (int ii = 0; ii < initial.nb_points; ii++) {
				for (int t = 1; t <= initial.T - 1; t++) {
					if (t + basic.period[i][ii] <= initial.T) {
						double mean = (1 - basic.alpha[0] - basic.alpha[1]) * basic.base_demand[i][ii][t];
						// obtain Y1
						for (int s = 0; s < initial.nb_nodes; s++) {
							basic.Y1[i][ii][t][t + basic.period[i][ii]][s] += (double)(rand() / (double)RAND_MAX) * mean + 0.5 * mean;
						}
						// obtain Y
						for (int k = 0; k < basic.nb_firms; k++) {
							double mean_e = basic.alpha[k] * basic.base_demand[i][ii][t];
							for (int s = 0; s < initial.nb_nodes; s++) {
								basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] += (double)(rand() / (double)RAND_MAX) * mean_e + 0.5 * mean_e;
							}
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < initial.nb_points; i++) {
		for (int ii = 0; ii < initial.nb_points; ii++) {
			for (int t = 1; t <= initial.T - 1; t++) {
				if (t + basic.period[i][ii] <= initial.T) {
					// obtain Y1
					for (int s = 0; s < initial.nb_nodes; s++) {
						basic.Y1[i][ii][t][t + basic.period[i][ii]][s] = basic.Y1[i][ii][t][t + basic.period[i][ii]][s] / 50;
					}
					// obtain Y
					for (int k = 0; k < basic.nb_firms; k++) {
						for (int s = 0; s < initial.nb_nodes; s++) {
							basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] = basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] / 50;
						}
					}
				}
			}
		}
	}

	// poisson distribution
	double temp_gen = 0;

	for (int nb = 1; nb < 21; nb++) {
		std::default_random_engine gen(seed + nb);

		for (int i = 0; i < initial.nb_points; i++) {
			for (int ii = 0; ii < initial.nb_points; ii++) {
				for (int t = 1; t <= initial.T - 1; t++) {
					if (t + basic.period[i][ii] <= initial.T) {
						double mean = (1 - basic.alpha[0] - basic.alpha[1]) * basic.base_demand[i][ii][t];
						// obtain Y1
						std::poisson_distribution<int> dis(mean);
						for (int s = 0; s < initial.nb_nodes; s++) {
							temp_gen = dis(gen);
							if (temp_gen < 0) {
								temp_gen = 0;
							}
							basic.Y1[i][ii][t][t + basic.period[i][ii]][s] += temp_gen;
						}
						// obtain Y
						for (int k = 0; k < basic.nb_firms; k++) {
							double mean_e = basic.alpha[k] * basic.base_demand[i][ii][t];
							std::poisson_distribution<int> dis_e(mean_e);
							for (int s = 0; s < initial.nb_nodes; s++) {
								temp_gen = dis_e(gen);
								if (temp_gen < 0) {
									temp_gen = 0;
								}
								basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] += temp_gen;
							}
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < initial.nb_points; i++) {
		for (int ii = 0; ii < initial.nb_points; ii++) {
			for (int t = 1; t <= initial.T - 1; t++) {
				if (t + basic.period[i][ii] <= initial.T) {
					// obtain Y1
					for (int s = 0; s < initial.nb_nodes; s++) {
						basic.Y1[i][ii][t][t + basic.period[i][ii]][s] = basic.Y1[i][ii][t][t + basic.period[i][ii]][s] / 20;
					}
					// obtain Y
					for (int k = 0; k < basic.nb_firms; k++) {
						for (int s = 0; s < initial.nb_nodes; s++) {
							basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] = basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] / 20;
						}
					}
				}
			}
		}
	}
	*/

	// Yk be the same
	if (basic.alpha[0] == basic.alpha[1]) {
		for (int i = 0; i < initial.nb_points; i++) {
			for (int ii = 0; ii < initial.nb_points; ii++) {
				for (int t = 1; t <= initial.T - 1; t++) {
					for (int s = 0; s < initial.nb_nodes; s++) {
						if (t + basic.period[i][ii] <= initial.T) {
							basic.Y[1][i][ii][t][t + basic.period[i][ii]][s] = basic.Y[0][i][ii][t][t + basic.period[i][ii]][s];
						}
					}
				}
			}
		}
	}

}

void demand_prop(initial_parameter initial, basic_parameter& basic, string root) {

	int start = basic.start_hour * 6;

	string prop_filename;
	string prop_file;
	string prop_loc = root + "data/proportion/";
	string pickvalue;

	for (int t = 1; t <= initial.T; t++) {
		prop_filename = "proportion_" + to_string(start + t) + ".csv";
		prop_file = prop_loc + prop_filename;
		ifstream prop_stream(prop_file);

		if (!prop_stream.good()) {
			cerr << "can not open " << prop_loc << endl;
		}
		else {
			for (int i = 0; i <= initial.nb_points; i++) {
				for (int j = 0; j <= initial.nb_points; j++) {
					if (j != initial.nb_points) {
						getline(prop_stream, pickvalue, ',');
					}
					else {
						getline(prop_stream, pickvalue, '\n');
					}
					if (i >= 1 && j >= 1) {
						basic.proportion.at(t).at(i-1).at(j-1) = atof(pickvalue.c_str());
					}
				}
			}
		}

	}

}

void demand_assign(initial_parameter initial, basic_parameter& basic, string root) {

	int start = basic.start_hour * 6;

	string demand_filename;
	string demand_file;
	string demand_loc;
	string pickvalue;

	if (initial.nb_nodes == 3) {
		demand_loc = root + "data/demand_cluster/";
	}
	else {
		demand_loc = root + "data/demand_1000/";
	}

	cout << endl << demand_loc << endl;

	for (int s = 1; s <= initial.nb_nodes; s++) {
		demand_filename = "demand_" + to_string(s) + ".csv";
		demand_file = demand_loc + demand_filename;
		ifstream demand_stream(demand_file);

		if (!demand_stream.good()) {
			cerr << "can not open " << demand_loc << endl;
		}
		else {
			for (int i = 0; i <= initial.nb_points; i++) {
				for (int t = 0; t <= 144; t++) {
					if (t != 144) {
						getline(demand_stream, pickvalue, ',');
					}
					else {
						getline(demand_stream, pickvalue, '\n');
					}
					if (i >= 1 && t >= 1) {
						basic.demand_region.at(t).at(i-1).at(s-1) = atof(pickvalue.c_str()) * initial.demand_level * 2; // we have two firms so times 2
					}
				}
			}
		}
	}

	// disloyal customers
	for (int i = 0; i < initial.nb_points; i++) {
		for (int ii = 0; ii < initial.nb_points; ii++) {
			for (int t = 1; t <= initial.T - 1; t++) {
				if (t + basic.period[i][ii] <= initial.T) {
					for (int s = 0; s < initial.nb_nodes; s++) {
						basic.Y1[i][ii][t][t + basic.period[i][ii]][s] = (1 - basic.alpha[0] - basic.alpha[1])
							* basic.demand_region.at(start + t).at(i).at(s) * basic.proportion.at(t).at(i).at(ii);
					}
				}
			}
		}
	}

	// disloyal customers
	for (int i = 0; i < initial.nb_points; i++) {
		for (int ii = 0; ii < initial.nb_points; ii++) {
			for (int t = 1; t <= initial.T - 1; t++) {
				if (t + basic.period[i][ii] <= initial.T) {
					for (int k = 0; k < basic.nb_firms; k++) {
						for (int s = 0; s < initial.nb_nodes; s++) {
							basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] = basic.alpha.at(k)
								* basic.demand_region.at(start + t).at(i).at(s) * basic.proportion.at(t).at(i).at(ii);
						}
					}
				}
			}
		}
	}

}

// help fucntion of calculating the costs
void objvalue(initial_parameter initial, basic_parameter& basic, primal_variable primal){

	for (int k = 0; k < basic.nb_firms; k++){
		double totalobj = 0;
		double obj1 = 0; // stage 1
		double obj2 = 0; // stage 2

		cout << endl << "Firm_" << k << endl;
		
		//================ stage 1 ================

		//----- costs of allocations -----
		double sumx = 0;

		for (int j = 0; j < initial.nb_points; j++){
			sumx += basic.cplex.getValue(primal.park[k][j]);
		}

		obj1 += basic.cost_purchase[k] * sumx;

		cout << endl << "allocation cost = " << obj1 << endl;

		//================ stage 2 ================

		for (int s = 0; s < initial.nb_nodes; s++){
			double sum_rental_cost = 0;

			cout << endl << "Sce_" << s << endl;

			//----- costs of rental arcs -----
			for (int i = 0; i < initial.nb_points; i++){
				for (int ii = 0; ii < initial.nb_points; ii++){
					for (int t = 1; t < initial.T; t++){
						if (t + basic.period[i][ii] <= initial.T){
							sum_rental_cost += -basic.rental_price[k] * basic.cplex.getValue(primal.flow[k][i][ii][t][0][s]);
							sum_rental_cost += basic.penalty[k] * (basic.cplex.getValue(primal.demandvar[k][i][ii][t][s]) - basic.cplex.getValue(primal.flow[k][i][ii][t][0][s]));
						}
					}
				}
			}
			cout << endl << "rental_cost = " << sum_rental_cost << endl;
			//----- costs of relocation arcs -----
			double sum_relocation_cost = 0;
			for (int i = 0; i < initial.nb_points; i++){
				for (int ii = 0; ii < initial.nb_points; ii++){
					for (int t = 1; t < initial.T; t++){
						if (t + basic.period[i][ii] <= initial.T){
							sum_relocation_cost += basic.h[k][1] * basic.cplex.getValue(primal.flow[k][i][ii][t][1][s]);
						}
					}
				}
			}
			cout << endl << "relocation_cost = " << sum_relocation_cost << endl;
			//----- costs of idle arcs -----
			double sum_idle_cost = 0;
			for (int i = 0; i < initial.nb_points; i++){
				for (int t = 0; t < initial.T; t++){
					sum_idle_cost += basic.h[k][2] * basic.cplex.getValue(primal.flow[k][i][i][t][2][s]);
				}
			}
			cout << endl << "idle_cost = " << sum_idle_cost << endl;
			//----- costs of transfer arcs -----
			double sum_trans_cost = 0;
			if (basic.sharing_option == 1) {
				for (int i = 0; i < initial.nb_points; i++) {
					for (int t = 1; t <= initial.T - 1; t++) {
						sum_trans_cost += -basic.trans_price[k] * basic.cplex.getValue(primal.flow[k][i][i][t][3][s]);
						sum_trans_cost += basic.trans_price[1 - k] * basic.cplex.getValue(primal.flow[1 - k][i][i][t][3][s]);
					}
				}
			}
			cout << endl << "trans_cost = " << sum_trans_cost << endl;
			//----- total costs of stage 2 -----
			obj2 += basic.probTheta[s] * (sum_rental_cost + sum_relocation_cost + sum_idle_cost + sum_trans_cost);

		}
		basic.totalcost[k] = obj1 + obj2;
	}

}

void output(initial_parameter initial, basic_parameter basic, primal_variable primal, string root){

	// ====================================== basic output ======================================

	stringstream filename_output;
	ofstream outresult;

	// cout objvalue
	filename_output << root << "/results/schedule_P_" << initial.nb_plates << "_S_" << initial.nb_nodes << "_alpha_" << basic.alpha[0] << '_' << basic.alpha[1]
		<< "_B_" << initial.budget_A << '_'<< initial.budget_B << "_option_" << basic.obj_option << "_sharing_" << basic.sharing_option << "_differ_" << basic.differ_ratio << "_comp_" << basic.competition_option << "_cp_" << basic.penalty[0] << "_l_" << initial.demand_level << ".csv";
	outresult.open(filename_output.str().c_str());
	//outresult << "Obj Value" << ',' << basic.cplex.getObjValue() << endl;

	outresult << "Time" << ',' << basic.time << endl;

	// cout the cost of each firm
	objvalue(initial, basic, primal);

	outresult << "Expected Cost" << endl;
	outresult << "Firm1" << ',' << "Firm2" << ',' << "Total" << endl;
	outresult << basic.totalcost[0] << ',' << basic.totalcost[1] << ',' << basic.totalcost[0] + basic.totalcost[1] << endl;

	// cout the allocation of each firm
	double sum_park_1 = 0; double sum_park_2 = 0;
	outresult << endl << "Allocation" << endl;
	for (int i = 0; i < initial.nb_points; i++){
		outresult << basic.cplex.getValue(primal.park[0][i]) << ',';
		sum_park_1 += basic.cplex.getValue(primal.park[0][i]);
	}
	outresult << ',' << ','; 
	for (int i = 0; i < initial.nb_points; i++){
		outresult << basic.cplex.getValue(primal.park[1][i]) << ',';
		sum_park_2 += basic.cplex.getValue(primal.park[1][i]);
	}
	outresult << endl << "Firm1" << ',' << "Firm2" << ',' << "Total" << endl;
	outresult << sum_park_1 << ',' << sum_park_2 << ',' << sum_park_1 + sum_park_2 << endl;

	// cout the ratio of total transfers and total satisfied demands
	double total_trans_A = 0; double total_trans_B = 0; double total_trans = 0;
	vector<double> satisfied_dmd_A(initial.nb_nodes); vector<double> satisfied_dmd_B(initial.nb_nodes);
	double total_satisfied_dmd_A = 0; double total_satisfied_dmd_B = 0;
	vector<double> dmd_A(initial.nb_nodes); vector<double> dmd_B(initial.nb_nodes);
	double total_dmd_A = 0; double total_dmd_B = 0; double total_dmd = 0;
	double total_dmd_loss_A = 0; double total_dmd_loss_B = 0; double total_dmd_loss = 0;
	double total_service_level_A = 0; double total_service_level_B = 0; double total_service_level = 0;
	double ratio_A = 0; double ratio_B = 0; double ratio_T = 0;

	// total satisfied demands
	for (int i = 0; i < initial.nb_points; i++) {
		for (int j = 0; j < initial.nb_points; j++) {
			for (int t = 1; t <= initial.T - 1; t++) {
				for (int s = 0; s < initial.nb_nodes; s++) {
					if (j != i && t + basic.period[i][j] <= initial.T) {
						satisfied_dmd_A.at(s) += basic.cplex.getValue(primal.flow[0][i][j][t][0][s]);
						satisfied_dmd_B.at(s) += basic.cplex.getValue(primal.flow[1][i][j][t][0][s]);
					}
				}
			}
		}
	}

	for (int s = 0; s < initial.nb_nodes; s++) {
		total_satisfied_dmd_A += basic.probTheta.at(s) * satisfied_dmd_A.at(s);
		total_satisfied_dmd_B += basic.probTheta.at(s) * satisfied_dmd_B.at(s);
	}

	//total demands
	for (int i = 0; i < initial.nb_points; i++) {
		for (int ii = 0; ii < initial.nb_points; ii++) {
			for (int t = 1; t <= initial.T - 1; t++) {
				for (int s = 0; s < initial.nb_nodes; s++) {
					if (i != ii && t + basic.period[i][ii] <= initial.T) {
						dmd_A.at(s) += basic.cplex.getValue(primal.demandvar[0][i][ii][t][s]);
						dmd_B.at(s) += basic.cplex.getValue(primal.demandvar[1][i][ii][t][s]);
					}
				}
			}
		}
	}

	for (int s = 0; s < initial.nb_nodes; s++) {
		total_dmd_A += basic.probTheta.at(s) * dmd_A.at(s);
		total_dmd_B += basic.probTheta.at(s) * dmd_B.at(s);
		total_dmd += basic.probTheta.at(s) * (dmd_A.at(s) + dmd_B.at(s));
	}

	// total demand loss
	for (int s = 0; s < initial.nb_nodes; s++) {
		total_dmd_loss_A += basic.probTheta.at(s) * (dmd_A.at(s) - satisfied_dmd_A.at(s));
		total_dmd_loss_B += basic.probTheta.at(s) * (dmd_B.at(s) - satisfied_dmd_B.at(s));
		total_dmd_loss += basic.probTheta.at(s) * (dmd_A.at(s) + dmd_B.at(s) - satisfied_dmd_A.at(s) - satisfied_dmd_B.at(s));
	}

	// total service level
	if (sum_park_1 > 0) {
		for (int s = 0; s < initial.nb_nodes; s++) {
			total_service_level_A += basic.probTheta.at(s) * satisfied_dmd_A.at(s) / dmd_A.at(s) * 100.;
		}
	}
	else {
		total_service_level_A = -1;
	}

	if (sum_park_2 > 0) {
		for (int s = 0; s < initial.nb_nodes; s++) {
			total_service_level_B += basic.probTheta.at(s) * satisfied_dmd_B.at(s) / dmd_B.at(s) * 100.;
		}
	}
	else {
		total_service_level_B = -1;
	}


	if (sum_park_1 + sum_park_2 > 0) {
		for (int s = 0; s < initial.nb_nodes; s++) {
			total_service_level += basic.probTheta.at(s) * (satisfied_dmd_A.at(s) + satisfied_dmd_B.at(s)) / (dmd_A.at(s) + dmd_B.at(s)) * 100.;
		}
	}
	else {
		total_service_level = -1;
	}


	// relocation
	double relocation_A = 0; double relocation_B = 0; double relocation_total = 0;
	vector<double> relocation_time_A(initial.T+1, 0); vector<double> relocation_time_B(initial.T + 1, 0); vector<double> relocation_time_total(initial.T + 1, 0);
	vector<vector<double> > relocation_space_A(initial.nb_points, vector<double>(initial.nb_points, 0));
	vector<vector<double> > relocation_space_B(initial.nb_points, vector<double>(initial.nb_points, 0));
	vector<vector<double> > relocation_space_Total(initial.nb_points, vector<double>(initial.nb_points, 0));
	for (int t = 1; t < initial.T; t++) {
		for (int s = 0; s < initial.nb_nodes; s++) {
			for (int i = 0; i < initial.nb_points; i++) {
				for (int j = 0; j < initial.nb_points; j++) {
					if (j != i && t + basic.period[i][j] <= initial.T) {
						relocation_time_A.at(t) += basic.probTheta.at(s) * basic.cplex.getValue(primal.flow[0][i][j][t][1][s]);
						relocation_time_B.at(t) += basic.probTheta.at(s) * basic.cplex.getValue(primal.flow[1][i][j][t][1][s]);
						relocation_time_total.at(t) += basic.probTheta.at(s) * (basic.cplex.getValue(primal.flow[0][i][j][t][1][s]) + basic.cplex.getValue(primal.flow[1][i][j][t][1][s]));
					}
				}
			}
		}
	}
	for (int t = 1; t < initial.T; t++) {
		relocation_A += relocation_time_A.at(t);
		relocation_B += relocation_time_B.at(t);
		relocation_total += relocation_time_total.at(t);
	}
	for (int i = 0; i < initial.nb_points; i++) {
		for (int j = 0; j < initial.nb_points; j++) {
			for (int t = 1; t < initial.T; t++) {
				for (int s = 0; s < initial.nb_nodes; s++) {
					if (j != i && t + basic.period[i][j] <= initial.T) {
						relocation_space_A.at(i).at(j) += basic.probTheta.at(s) / (initial.T-1) * basic.cplex.getValue(primal.flow[0][i][j][t][1][s]);
						relocation_space_B.at(i).at(j) += basic.probTheta.at(s) / (initial.T-1) * basic.cplex.getValue(primal.flow[1][i][j][t][1][s]);
						relocation_space_Total.at(i).at(j) += basic.probTheta.at(s) / (initial.T-1) * (basic.cplex.getValue(primal.flow[0][i][j][t][1][s]) + basic.cplex.getValue(primal.flow[1][i][j][t][1][s]));
					}
				}
			}
		}
	}

	/*if (basic.sharing_option == 1) {
		ratio_A = total_trans_A / total_satisfied_dmd_A;
		ratio_B = total_trans_B / total_satisfied_dmd_B;
		ratio_T = (total_trans_A + total_trans_B) / (total_satisfied_dmd_A + total_satisfied_dmd_B);

		outresult << endl << "Ratio_A" << ',' << "Ration_B" << ',' << "Ratio_T" << endl;
		outresult << ratio_A << ',' << ratio_B << ',' << ratio_T;
	}*/

	outresult << endl << "Total Demand" << endl;
	outresult << "Firm1" << ',' << "Firm2" << ',' << "Total" << endl;
	outresult << total_dmd_A << ',' << total_dmd_B << ',' << total_dmd << endl;

	outresult << endl << "Demand Loss" << endl;
	outresult << "Firm1" << ',' << "Firm2" << ',' << "Total" << endl;
	outresult << total_dmd_loss_A << ',' << total_dmd_loss_B << ',' << total_dmd_loss << endl;

	outresult << endl << "Service Level" << endl;
	outresult << "Firm1" << ',' << "Firm2" << ',' << "Total" << endl;
	outresult << total_service_level_A << ',' << total_service_level_B << ',' << total_service_level << endl;

	outresult << endl << "Relocation" << endl;
	outresult << "Firm1" << ',' << "Firm2" << ',' << "Total" << endl;
	outresult << relocation_A << ',' << relocation_B << ',' << relocation_total << endl;
	
	outresult << endl << "Firm A" << endl;
	for (int t = 1; t < initial.T; t++) {
		outresult << "T_" << t << ',';
	}
	outresult << endl;
	for (int t = 1; t < initial.T; t++) {
		outresult << relocation_time_A.at(t) << ',';
	}
	outresult << endl << "Firm B" << endl;
	for (int t = 1; t < initial.T; t++) {
		outresult << "T_" << t << ',';
	}
	outresult << endl;
	for (int t = 1; t < initial.T; t++) {
		outresult << relocation_time_B.at(t) << ',';
	}
	outresult << endl << "Total" << endl;
	for (int t = 1; t < initial.T; t++) {
		outresult << "T_" << t << ',';
	}
	outresult << endl;
	for (int t = 1; t < initial.T; t++) {
		outresult << relocation_time_total.at(t) << ',';
	}
	outresult << endl << endl << "Firm A" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		for (int j = 0; j < initial.nb_points; j++) {
			outresult << relocation_space_A.at(i).at(j) << ',';
		}
		outresult << endl;
	}
	outresult << "Firm B" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		for (int j = 0; j < initial.nb_points; j++) {
			outresult << relocation_space_B.at(i).at(j) << ',';
		}
		outresult << endl;
	}
	outresult << "Total" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		for (int j = 0; j < initial.nb_points; j++) {
			outresult << relocation_space_Total.at(i).at(j) << ',';
		}
		outresult << endl;
	}
} // the end of output function

void output_service_nosharing(initial_parameter initial, basic_parameter basic, primal_variable primal, string root) {
	stringstream filename_output;
	ofstream outresult;

	// ==================== output parameter ====================
	filename_output << root << "/results/service_alpha_" << basic.alpha[0] << '_' << basic.alpha[1]
		<< "_B_" << initial.budget_A << '_' << initial.budget_B << "_option_" << basic.obj_option << "_sharing_" << basic.sharing_option << "_differ_" << basic.differ_ratio << "_service_" << basic.service << "_cp_" << basic.penalty[0] << "_l_" << initial.demand_level << ".csv";
	outresult.open(filename_output.str().c_str());
	outresult << "Input Parameter" << endl;
	outresult << "Loyal Rate,# Regions,Budget,Procurement Cost,Reposition Cost,Idle Cost,Transfer Cost,Plates,Rental Price" << endl;
	outresult << '(' << basic.alpha[0] << ';' << basic.alpha[1] << ')' << ',' << initial.nb_points << ',' << initial.budget_A << ','
		<< '(' << basic.cost_purchase[0] << ';' << basic.cost_purchase[1] << ')' << ',' 
		<< '(' << basic.h[0][1] << ';' << basic.h[1][1] << ')' << ','
		<< '(' << basic.h[0][2] << ';' << basic.h[1][2] << ')' << ',' << basic.trans_price[0] << ',' << initial.nb_plates << ',' << basic.rental_price[0] << endl;
	// ==================== output results ====================
	outresult << "Results" << endl;
	// -------- Cost --------
	objvalue(initial, basic, primal);
	outresult << "FirmA's Cost" << ',' << basic.totalcost[0] << ',' << "FirmB's Cost" << ',' << basic.totalcost[1] << ',' << "Total Costs" << ',' << basic.cplex.getObjValue() << endl;
	// -------- Allocation --------
	double sum_park_1 = 0; double sum_park_2 = 0;
	outresult << "Allocation" << endl;
	outresult << "FirmA";
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << ',';
	}
	outresult << ',' << ',' << "FirmB" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << "Region_" << i + 1 << ',';
	}
	outresult << "Total" << ',' << ',';
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << "Region_" << i + 1 << ',';
	}
	outresult << "Total" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << basic.cplex.getValue(primal.park[0][i]) << ',';
		sum_park_1 += basic.cplex.getValue(primal.park[0][i]);
	}
	outresult << sum_park_1 << ',' << ',';
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << basic.cplex.getValue(primal.park[1][i]) << ',';
		sum_park_2 += basic.cplex.getValue(primal.park[1][i]);
	}
	outresult << sum_park_2 << endl;

	// Service Level
	// cout the ratio of total transfers and total satisfied demands	
	outresult << "Service Level" << endl;
	for (int s = 0; s < initial.nb_nodes; s++) {
		outresult << "Scenario_" << s + 1 << endl;
		for (int i = 0; i < initial.nb_points; i++) {
			// Firm A
			outresult << "A's Region_" << i + 1 << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				outresult << "Time_" << t << ',';
			}
			outresult << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				// calculate service level
				double total_satisfied_dmd_A = 0; double total_dmd_A = 0; double service_level = 0;
				for (int j = 0; j < initial.nb_points; j++) {
					if (j != i && t + basic.period[i][j] <= initial.T) {
						// FirmA's satisfied demands
						total_satisfied_dmd_A += basic.cplex.getValue(primal.flow[0][i][j][t][0][s]);
						// FirmA's demands
						total_dmd_A += basic.cplex.getValue(primal.demandvar[0][i][j][t][s]);
					}
				}
				// Service Level
				if(total_dmd_A == 0){
					service_level = 0;
				}
				else {
					service_level = total_satisfied_dmd_A / total_dmd_A;
				}
				outresult << service_level << ',';
			}
			outresult << endl;
			// Firm B
			outresult << "B's Region_" << i + 1 << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				outresult << "Time_" << t << ',';
			}
			outresult << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				// calculate service level
				double total_satisfied_dmd_B = 0; double total_dmd_B = 0; double service_level = 0;
				for (int j = 0; j < initial.nb_points; j++) {
					if (j != i && t + basic.period[i][j] <= initial.T) {
						// FirmB's satisfied demands
						total_satisfied_dmd_B += basic.cplex.getValue(primal.flow[1][i][j][t][0][s]);
						// FirmB's demands
						total_dmd_B += basic.cplex.getValue(primal.demandvar[1][i][j][t][s]);
					}
				}
				// Service Level
				if (total_dmd_B == 0) {
					service_level = 0;
				}
				else {
					service_level = total_satisfied_dmd_B / total_dmd_B;
				}
				outresult << service_level << ',';

			}
			outresult << endl;
			// Total
			outresult << "Total's Region_" << i + 1 << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				outresult << "Time_" << t << ',';
			}
			outresult << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				// calculate service level
				double total_satisfied_dmd = 0; double total_dmd = 0; double service_level = 0;
				for (int j = 0; j < initial.nb_points; j++) {
					if (j != i && t + basic.period[i][j] <= initial.T) {
						// Total firms' satisfied demands
						total_satisfied_dmd += basic.cplex.getValue(primal.flow[0][i][j][t][0][s]) + basic.cplex.getValue(primal.flow[1][i][j][t][0][s]);
						// Two firms' demands
						total_dmd += basic.cplex.getValue(primal.demandvar[0][i][j][t][s]) + basic.cplex.getValue(primal.demandvar[1][i][j][t][s]);
					}
				}
				// Service Level
				service_level = total_satisfied_dmd / total_dmd;
				outresult << service_level << ',';
			}
			outresult << endl << endl;
		}
		outresult << endl;
	}

}

void output_loss_nosharing(initial_parameter initial, basic_parameter basic, primal_variable primal, string root) {
	stringstream filename_output;
	ofstream outresult;

	// ==================== output parameter ====================
	filename_output << root << "/results/loss_alpha_" << basic.alpha[0] << '_' << basic.alpha[1]
		<< "_B_" << initial.budget_A << '_' << initial.budget_B << "_option_" << basic.obj_option << "_sharing_" << basic.sharing_option << "_differ_" << basic.differ_ratio << "_service_" << basic.service << "_cp_" << basic.penalty[0] << "_l_" << initial.demand_level << ".csv";
	outresult.open(filename_output.str().c_str());
	outresult << "Input Parameter" << endl;
	outresult << "Loyal Rate,# Regions,Budget,Procurement Cost,Reposition Cost,Idle Cost,Transfer Cost,Plates,Rental Price" << endl;
	outresult << '(' << basic.alpha[0] << ';' << basic.alpha[1] << ')' << ',' << initial.nb_points << ',' << initial.budget_A << ','
		<< '(' << basic.cost_purchase[0] << ';' << basic.cost_purchase[1] << ')' << ','
		<< '(' << basic.h[0][1] << ';' << basic.h[1][1] << ')' << ','
		<< '(' << basic.h[0][2] << ';' << basic.h[1][2] << ')' << ',' << basic.trans_price[0] << ',' << initial.nb_plates << ',' << basic.rental_price[0] << endl;
	// ==================== output results ====================
	outresult << "Results" << endl;
	// -------- Cost --------
	objvalue(initial, basic, primal);
	outresult << "FirmA's Cost" << ',' << basic.totalcost[0] << ',' << "FirmB's Cost" << ',' << basic.totalcost[1] << ',' << "Total Costs" << ',' << basic.cplex.getObjValue() << endl;
	// -------- Allocation --------
	double sum_park_1 = 0; double sum_park_2 = 0;
	outresult << "Allocation" << endl;
	outresult << "FirmA";
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << ',';
	}
	outresult << ',' << ',' << "FirmB" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << "Region_" << i + 1 << ',';
	}
	outresult << "Total" << ',' << ',';
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << "Region_" << i + 1 << ',';
	}
	outresult << "Total" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << basic.cplex.getValue(primal.park[0][i]) << ',';
		sum_park_1 += basic.cplex.getValue(primal.park[0][i]);
	}
	outresult << sum_park_1 << ',' << ',';
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << basic.cplex.getValue(primal.park[1][i]) << ',';
		sum_park_2 += basic.cplex.getValue(primal.park[1][i]);
	}
	outresult << sum_park_2 << endl;

	// Demand Loss
	outresult << "Demand Loss" << endl;
	for (int s = 0; s < initial.nb_nodes; s++) {
		outresult << "Scenario_" << s + 1 << endl;
		for (int i = 0; i < initial.nb_points; i++) {
			// Firm A
			outresult << "A's Region_" << i + 1 << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				outresult << "Time_" << t << ',';
			}
			outresult << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				double total_satisfied_dmd_A = 0; double total_dmd_A = 0; double demand_loss = 0;
				for (int j = 0; j < initial.nb_points; j++) {
					if (j != i && t + basic.period[i][j] <= initial.T) {
						// FirmA's satisfied demands
						total_satisfied_dmd_A += basic.cplex.getValue(primal.flow[0][i][j][t][0][s]);
						// FirmA's demands
						total_dmd_A += basic.cplex.getValue(primal.demandvar[0][i][j][t][s]);
					}
				}
				// Demand Loss
				demand_loss = total_dmd_A - total_satisfied_dmd_A;
				outresult << demand_loss << ',';
			}
			outresult << endl;
			// Firm B
			outresult << "B's Region_" << i + 1 << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				outresult << "Time_" << t << ',';
			}
			outresult << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				double total_satisfied_dmd_B = 0; double total_dmd_B = 0; double demand_loss = 0;
				for (int j = 0; j < initial.nb_points; j++) {
					if (j != i && t + basic.period[i][j] <= initial.T) {
						// FirmB's satisfied demands
						total_satisfied_dmd_B += basic.cplex.getValue(primal.flow[1][i][j][t][0][s]);
						// FirmB's demands
						total_dmd_B += basic.cplex.getValue(primal.demandvar[1][i][j][t][s]);
					}
				}
				// Demand Loss
				demand_loss = total_dmd_B - total_satisfied_dmd_B;
				outresult << demand_loss << ',';

			}
			outresult << endl;
			// Total
			outresult << "Total's Region_" << i + 1 << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				outresult << "Time_" << t << ',';
			}
			outresult << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				double total_satisfied_dmd = 0; double total_dmd = 0; double demand_loss = 0;
				for (int j = 0; j < initial.nb_points; j++) {
					if (j != i && t + basic.period[i][j] <= initial.T) {
						// Total firms' satisfied demands
						total_satisfied_dmd += basic.cplex.getValue(primal.flow[0][i][j][t][0][s]) + basic.cplex.getValue(primal.flow[1][i][j][t][0][s]);
						// Two firms' demands
						total_dmd += basic.cplex.getValue(primal.demandvar[0][i][j][t][s]) + basic.cplex.getValue(primal.demandvar[1][i][j][t][s]);
					}
				}
				// Demand Loss
				demand_loss = total_dmd - total_satisfied_dmd;
				outresult << demand_loss << ',';
			}
			outresult << endl << endl;
		}
		outresult << endl;
	}
}

void output_utility_nosharing(initial_parameter initial, basic_parameter basic, primal_variable primal, string root) {
	stringstream filename_output;
	ofstream outresult;

	// ==================== output parameter ====================
	filename_output << root << "/results/utility_alpha_" << basic.alpha[0] << '_' << basic.alpha[1]
		<< "_B_" << initial.budget_A << '_' << initial.budget_B << "_option_" << basic.obj_option << "_sharing_" << basic.sharing_option << "_differ_" << basic.differ_ratio << "_service_" << basic.service << "_cp_" << basic.penalty[0] << "_l_" << initial.demand_level << ".csv";
	outresult.open(filename_output.str().c_str());
	outresult << "Input Parameter" << endl;
	outresult << "Loyal Rate,# Regions,Budget,Procurement Cost,Reposition Cost,Idle Cost,Transfer Cost,Plates,Rental Price" << endl;
	outresult << '(' << basic.alpha[0] << ';' << basic.alpha[1] << ')' << ',' << initial.nb_points << ',' << initial.budget_A << ','
		<< '(' << basic.cost_purchase[0] << ';' << basic.cost_purchase[1] << ')' << ','
		<< '(' << basic.h[0][1] << ';' << basic.h[1][1] << ')' << ','
		<< '(' << basic.h[0][2] << ';' << basic.h[1][2] << ')' << ',' << basic.trans_price[0] << ',' << initial.nb_plates << ',' << basic.rental_price[0] << endl;
	// ==================== output results ====================
	outresult << "Results" << endl;
	// -------- Cost --------
	objvalue(initial, basic, primal);
	outresult << "FirmA's Cost" << ',' << basic.totalcost[0] << ',' << "FirmB's Cost" << ',' << basic.totalcost[1] << ',' << "Total Costs" << ',' << basic.cplex.getObjValue() << endl;
	// -------- Allocation --------
	double sum_park_1 = 0; double sum_park_2 = 0;
	outresult << "Allocation" << endl;
	outresult << "FirmA";
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << ',';
	}
	outresult << ',' << ',' << "FirmB" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << "Region_" << i + 1 << ',';
	}
	outresult << "Total" << ',' << ',';
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << "Region_" << i + 1 << ',';
	}
	outresult << "Total" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << basic.cplex.getValue(primal.park[0][i]) << ',';
		sum_park_1 += basic.cplex.getValue(primal.park[0][i]);
	}
	outresult << sum_park_1 << ',' << ',';
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << basic.cplex.getValue(primal.park[1][i]) << ',';
		sum_park_2 += basic.cplex.getValue(primal.park[1][i]);
	}
	outresult << sum_park_2 << endl;

	// Utility
	outresult << "Utility" << endl;
	for (int s = 0; s < initial.nb_nodes; s++) {
		outresult << "Scenario_" << s + 1 << endl;
		for (int i = 0; i < initial.nb_points; i++) {
			// Firm A
			outresult << "A's Region_" << i + 1 << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				outresult << "Time_" << t << ',';
			}
			outresult << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				// rentals & relocations
				double total_rentals_A = 0; double total_relocations_A = 0; double total_idle_A = 0;
				for (int j = 0; j < initial.nb_points; j++) {
					if (j != i && t + basic.period[i][j] <= initial.T) {
						// FirmA's rentals (satisfied demands)
						total_rentals_A += basic.cplex.getValue(primal.flow[0][i][j][t][0][s]);
						// FirmA's relocations
						total_relocations_A += basic.cplex.getValue(primal.flow[0][i][j][t][1][s]);
					}
				}
				// idles
				total_idle_A = basic.cplex.getValue(primal.flow[0][i][i][t][2][s]);
				// utility
				double utility_A = 0;
				double available_A = total_rentals_A + total_relocations_A + total_idle_A;
				if (available_A > 0) {
					utility_A = total_rentals_A / available_A;
				}
				outresult << utility_A << ',';
			}
			outresult << endl;

			// Firm B
			outresult << "B's Region_" << i + 1 << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				outresult << "Time_" << t << ',';
			}
			outresult << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				// rentals & relocations
				double total_rentals_B = 0; double total_relocations_B = 0; double total_idle_B = 0;
				for (int j = 0; j < initial.nb_points; j++) {
					if (j != i && t + basic.period[i][j] <= initial.T) {
						// FirmB's rentals (satisfied demands)
						total_rentals_B += basic.cplex.getValue(primal.flow[1][i][j][t][0][s]);
						// FirmB's relocations
						total_relocations_B += basic.cplex.getValue(primal.flow[1][i][j][t][1][s]);
					}
				}
				// idles
				total_idle_B = basic.cplex.getValue(primal.flow[1][i][i][t][2][s]);
				// utility
				double utility_B = 0;
				double available_B = total_rentals_B + total_relocations_B + total_idle_B;
				if (available_B > 0) {
					utility_B = total_rentals_B / available_B;
				}
				outresult << utility_B << ',';
			}
			outresult << endl;
			// Total
			outresult << "Total's Region_" << i + 1 << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				outresult << "Time_" << t << ',';
			}
			outresult << endl;
			for (int t = 1; t <= initial.T - 1; t++) {
				// rentals & relocations
				double total_rentals_T = 0; double total_relocations_T = 0; double total_idle_T = 0;
				for (int j = 0; j < initial.nb_points; j++) {
					if (j != i && t + basic.period[i][j] <= initial.T) {
						// Two Firms' rentals (satisfied demands)
						total_rentals_T += basic.cplex.getValue(primal.flow[0][i][j][t][0][s]) + basic.cplex.getValue(primal.flow[1][i][j][t][0][s]);
						// Two Firms' relocations
						total_relocations_T += basic.cplex.getValue(primal.flow[0][i][j][t][1][s]) + basic.cplex.getValue(primal.flow[1][i][j][t][1][s]);
					}
				}
				// idles
				total_idle_T = basic.cplex.getValue(primal.flow[0][i][i][t][2][s]) + basic.cplex.getValue(primal.flow[1][i][i][t][2][s]);
				// utility
				double utility_T = 0;
				double available_T = total_rentals_T + total_relocations_T + total_idle_T;
				if (available_T > 0) {
					utility_T = total_rentals_T / available_T;
				}
				outresult << utility_T << ',';
			}
			outresult << endl << endl;
		}
		outresult << endl;
	}
}

void output_transfer(initial_parameter initial, basic_parameter basic, primal_variable primal, string root) {
	stringstream filename_output;
	ofstream outresult;

	// ==================== output parameter ====================
	filename_output << root << "/results/transfer_alpha_" << basic.alpha[0] << '_' << basic.alpha[1]
		<< "_B_" << initial.budget_A << '_' << initial.budget_B << "_option_" << basic.obj_option << "_differ_" << basic.differ_ratio << "_sharing_" << basic.sharing_option << "_service_" << basic.service << "_cp_" << basic.penalty[0] << "_l_" << initial.demand_level << ".csv";
	outresult.open(filename_output.str().c_str());
	outresult << "Input Parameter" << endl;
	outresult << "Loyal Rate,# Regions,Budget,Procurement Cost,Reposition Cost,Idle Cost,Transfer Cost,Plates,Rental Price" << endl;
	outresult << '(' << basic.alpha[0] << ';' << basic.alpha[1] << ')' << ',' << initial.nb_points << ',' << initial.budget_A << ','
		<< '(' << basic.cost_purchase[0] << ';' << basic.cost_purchase[1] << ')' << ','
		<< '(' << basic.h[0][1] << ';' << basic.h[1][1] << ')' << ','
		<< '(' << basic.h[0][2] << ';' << basic.h[1][2] << ')' << ',' << basic.trans_price[0] << ',' << initial.nb_plates << ',' << basic.rental_price[0] << endl;
	// ==================== output results ====================
	outresult << "Results" << endl;
	// -------- Cost --------
	objvalue(initial, basic, primal);
	outresult << "FirmA's Cost" << ',' << basic.totalcost[0] << ',' << "FirmB's Cost" << ',' << basic.totalcost[1] << ',' << "Objective" << ',' << basic.cplex.getObjValue() << endl;
	// -------- Allocation --------
	double sum_park_1 = 0; double sum_park_2 = 0;
	outresult << "Allocation" << endl;
	outresult << "FirmA";
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << ',';
	}
	outresult << ',' << ',' << "FirmB" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << "Region_" << i + 1 << ',';
	}
	outresult << "Total" << ',' << ',';
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << "Region_" << i + 1 << ',';
	}
	outresult << "Total" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << basic.cplex.getValue(primal.park[0][i]) << ',';
		sum_park_1 += basic.cplex.getValue(primal.park[0][i]);
	}
	outresult << sum_park_1 << ',' << ',';
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << basic.cplex.getValue(primal.park[1][i]) << ',';
		sum_park_2 += basic.cplex.getValue(primal.park[1][i]);
	}
	outresult << sum_park_2 << endl;

	// -------- Transfers --------
	outresult << "Transfers" << endl;
	for (int s = 0; s < initial.nb_nodes; s++) {
		outresult << "Scenario_" << s + 1 << endl;
		// Firm A
		outresult << "Firm A" << endl << ',';
		for (int t = 1; t <= initial.T - 1; t++) {
			outresult << "Time_" << t << ',';
		}
		outresult << endl;
		for (int i = 0; i < initial.nb_points; i++) {
			outresult << "Region_" << i + 1 << ',';
			for (int t = 1; t <= initial.T - 1; t++) {
				outresult << basic.cplex.getValue(primal.flow[0][i][i][t][3][s]) << ',';
			}
			outresult << endl;
		}
		outresult << endl;
		// Firm B
		outresult << "Firm B" << endl << ',';
		for (int t = 1; t <= initial.T - 1; t++) {
			outresult << "Time_" << t << ',';
		}
		outresult << endl;
		for (int i = 0; i < initial.nb_points; i++) {		
			outresult << "Region_" << i + 1 << ',';
			for (int t = 1; t <= initial.T - 1; t++) {
				outresult << basic.cplex.getValue(primal.flow[1][i][i][t][3][s]) << ',';
			}
			outresult << endl;
		}
		outresult << endl << endl;
	}
}

void output_summary(initial_parameter initial, basic_parameter basic, primal_variable primal, string root) {
	stringstream filename_output;
	ofstream outresult;

	// ==================== output parameter ====================
	filename_output << root << "/results/summary_alpha_" << basic.alpha[0] << '_' << basic.alpha[1]
		<< "_B_" << initial.budget_A << '_' << initial.budget_B << "_option_" << basic.obj_option << "_differ_" << basic.differ_ratio << "_sharing_" << basic.sharing_option << "_service_" << basic.service << "_cp_" << basic.penalty[0] << "_l_" << initial.demand_level << ".csv";
	outresult.open(filename_output.str().c_str());
	outresult << "Input Parameter" << endl;
	outresult << "Loyal Rate,# Regions,Budget,Procurement Cost,Reposition Cost,Idle Cost,Transfer Cost,Plates,Rental Price" << endl;
	outresult << '(' << basic.alpha[0] << ';' << basic.alpha[1] << ')' << ',' << initial.nb_points << ',' << initial.budget_A << ','
		<< '(' << basic.cost_purchase[0] << ';' << basic.cost_purchase[1] << ')' << ','
		<< '(' << basic.h[0][1] << ';' << basic.h[1][1] << ')' << ','
		<< '(' << basic.h[0][2] << ';' << basic.h[1][2] << ')' << ',' << basic.trans_price[0] << ',' << initial.nb_plates << ',' << basic.rental_price[0] << endl;
	// ==================== output results ====================
	outresult << "Results" << endl;
	// -------- Cost --------
	objvalue(initial, basic, primal);
	outresult << "FirmA's Cost" << ',' << basic.totalcost[0] << ',' << "FirmB's Cost" << ',' << basic.totalcost[1] << ',' << "Objective" << ',' << basic.cplex.getObjValue() << endl;
	// -------- Allocation --------
	double sum_park_1 = 0; double sum_park_2 = 0;
	outresult << "Allocation" << endl;
	outresult << "FirmA";
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << ',';
	}
	outresult << ',' << ',' << "FirmB" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << "Region_" << i + 1 << ',';
	}
	outresult << "Total" << ',' << ',';
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << "Region_" << i + 1 << ',';
	}
	outresult << "Total" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << basic.cplex.getValue(primal.park[0][i]) << ',';
		sum_park_1 += basic.cplex.getValue(primal.park[0][i]);
	}
	outresult << sum_park_1 << ',' << ',';
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << basic.cplex.getValue(primal.park[1][i]) << ',';
		sum_park_2 += basic.cplex.getValue(primal.park[1][i]);
	}
	outresult << sum_park_2 << endl;

	// -------- demand loss & service level --------
	vector<double> satisfied_dmd_A(initial.nb_nodes); vector<double> satisfied_dmd_B(initial.nb_nodes);
	double total_satisfied_dmd_A = 0; double total_satisfied_dmd_B = 0;
	vector<double> dmd_A(initial.nb_nodes); vector<double> dmd_B(initial.nb_nodes);
	double total_dmd_A = 0; double total_dmd_B = 0; double total_dmd = 0;
	double total_dmd_loss_A = 0; double total_dmd_loss_B = 0; double total_dmd_loss = 0;
	double total_service_level_A = 0; double total_service_level_B = 0; double total_service_level = 0;

	// total satisfied demands
	for (int i = 0; i < initial.nb_points; i++) {
		for (int j = 0; j < initial.nb_points; j++) {
			for (int t = 1; t <= initial.T - 1; t++) {
				for (int s = 0; s < initial.nb_nodes; s++) {
					if (j != i && t + basic.period[i][j] <= initial.T) {
						satisfied_dmd_A.at(s) += basic.cplex.getValue(primal.flow[0][i][j][t][0][s]);
						satisfied_dmd_B.at(s) += basic.cplex.getValue(primal.flow[1][i][j][t][0][s]);
					}
				}
			}
		}
	}

	for (int s = 0; s < initial.nb_nodes; s++) {
		total_satisfied_dmd_A += basic.probTheta.at(s) * satisfied_dmd_A.at(s);
		total_satisfied_dmd_B += basic.probTheta.at(s) * satisfied_dmd_B.at(s);
	}

	//total demands
	for (int i = 0; i < initial.nb_points; i++) {
		for (int ii = 0; ii < initial.nb_points; ii++) {
			for (int t = 1; t <= initial.T - 1; t++) {
				for (int s = 0; s < initial.nb_nodes; s++) {
					if (i != ii && t + basic.period[i][ii] <= initial.T) {
						dmd_A.at(s) += basic.cplex.getValue(primal.demandvar[0][i][ii][t][s]);
						dmd_B.at(s) += basic.cplex.getValue(primal.demandvar[1][i][ii][t][s]);
					}
				}
			}
		}
	}

	for (int s = 0; s < initial.nb_nodes; s++) {
		total_dmd_A += basic.probTheta.at(s) * dmd_A.at(s);
		total_dmd_B += basic.probTheta.at(s) * dmd_B.at(s);
		total_dmd += basic.probTheta.at(s) * (dmd_A.at(s) + dmd_B.at(s));
	}

	// total demand loss
	for (int s = 0; s < initial.nb_nodes; s++) {
		total_dmd_loss_A += basic.probTheta.at(s) * (dmd_A.at(s) - satisfied_dmd_A.at(s));
		total_dmd_loss_B += basic.probTheta.at(s) * (dmd_B.at(s) - satisfied_dmd_B.at(s));
		total_dmd_loss += basic.probTheta.at(s) * (dmd_A.at(s) + dmd_B.at(s) - satisfied_dmd_A.at(s) - satisfied_dmd_B.at(s));
	}

	// total service level
	if (sum_park_1 > 0) {
		for (int s = 0; s < initial.nb_nodes; s++) {
			total_service_level_A += basic.probTheta.at(s) * satisfied_dmd_A.at(s) / dmd_A.at(s) * 100.;
		}
	}
	else {
		total_service_level_A = -1;
	}

	if (sum_park_2 > 0) {
		for (int s = 0; s < initial.nb_nodes; s++) {
			total_service_level_B += basic.probTheta.at(s) * satisfied_dmd_B.at(s) / dmd_B.at(s) * 100.;
		}
	}
	else {
		total_service_level_B = -1;
	}


	if (sum_park_1 + sum_park_2 > 0) {
		for (int s = 0; s < initial.nb_nodes; s++) {
			total_service_level += basic.probTheta.at(s) * (satisfied_dmd_A.at(s) + satisfied_dmd_B.at(s)) / (dmd_A.at(s) + dmd_B.at(s)) * 100.;
		}
	}
	else {
		total_service_level = -1;
	}

	// -------- transfer --------
	
	vector<vector<vector<double> > > transfer(basic.nb_firms, vector<vector<double>>(initial.nb_points, vector<double>(initial.T, 0)));
	
	
	for (int k = 0; k < basic.nb_firms; k++) {
		for (int i = 0; i < initial.nb_points; i++) {
			for (int t = 1; t <= initial.T - 1; t++) {
				for (int s = 0; s < initial.nb_nodes; s++) {
					transfer[k][i][t] += basic.probTheta.at(s) * basic.cplex.getValue(primal.flow[k][i][i][t][3][s]);
				}
			}
		}
	}

	// ratio = transfer / satisfied demands
	vector<double> ratio_A(initial.T+1); vector<double> ratio_B(initial.T+1); vector<double> ratio(initial.T+1);
	double total_ratio_A = 0; double total_ratio_B = 0; double total_ratio = 0;
	vector<double> trans_A_avg(initial.T+1); vector<double> trans_B_avg(initial.T+1); vector<double> trans_avg(initial.T+1);
	double sum_trans_A_avg = 0; double sum_trans_B_avg = 0;

	for (int t = 1; t <= initial.T - 1; t++) {
		for (int s = 0; s < initial.nb_nodes; s++) {
			double sum_trans_A = 0;
			double sum_trans_B = 0;
			for (int i = 0; i < initial.nb_points; i++) {
				sum_trans_A += basic.cplex.getValue(primal.flow[1-0][i][i][t][3][s]);
				sum_trans_B += basic.cplex.getValue(primal.flow[1-1][i][i][t][3][s]);
			}
			double sum_satisfied_dmd_A = 0;
			double sum_satisfied_dmd_B = 0;
			for (int i = 0; i < initial.nb_points; i++) {
				for (int ii = 0; ii < initial.nb_points; ii++) {
					if (i != ii && t + basic.period[i][ii] <= initial.T) {
						sum_satisfied_dmd_A += basic.cplex.getValue(primal.flow[0][i][ii][t][0][s]);
						sum_satisfied_dmd_B += basic.cplex.getValue(primal.flow[1][i][ii][t][0][s]);
					}
				}
			}

			ratio_A.at(t) += basic.probTheta.at(s) * sum_trans_A / sum_satisfied_dmd_A;
			ratio_B.at(t) += basic.probTheta.at(s) * sum_trans_B / sum_satisfied_dmd_B;
			ratio.at(t) += basic.probTheta.at(s) * (sum_trans_A + sum_trans_B) / (sum_satisfied_dmd_A + sum_satisfied_dmd_B);
		}
	}

	for (int s = 0; s < initial.nb_nodes; s++) {
		double sum_trans_A = 0;
		double sum_trans_B = 0;
		for (int t = 1; t <= initial.T - 1; t++) {
			for (int i = 0; i < initial.nb_points; i++) {
				sum_trans_A += basic.cplex.getValue(primal.flow[1-0][i][i][t][3][s]);
				sum_trans_B += basic.cplex.getValue(primal.flow[1-1][i][i][t][3][s]);
			}
		}
		double sum_satisfied_dmd_A = 0;
		double sum_satisfied_dmd_B = 0;
		for (int t = 1; t <= initial.T - 1; t++) {
			for (int i = 0; i < initial.nb_points; i++) {
				for (int ii = 0; ii < initial.nb_points; ii++) {
					if (i != ii && t + basic.period[i][ii] <= initial.T) {
						sum_satisfied_dmd_A += basic.cplex.getValue(primal.flow[0][i][ii][t][0][s]);
						sum_satisfied_dmd_B += basic.cplex.getValue(primal.flow[1][i][ii][t][0][s]);
					}
				}
			}
		}
		total_ratio_A += basic.probTheta.at(s) * sum_trans_A / sum_satisfied_dmd_A;
		total_ratio_B += basic.probTheta.at(s) * sum_trans_B / sum_satisfied_dmd_B;
		total_ratio += basic.probTheta.at(s) * (sum_trans_A+sum_trans_B) / (sum_satisfied_dmd_A + sum_satisfied_dmd_B);
	}


	for (int t = 1; t <= initial.T - 1; t++) {
		for (int s = 0; s < initial.nb_nodes; s++) {
			double sum_trans_A = 0;
			double sum_trans_B = 0;
			for (int i = 0; i < initial.nb_points; i++) {	
				sum_trans_A += basic.cplex.getValue(primal.flow[0][i][i][t][3][s]);
				sum_trans_B += basic.cplex.getValue(primal.flow[1][i][i][t][3][s]);
			}
			trans_A_avg[t] += basic.probTheta.at(s) * sum_trans_A / initial.nb_points;
			trans_B_avg[t] += basic.probTheta.at(s) * sum_trans_B / initial.nb_points;
		}
		sum_trans_A_avg += trans_A_avg[t] / (initial.T - 1);
		sum_trans_B_avg += trans_B_avg[t] / (initial.T - 1);
	}

	outresult << endl << "Total Demand" << endl;
	outresult << "Firm1" << ',' << "Firm2" << ',' << "Total" << endl;
	outresult << total_dmd_A << ',' << total_dmd_B << ',' << total_dmd << endl;

	outresult << endl << "Demand Loss" << endl;
	outresult << "Firm1" << ',' << "Firm2" << ',' << "Total" << endl;
	outresult << total_dmd_loss_A << ',' << total_dmd_loss_B << ',' << total_dmd_loss << endl;

	outresult << endl << "Service Level" << endl;
	outresult << "Firm1" << ',' << "Firm2" << ',' << "Total" << endl;
	outresult << total_service_level_A << ',' << total_service_level_B << ',' << total_service_level << endl;

	outresult << endl << "Transfer Ratio" << endl;
	outresult << "Firm1" << ',' << "Firm2" << ',' << "Total" << endl;
	outresult << total_ratio_A << ',' << total_ratio_B << ',' << total_ratio << endl;

	outresult << endl << "Transfer Ratio in period" << endl << ',';
	for (int t = 1; t <= initial.T - 1; t++) {
		outresult << "T_" << t << ',';
	}
	outresult << endl << "Firm_A" << ',';
	for (int t = 1; t <= initial.T - 1; t++) {
		outresult << ratio_A.at(t) << ',';
	}
	outresult << endl << "Firm_B" << ',';
	for (int t = 1; t <= initial.T - 1; t++) {
		outresult << ratio_B.at(t) << ',';
	}
	outresult << endl << "Total" << ',';
	for (int t = 1; t <= initial.T - 1; t++) {
		outresult << ratio.at(t) << ',';
	}
	outresult << endl;
	outresult << endl << "Transfer_A" << endl << ',';
	for (int t = 1; t <= initial.T - 1; t++) {
		outresult << "T_" << t << ',';
	}
	outresult << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << "R_" << i + 1 << ',';
		for (int t = 1; t <= initial.T - 1; t++) {
			outresult << transfer[0][i][t] << ',';
		}
		outresult << endl;
	}
	outresult << "AVG" << ',';
	for (int t = 1; t <= initial.T - 1; t++) {
		outresult << trans_A_avg[t] << ',';
	}
	outresult << endl << "Transfer_B" << endl << ',';
	for (int t = 1; t <= initial.T - 1; t++) {
		outresult << "T_" << t << ',';
	}
	outresult << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		outresult << "R_" << i + 1 << ',';
		for (int t = 1; t <= initial.T - 1; t++) {
			outresult << transfer[1][i][t] << ',';
		}
		outresult << endl;
	}
	outresult << "AVG" << ',';
	for (int t = 1; t <= initial.T - 1; t++) {
		outresult << trans_B_avg[t] << ',';
	}
	outresult << endl << "Total A" << ',' << "Total B" << ',' << "Total" << endl;
	outresult << sum_trans_A_avg << ',' << sum_trans_B_avg << ',' << (sum_trans_A_avg + sum_trans_B_avg) << endl;
	// relocation
	double relocation_A = 0; double relocation_B = 0; double relocation_total = 0;
	vector<double> relocation_time_A(initial.T + 1, 0); vector<double> relocation_time_B(initial.T + 1, 0); vector<double> relocation_time_total(initial.T + 1, 0);
	vector<vector<double> > relocation_space_A(initial.nb_points, vector<double>(initial.nb_points, 0));
	vector<vector<double> > relocation_space_B(initial.nb_points, vector<double>(initial.nb_points, 0));
	vector<vector<double> > relocation_space_Total(initial.nb_points, vector<double>(initial.nb_points, 0));
	for (int t = 1; t < initial.T; t++) {
		for (int s = 0; s < initial.nb_nodes; s++) {
			for (int i = 0; i < initial.nb_points; i++) {
				for (int j = 0; j < initial.nb_points; j++) {
					if (j != i && t + basic.period[i][j] <= initial.T) {
						relocation_time_A.at(t) += basic.probTheta.at(s) * basic.cplex.getValue(primal.flow[0][i][j][t][1][s]);
						relocation_time_B.at(t) += basic.probTheta.at(s) * basic.cplex.getValue(primal.flow[1][i][j][t][1][s]);
						relocation_time_total.at(t) += basic.probTheta.at(s) * (basic.cplex.getValue(primal.flow[0][i][j][t][1][s]) + basic.cplex.getValue(primal.flow[1][i][j][t][1][s]));
					}
				}
			}
		}
	}
	for (int t = 1; t < initial.T; t++) {
		relocation_A += relocation_time_A.at(t);
		relocation_B += relocation_time_B.at(t);
		relocation_total += relocation_time_total.at(t);
	}
	for (int i = 0; i < initial.nb_points; i++) {
		for (int j = 0; j < initial.nb_points; j++) {
			for (int t = 1; t < initial.T; t++) {
				for (int s = 0; s < initial.nb_nodes; s++) {
					if (j != i && t + basic.period[i][j] <= initial.T) {
						relocation_space_A.at(i).at(j) += basic.probTheta.at(s) / (initial.T - 1) * basic.cplex.getValue(primal.flow[0][i][j][t][1][s]);
						relocation_space_B.at(i).at(j) += basic.probTheta.at(s) / (initial.T - 1) * basic.cplex.getValue(primal.flow[1][i][j][t][1][s]);
						relocation_space_Total.at(i).at(j) += basic.probTheta.at(s) / (initial.T - 1) * (basic.cplex.getValue(primal.flow[0][i][j][t][1][s]) + basic.cplex.getValue(primal.flow[1][i][j][t][1][s]));
					}
				}
			}
		}
	}

	/*if (basic.sharing_option == 1) {
		ratio_A = total_trans_A / total_satisfied_dmd_A;
		ratio_B = total_trans_B / total_satisfied_dmd_B;
		ratio_T = (total_trans_A + total_trans_B) / (total_satisfied_dmd_A + total_satisfied_dmd_B);

		outresult << endl << "Ratio_A" << ',' << "Ration_B" << ',' << "Ratio_T" << endl;
		outresult << ratio_A << ',' << ratio_B << ',' << ratio_T;
	}*/

	outresult << endl << "Relocation" << endl;
	outresult << "Firm1" << ',' << "Firm2" << ',' << "Total" << endl;
	outresult << relocation_A << ',' << relocation_B << ',' << relocation_total << endl;

	outresult << endl << "Firm A" << endl;
	for (int t = 1; t < initial.T; t++) {
		outresult << "T_" << t << ',';
	}
	outresult << endl;
	for (int t = 1; t < initial.T; t++) {
		outresult << relocation_time_A.at(t) << ',';
	}
	outresult << endl << "Firm B" << endl;
	for (int t = 1; t < initial.T; t++) {
		outresult << "T_" << t << ',';
	}
	outresult << endl;
	for (int t = 1; t < initial.T; t++) {
		outresult << relocation_time_B.at(t) << ',';
	}
	outresult << endl << "Total" << endl;
	for (int t = 1; t < initial.T; t++) {
		outresult << "T_" << t << ',';
	}
	outresult << endl;
	for (int t = 1; t < initial.T; t++) {
		outresult << relocation_time_total.at(t) << ',';
	}
	outresult << endl << endl << "Firm A" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		for (int j = 0; j < initial.nb_points; j++) {
			outresult << relocation_space_A.at(i).at(j) << ',';
		}
		outresult << endl;
	}
	outresult << "Firm B" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		for (int j = 0; j < initial.nb_points; j++) {
			outresult << relocation_space_B.at(i).at(j) << ',';
		}
		outresult << endl;
	}
	outresult << "Total" << endl;
	for (int i = 0; i < initial.nb_points; i++) {
		for (int j = 0; j < initial.nb_points; j++) {
			outresult << relocation_space_Total.at(i).at(j) << ',';
		}
		outresult << endl;
	}

}