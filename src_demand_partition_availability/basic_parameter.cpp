#include "basic_parameter.h"

initial_parameter::initial_parameter(){};
initial_parameter::~initial_parameter(){};

basic_parameter::basic_parameter(int T, int nb_points, int nb_nodes, int nb_plates, int budget_A, int budget_B, double demand_level)
{

	nb_firms = 2;
	nb_arcs = 4;
	interval = 4;
	start_hour = 0;
	M = 9;
	//========================= Input Parameter =========================
	budget.resize(nb_firms);
	h.resize(nb_firms);
	alpha.resize(nb_firms);
	probTheta.resize(nb_nodes);
	cost_purchase.resize(nb_firms);
	penalty.resize(nb_firms);
	rental_price.resize(nb_firms);
	trans_price.resize(nb_firms);
	totallots.resize(nb_points);
	demand_value.resize(4350);
	period.resize(nb_points);
	proportion.resize(T+1);
	demand_region.resize(145);

	for (int k = 0; k < nb_firms; k++) {
		h[k].resize(nb_arcs);
	}

	for (int i = 0; i < 4350; i++) {
		demand_value[i].resize(4);
	}

	for (int i = 0; i < nb_points; i++){
		period[i].resize(nb_points);
	}

	for (int t = 0; t <= T; t++) {
		proportion.at(t).resize(nb_points);
		for (int i = 0; i < nb_points; i++) {
			proportion.at(t).at(i).resize(nb_points);
		}
	}

	for (int t = 0; t <= 144; t++) {
		demand_region.at(t).resize(nb_points);
		for (int i = 0; i < nb_points; i++) {
			demand_region.at(t).at(i).resize(nb_nodes);
		}
	}

	time_multiplier.resize(25);

	if (nb_nodes == 3) {
		probTheta[0] = 0.345; probTheta[1] = 0.319; probTheta[2] = 0.336;
	}
	else {
		for (int i = 0; i < nb_nodes; i++) {
			probTheta[i] = 1. / nb_nodes;
		}
	}

	Y1.resize(nb_points);
	base_demand.resize(nb_points);
	for (int i = 0; i < nb_points; i++){
		Y1[i].resize(nb_points);
		base_demand[i].resize(nb_points);
		for (int ii = 0; ii < nb_points; ii++){
			Y1[i][ii].resize(T + 1);
			base_demand[i][ii].resize(T + 1);
			for (int t = 0; t <= T; t++){
				Y1[i][ii][t].resize(T + 1);
				for (int tt = 0; tt <= T; tt++){
					Y1[i][ii][t][tt].resize(nb_nodes);
				}
			}
		}
	}

	Y.resize(nb_firms);
	for (int k = 0; k < nb_firms; k++){
		Y[k].resize(nb_points);
		for (int i = 0; i < nb_points; i++){
			Y[k][i].resize(nb_points);
			for (int ii = 0; ii < nb_points; ii++){
				Y[k][i][ii].resize(T + 1);
				for (int t = 0; t <= T; t++){
					Y[k][i][ii][t].resize(T + 1);
					for (int tt = 0; tt <= T; tt++){
						Y[k][i][ii][t][tt].resize(nb_nodes);
					}
				}
			}
		}
	}

	//========================= Assign Values =========================
	budget[0] = budget_A; budget[1] = budget_B;

	vector<double> ratio(nb_points);
	if (nb_points == 6) {
		ratio.at(0) = 0.14; ratio.at(1) = 0.19; ratio.at(2) = 0.22; ratio.at(3) = 0.17; ratio.at(4) = 0.16; ratio.at(5) = 0.12;
	}
	if (nb_points == 2) {
		// ratio.at(0) = 0.48; ratio.at(1) = 0.52; this is right
		ratio.at(0) = 0.52; ratio.at(1) = 0.48;
	}

	int sum_temp = 0;
	for (int i = 0; i < nb_points-1; i++){
		totallots[i] = static_cast<int>(nb_plates * nb_points * demand_level * ratio.at(i));
		sum_temp += totallots[i];
	}

	totallots.at(nb_points - 1) = nb_plates * nb_points * demand_level - sum_temp;

	//======================================== Value of firms

	x_val_A.resize(nb_points); x_val_B.resize(nb_points); x_val_B_old.resize(nb_points);

	flow_val_A.resize(nb_points);
	flow_val_B.resize(nb_points);
	flow_val_B_old.resize(nb_points);
	for (int j = 0; j < nb_points; j++) {
		flow_val_A[j].resize(nb_points);
		flow_val_B[j].resize(nb_points);
		flow_val_B_old[j].resize(nb_points);
		for (int k = 0; k < nb_points; k++) {
			flow_val_A[j][k].resize(T + 1);
			flow_val_B[j][k].resize(T + 1);
			flow_val_B_old[j][k].resize(T + 1);
			for (int s = 0; s <= T; s++) {
				flow_val_A[j][k][s].resize(nb_arcs);
				flow_val_B[j][k][s].resize(nb_arcs);
				flow_val_B_old[j][k][s].resize(nb_arcs);
				for (int a = 0; a < nb_arcs; a++) {
					flow_val_A[j][k][s][a].resize(nb_nodes);
					flow_val_B[j][k][s][a].resize(nb_nodes);
					flow_val_B_old[j][k][s][a].resize(nb_nodes);
					for (int n = 0; n < nb_nodes; n++) {
						if (a < 3) {
							flow_val_B[j][k][s][a][n] = 0;
							flow_val_A[j][k][s][a][n] = 0;
						}
					}
				}
			}
		}
	}

	re_val_A.resize(nb_points);
	re_val_B.resize(nb_points);
	re_val_B_old.resize(nb_points);
	for (int i = 0; i < nb_points; i++) {
		re_val_A[i].resize(T + 1);
		re_val_B[i].resize(T + 1);
		re_val_B_old[i].resize(T + 1);
		for (int t = 0; t <= T; t++) {
			re_val_A[i][t].resize(3);
			re_val_B[i][t].resize(3);
			re_val_B_old[i][t].resize(3);
			for (int d = 0; d < 3; d++) {
				re_val_A[i][t][d].resize(nb_nodes);
				re_val_B[i][t][d].resize(nb_nodes);
				re_val_B_old[i][t][d].resize(nb_nodes);
			}
		}
	}

	dmd_val_A.resize(nb_points);
	dmd_val_B.resize(nb_points);
	for (int i = 0; i < nb_points; i++) {
		dmd_val_A[i].resize(nb_points);
		dmd_val_B[i].resize(nb_points);
		for (int j = 0; j < nb_points; j++) {
			dmd_val_A[i][j].resize(T + 1);
			dmd_val_B[i][j].resize(T + 1);
			for (int s = 0; s <= T; s++) {
				dmd_val_A[i][j][s].resize(nb_nodes);
				dmd_val_B[i][j][s].resize(nb_nodes);
			}
		}
	}

	//========================= Value for output =========================

	totalcost.resize(nb_firms);
	time = 0;

	//========================= Problem =========================
	model = IloModel(env);
	model_final = IloModel(env);
	model_independent = IloModel(env);
	cplex = IloCplex(env);
	myobj = IloObjective(env);
	mycons = IloRangeArray(env);
};
basic_parameter::~basic_parameter(){};