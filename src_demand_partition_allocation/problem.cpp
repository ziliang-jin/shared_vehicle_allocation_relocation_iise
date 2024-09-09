#include "problem.h"
#include <vector>
#include <math.h>

void create_primal_problem(initial_parameter initial, basic_parameter& basic, primal_variable primal) {

	//========================== First Stage ==============================
	
	//----------- Budget Cons ----------
	IloRangeArray budgetcons(basic.env);

	for (int i = 0; i < basic.nb_firms; i++){
		IloExpr totalallocationexpr(basic.env);
		for (int j = 0; j < initial.nb_points; j++){
			totalallocationexpr += primal.park[i][j];
		}
		budgetcons.add(totalallocationexpr - basic.budget[i] <= 0);
		totalallocationexpr.end();
	}

	basic.model.add(budgetcons);
	basic.model_final.add(budgetcons);
	budgetcons.setNames("budgetcons");
	budgetcons.end();
	//----------- Allocation Cons -----------
	IloRangeArray allocationcons(basic.env);

	for (int j = 0; j < initial.nb_points; j++){
		allocationcons.add(primal.park[0][j] + primal.park[1][j] <= basic.totallots[j]);
	}

	basic.model.add(allocationcons);
	basic.model_final.add(allocationcons);
	allocationcons.setNames("allocationcons");
	allocationcons.end();

	//========================== Second Stage ==========================

	//----------- Demand Cons -----------
	double sum_total_allocation = 0;
	IloExpr sum_allocationvar_firm1(basic.env);
	IloExpr sum_allocationvar_firm2(basic.env);
	for (int i = 0; i < initial.nb_points; i++){
		sum_total_allocation += basic.totallots[i];
		sum_allocationvar_firm1 += primal.park[0][i];
		sum_allocationvar_firm2 += primal.park[1][i];
	}
	IloRangeArray demandcons(basic.env);

	for (int k = 0; k < basic.nb_firms; k++){
		for (int i = 0; i < initial.nb_points; i++){
			for (int ii = 0; ii < initial.nb_points; ii++){
				for (int t = 1; t <= initial.T; t++){
					for (int s = 0; s < initial.nb_nodes; s++){
						if (i != ii && t + basic.period[i][ii] <= initial.T){
							if (k == 0){
								demandcons.add(primal.demandvar[k][i][ii][t][s] - basic.Y1[i][ii][t][t + basic.period[i][ii]][s] * sum_allocationvar_firm1 / sum_total_allocation
									- basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] == 0);
							}
							else{
								demandcons.add(primal.demandvar[k][i][ii][t][s] - basic.Y1[i][ii][t][t + basic.period[i][ii]][s] * sum_allocationvar_firm2 / sum_total_allocation
									- basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] == 0);
							}
						}
					}
				}
			}
		}
	}

	sum_allocationvar_firm1.end();
	sum_allocationvar_firm2.end();

	basic.model.add(demandcons);
	basic.model_final.add(demandcons);
	demandcons.setNames("demandcons");
	demandcons.end();
	//----------- Node Balance Cons -----------
	IloRangeArray flowbalancecons(basic.env);

	// s=[1,T-1]
	for (int i = 0; i < basic.nb_firms; i++){
		for (int n = 0; n < initial.nb_nodes; n++){
			for (int k = 0; k < initial.nb_points; k++){
				for (int s = 1; s < initial.T; s++){
					IloExpr sumin(basic.env);
					IloExpr sumout(basic.env);
					// sumin
					// rental & relocation
					for (int j = 0; j < initial.nb_points; j++){
						if (j != k && basic.period[j][k] < s){
							sumin += primal.flow[i][j][k][s - basic.period[j][k]][0][n] + primal.flow[i][j][k][s - basic.period[j][k]][1][n];
						}
					}
					// idle & transfer
					sumin += primal.flow[i][k][k][s - 1][2][n] + primal.flow[1 - i][k][k][s][3][n];
					// return
					for (int d = 1; d <= 2; d++){
						if (s - d >= 1){
							sumin += primal.re[1 - i][k][s - d][d][n];
						}
					}
					// sumout
					// rental & relocation
					for (int j = 0; j < initial.nb_points; j++){
						if (j != k){
							sumout += primal.flow[i][k][j][s][0][n] + primal.flow[i][k][j][s][1][n];
						}
					}
					// idle & transfer
					sumout += primal.flow[i][k][k][s][2][n] + primal.flow[i][k][k][s][3][n];
					// return
					for (int d = 1; d <= 2; d++){
						if (s - d >= 1){
							sumout += primal.re[i][k][s - d][d][n];
						}
					}

					flowbalancecons.add(sumin - sumout == 0);

					sumin.end();
					sumout.end();
				}
			}
		}
	}
	// s=0
	for (int i = 0; i < basic.nb_firms; i++){
		for (int n = 0; n < initial.nb_nodes; n++){
			for (int j = 0; j < initial.nb_points; j++){
				flowbalancecons.add(primal.flow[i][j][j][0][2][n] - primal.park[i][j] == 0);
			}
		}
	}
	// s=T
	for (int i = 0; i < basic.nb_firms; i++){
		for (int n = 0; n < initial.nb_nodes; n++){
			for (int k = 0; k < initial.nb_points; k++){
				IloExpr suminT(basic.env);
				IloExpr sumoutT(basic.env);
				// suminT
				// rental & relocation
				for (int j = 0; j < initial.nb_points; j++){
					if (j != k && basic.period[j][k] < initial.T){
						suminT += primal.flow[i][j][k][initial.T - basic.period[j][k]][0][n] + primal.flow[i][j][k][initial.T - basic.period[j][k]][1][n];
					}
				}
				// idle
				suminT += primal.flow[i][k][k][initial.T - 1][2][n];
				// return
				for (int d = 1; d <= 2; d++){
					if (initial.T - d >= 1){
						suminT += primal.re[1 - i][k][initial.T - d][d][n];
					}
				}
				// sumoutT
				// return
				for (int d = 1; d <= 2; d++){
					if (initial.T - d >= 1){
						sumoutT += primal.re[i][k][initial.T - d][d][n];
					}
				}

				flowbalancecons.add(suminT - sumoutT - primal.park[i][k] == 0);

				suminT.end();
				sumoutT.end();
			}
		}
	}

	basic.model.add(flowbalancecons);
	basic.model_final.add(flowbalancecons);
	flowbalancecons.setNames("flowbalancecons");
	flowbalancecons.end();
	//----------- # of rental bikes constraints -----------
	IloRangeArray rentalcapacitycons(basic.env);

	for (int i = 0; i < basic.nb_firms; i++){
		for (int j = 0; j < initial.nb_points; j++){
			for (int k = 0; k < initial.nb_points; k++){
				for (int s = 1; s < initial.T; s++){
					for (int n = 0; n < initial.nb_nodes; n++){
						if (k != j && s + basic.period[j][k] <= initial.T){
							rentalcapacitycons.add(primal.flow[i][j][k][s][0][n] - primal.demandvar[i][j][k][s][n] <= 0);
						}
					}
				}
			}
		}
	}

	basic.model.add(rentalcapacitycons);
	basic.model_final.add(rentalcapacitycons);
	rentalcapacitycons.setNames("rentalcapacitycons");
	rentalcapacitycons.end();
	//----------- inventory level constraints -----------
	IloRangeArray inventorycapacitycons(basic.env);

	for (int i = 0; i < basic.nb_firms; i++){
		for (int j = 0; j < initial.nb_points; j++){
			for (int s = 1; s <= initial.T; s++){
				for (int n = 0; n < initial.nb_nodes; n++){
					inventorycapacitycons.add(primal.flow[i][j][j][s - 1][2][n] - primal.park[i][j] <= 0);
				}
			}
		}
	}

	inventorycapacitycons.setNames("inventorycapacitycons");
	basic.model.add(inventorycapacitycons);
	basic.model_final.add(inventorycapacitycons);
	inventorycapacitycons.end();

	//----------- Return Cons -----------
	if (basic.sharing_option == 1) {
		IloRangeArray returncons(basic.env);

		for (int k = 0; k < basic.nb_firms; k++) {
			for (int t = 1; t <= initial.T - 1; t++) {
				for (int s = 0; s < initial.nb_nodes; s++) {
					IloExpr sumtransin(basic.env);
					for (int i = 0; i < initial.nb_points; i++) {
						sumtransin += primal.flow[1 - k][i][i][t][3][s];
					}
					IloExpr sumreturn(basic.env);
					for (int i = 0; i < initial.nb_points; i++) {
						for (int d = 1; d <= std::min(2, initial.T - t); d++) {
							sumreturn += primal.re[k][i][t][d][s];
						}
					}

					returncons.add(sumtransin - sumreturn == 0);
					sumtransin.end();
					sumreturn.end();
				}
			}
		}

		returncons.setNames("returncons");
		basic.model.add(returncons);
		basic.model_final.add(returncons);
		returncons.end();
	}

	//----------- Additional Cons -----------
	IloRangeArray additionalcons(basic.env);

	if (basic.sharing_option == 1) {
		// transfer
		for (int s = 0; s < initial.nb_nodes; s++) {
			for (int t = 1; t <= initial.T - 1; t++) {
				for (int i = 0; i < initial.nb_points; i++) {
					IloExpr sum_demand_A(basic.env);
					IloExpr sum_demand_B(basic.env);
					for (int ii = 0; ii < initial.nb_points; ii++) {
						if (i != ii && t + basic.period[i][ii] <= initial.T) {
							sum_demand_A += primal.demandvar[0][i][ii][t][s];
							sum_demand_B += primal.demandvar[1][i][ii][t][s];
						}
					}
					additionalcons.add(primal.flow[1 - 0][i][i][t][3][s] - sum_demand_A <= 0);
					additionalcons.add(primal.flow[1 - 1][i][i][t][3][s] - sum_demand_B <= 0);

					sum_demand_A.end();
					sum_demand_B.end();
				}
			}
		}

		// no transfer simultaneously
		for (int i = 0; i < initial.nb_points; i++) {
			for (int t = 1; t <= initial.T - 1; t++) {
				for (int s = 0; s < initial.nb_nodes; s++) {
					additionalcons.add(primal.flow[0][i][i][t][3][s] + basic.totallots[i] * primal.z_trans[i][t][s] >= 0);
					additionalcons.add(primal.flow[0][i][i][t][3][s] - basic.totallots[i] * primal.z_trans[i][t][s] <= 0);
					additionalcons.add(primal.flow[1][i][i][t][3][s] + basic.totallots[i] * (1 - primal.z_trans[i][t][s]) >= 0);
					additionalcons.add(primal.flow[1][i][i][t][3][s] - basic.totallots[i] * (1 - primal.z_trans[i][t][s]) <= 0);
				}
			}
		}
		//
		for (int k = 0; k < basic.nb_firms; k++) {
			IloExpr sum_allo(basic.env);
			for (int i = 0; i < initial.nb_points; i++) {
				sum_allo += primal.park[k][i];
			}
			for (int t = 1; t <= initial.T - 1; t++) {
				for (int s = 0; s < initial.nb_nodes; s++) {
					IloExpr sum_trans(basic.env);
					for (int i = 0; i < initial.nb_points; i++) {
						sum_trans += primal.flow[k][i][i][t][3][s];
					}
					additionalcons.add(sum_trans - sum_allo <= 0);
					sum_trans.end();
				}
			}
			sum_allo.end();
		}
	}
	else {
		for (int k = 0; k < basic.nb_firms; k++) {
			for (int i = 0; i < initial.nb_points; i++) {
				for (int t = 1; t <= initial.T - 1; t++) {
					for (int s = 0; s < initial.nb_nodes; s++) {
						additionalcons.add(primal.flow[k][i][i][t][3][s] == 0);
					}
				}
			}
		}
	}

	if (basic.obj_option == 2 || basic.obj_option == 4) {
		// total satisfied demands
		IloExpr satisfied_dmd_A(basic.env);
		IloExpr satisfied_dmd_B(basic.env);
		for (int i = 0; i < initial.nb_points; i++) {
			for (int j = 0; j < initial.nb_points; j++) {
				for (int t = 1; t <= initial.T - 1; t++) {
					for (int s = 0; s < initial.nb_nodes; s++) {
						if (j != i && t + basic.period[i][j] <= initial.T) {
							satisfied_dmd_A += primal.flow[0][i][j][t][0][s];
							satisfied_dmd_B += primal.flow[1][i][j][t][0][s];
						}
					}
				}
			}
		}

		//total demands
		IloExpr dmd_A(basic.env);
		IloExpr dmd_B(basic.env);
		for (int i = 0; i < initial.nb_points; i++) {
			for (int ii = 0; ii < initial.nb_points; ii++) {
				for (int t = 1; t <= initial.T; t++) {
					for (int s = 0; s < initial.nb_nodes; s++) {
						if (i != ii && t + basic.period[i][ii] <= initial.T) {
							dmd_A += primal.demandvar[0][i][ii][t][s];
							dmd_B += primal.demandvar[1][i][ii][t][s];
						}
					}
				}
			}
		}

		additionalcons.add(satisfied_dmd_A - basic.service * dmd_A >= 0);
		additionalcons.add(satisfied_dmd_B - basic.service * dmd_B >= 0);
		// additionalcons.add((satisfied_dmd_A + satisfied_dmd_B) - basic.service * (dmd_A + dmd_B) >= 0);
		satisfied_dmd_A.end();
		satisfied_dmd_B.end();
		dmd_A.end();
		dmd_B.end();

	}

	basic.model.add(additionalcons);
	basic.model_final.add(additionalcons);
	additionalcons.setNames("additionalcons");
	additionalcons.end();

} // the end of void

void create_station_problem(initial_parameter initial, basic_parameter& basic, dual_variable dual){

	//========================== Stationarity of Allocation Variables ==========================
	IloRangeArray allocation_station_cons(basic.env);

	double sum_total_allocation = 0;
	for (int i = 0; i < initial.nb_points; i++){
		sum_total_allocation += basic.totallots[i];
	}

	for (int k = 0; k < basic.nb_firms; k++){
		for (int i = 0; i < initial.nb_points; i++){
			// summation of rho
			IloExpr sum_rho(basic.env);
			for (int i = 0; i < initial.nb_points; i++) {
				for (int ii = 0; ii < initial.nb_points; ii++) {
					for (int t = 1; t < initial.T; t++) {
						if (i != ii && t + basic.period[i][ii] <= initial.T) {
							for (int s = 0; s < initial.nb_nodes; s++) {
								sum_rho += (basic.Y1[i][ii][t][t + basic.period[i][ii]][s] / sum_total_allocation) * dual.rho[k][i][ii][t][t + basic.period[i][ii]][s];
							}
						}
					}
				}
			}
			// summation of gamma
			IloExpr sum_gamma0(basic.env);
			IloExpr sum_gammaT(basic.env);
			for (int s = 0; s < initial.nb_nodes; s++){
				sum_gamma0 += dual.gamma[k][i][0][s];
				sum_gammaT += dual.gamma[k][i][initial.T][s];
			}
			// summation of etaI
			IloExpr sum_etaI(basic.env);
			for (int t = 0; t <= initial.T - 1; t++){
				for (int s = 0; s < initial.nb_nodes; s++){
					sum_etaI += dual.etaI[k][i][t][s];
				}
			}

			allocation_station_cons.add(-basic.cost_purchase[k] - dual.tau[k] - dual.beta[k][i] + sum_rho
				 + sum_gamma0 - sum_gammaT + sum_etaI - dual.kappa[k][i] == 0);

			sum_rho.end();
			sum_gamma0.end();
			sum_gammaT.end();
			sum_etaI.end();
		}
	}
	
	basic.model.add(allocation_station_cons);
	allocation_station_cons.setNames("allocation_station_cons");
	allocation_station_cons.end();
	//========================== Stationarity of Demand Variables ==========================
	IloRangeArray demand_station_cons(basic.env);
	
	for (int k = 0; k < basic.nb_firms; k++){
		for (int i = 0; i < initial.nb_points; i++){
			for (int ii = 0; ii < initial.nb_points; ii++){
				for (int t = 1; t < initial.T; t++){
					if (i != ii && t + basic.period[i][ii] <= initial.T){
						for (int s = 0; s < initial.nb_nodes; s++){
							demand_station_cons.add(-basic.probTheta[s] * basic.penalty[k] - dual.rho[k][i][ii][t][t + basic.period[i][ii]][s]
								+ dual.etaR[k][i][ii][t][t + basic.period[i][ii]][s] == 0);
						}
					}
				}
			}
		}
	}

	basic.model.add(demand_station_cons);
	demand_station_cons.setNames("demand_station_cons");
	demand_station_cons.end();
	//========================== Stationarity of Rental Variables ==========================
	IloRangeArray rental_station_cons(basic.env);
	
	for (int k = 0; k < basic.nb_firms; k++){
		for (int i = 0; i < initial.nb_points; i++){
			for (int ii = 0; ii < initial.nb_points; ii++){
				for (int t = 1; t < initial.T; t++){
					if (i != ii && t + basic.period[i][ii] <= initial.T){
						for (int s = 0; s < initial.nb_nodes; s++){
							rental_station_cons.add(basic.probTheta[s] * basic.rental_price[k] + basic.probTheta[s] * basic.penalty[k]
								- dual.gamma[k][i][t][s] + dual.gamma[k][ii][t + basic.period[i][ii]][s]
								- dual.etaR[k][i][ii][t][t + basic.period[i][ii]][s] - dual.dwR[k][i][ii][t][t + basic.period[i][ii]][s] == 0);						
						}
					}
				}
			}
		}
	}

	basic.model.add(rental_station_cons);
	rental_station_cons.setNames("rental_station_cons");
	rental_station_cons.end();
	//========================== Stationarity of Relocation Variables ==========================
	
	IloRangeArray relocation_station_cons(basic.env);
	
	for (int k = 0; k < basic.nb_firms; k++){
		for (int i = 0; i < initial.nb_points; i++){
			for (int ii = 0; ii < initial.nb_points; ii++){
				for (int t = 1; t < initial.T; t++){
					if (i != ii && t + basic.period[i][ii] <= initial.T){
						for (int s = 0; s < initial.nb_nodes; s++){
							relocation_station_cons.add(-basic.probTheta[s] * basic.h[k][1] - dual.gamma[k][i][t][s] + dual.gamma[k][ii][t + basic.period[i][ii]][s]
								- dual.dwL[k][i][ii][t][t + basic.period[i][ii]][s] == 0);
							
						}
					}
				}
			}
		}
	}

	basic.model.add(relocation_station_cons);
	relocation_station_cons.setNames("relocation_station_cons");
	relocation_station_cons.end();

	//========================== Stationarity of Idle Variables ==========================
	IloRangeArray idle_station_cons(basic.env);
	
	for (int k = 0; k < basic.nb_firms; k++){
		for (int i = 0; i < initial.nb_points; i++){
			for (int t = 0; t <= initial.T - 1; t++){
				for (int s = 0; s < initial.nb_nodes; s++){
					idle_station_cons.add(-basic.probTheta[s] * basic.h[k][2] - dual.gamma[k][i][t][s] + dual.gamma[k][i][t+1][s]
						- dual.etaI[k][i][t][s] - dual.dwI[k][i][t][s] == 0);
				}
			}
		}
	}

	basic.model.add(idle_station_cons);
	idle_station_cons.setNames("idle_station_cons");
	idle_station_cons.end();
	//========================== Stationarity of Transfer Variables ==========================
	if (basic.sharing_option == 1) {
		IloRangeArray transfer_station_cons(basic.env);

		for (int k = 0; k < basic.nb_firms; k++) {
			for (int i = 0; i < initial.nb_points; i++) {
				for (int t = 1; t <= initial.T - 1; t++) {
					for (int s = 0; s < initial.nb_nodes; s++) {
						transfer_station_cons.add(basic.probTheta[s] * basic.trans_price[k] - dual.gamma[k][i][t][s] - dual.dwT[k][i][t][s] == 0);
					}
				}
			}
		}

		basic.model.add(transfer_station_cons);
		transfer_station_cons.setNames("transfer_station_cons");
		transfer_station_cons.end();
	}
	//========================== Stationarity of Return Variables ==========================
	if (basic.sharing_option == 1) {
		IloRangeArray return_station_cons(basic.env);
		for (int k = 0; k < basic.nb_firms; k++) {
			for (int i = 0; i < initial.nb_points; i++) {
				for (int t = 1; t <= initial.T - 1; t++) {
					for (int d = 1; d <= std::min(2, initial.T - t); d++) {
						for (int s = 0; s < initial.nb_nodes; s++) {
							return_station_cons.add(dual.gamma[k][i][t + d][s] - dual.y[k][t][s] + dual.dre[k][i][t][d][s] == 0);
						}
					}
				}
			}
		}

		basic.model.add(return_station_cons);
		return_station_cons.setNames("return_station_cons");
		return_station_cons.end();
	}
} // the end of void

void create_comp_problem(initial_parameter initial, basic_parameter& basic, primal_variable primal, dual_variable dual){
	//========================== Budget Complementary Cons ==========================
	IloRangeArray budget_comp_cons(basic.env);
	
	double bigm = 10000;

	
	for (int k = 0; k < basic.nb_firms; k++){
		IloExpr sum_allocation(basic.env);
		for (int i = 0; i < initial.nb_points; i++){
			sum_allocation += primal.park[k][i];
		}
		budget_comp_cons.add(sum_allocation - basic.budget[k] + bigm * dual.z_tau[k] >= 0);
		budget_comp_cons.add(sum_allocation - basic.budget[k] - bigm * dual.z_tau[k] <= 0);
		budget_comp_cons.add(dual.tau[k] + bigm * (1 - dual.z_tau[k]) >= 0);
		budget_comp_cons.add(dual.tau[k] - bigm * (1 - dual.z_tau[k]) <= 0);

		sum_allocation.end();
	}
	/*
	for (int k = 0; k < basic.nb_firms; k++) {
		budget_comp_cons.add(dual.tau[k] == 0);
	}
	*/
	basic.model.add(budget_comp_cons);
	budget_comp_cons.setNames("budget_comp_cons");
	budget_comp_cons.end();
	//========================== Allocation Complementary Cons ==========================
	IloRangeArray allocation_comp_cons(basic.env);
	
	for (int k = 0; k < basic.nb_firms; k++){
		for (int i = 0; i < initial.nb_points; i++){
			allocation_comp_cons.add(primal.park[k][i] + primal.park[1 - k][i] - basic.totallots[i] + bigm * dual.z_beta[k][i] >= 0);
			allocation_comp_cons.add(primal.park[k][i] + primal.park[1 - k][i] - basic.totallots[i] - bigm * dual.z_beta[k][i] <= 0);
			allocation_comp_cons.add(dual.beta[k][i] + bigm * (1 - dual.z_beta[k][i]) >= 0);
			allocation_comp_cons.add(dual.beta[k][i] - bigm * (1 - dual.z_beta[k][i]) <= 0);
		}
	}

	basic.model.add(allocation_comp_cons);
	allocation_comp_cons.setNames("allocation_comp_cons");
	allocation_comp_cons.end();
	//========================== Rental Capacity Complementary Cons ==========================
	IloRangeArray rentalcap_comp_cons(basic.env);

	for (int k = 0; k < basic.nb_firms; k++){
		for (int i = 0; i < initial.nb_points; i++){
			for (int ii = 0; ii < initial.nb_points; ii++){
				for (int t = 1; t < initial.T; t++){
					if (t + basic.period[i][ii] <= initial.T){
						for (int s = 0; s < initial.nb_nodes; s++){
							rentalcap_comp_cons.add(primal.flow[k][i][ii][t][0][s] - primal.demandvar[k][i][ii][t][s]
								+ bigm * dual.z_etaR[k][i][ii][t][t + basic.period[i][ii]][s] >= 0);
							rentalcap_comp_cons.add(primal.flow[k][i][ii][t][0][s] - primal.demandvar[k][i][ii][t][s]
								- bigm * dual.z_etaR[k][i][ii][t][t + basic.period[i][ii]][s] <= 0);
							rentalcap_comp_cons.add(dual.etaR[k][i][ii][t][t + basic.period[i][ii]][s]
								+ bigm * (1 - dual.z_etaR[k][i][ii][t][t + basic.period[i][ii]][s]) >= 0);
							rentalcap_comp_cons.add(dual.etaR[k][i][ii][t][t + basic.period[i][ii]][s]
								- bigm * (1 - dual.z_etaR[k][i][ii][t][t + basic.period[i][ii]][s]) <= 0);
						}
					}
				}
			}
		}
	}


	basic.model.add(rentalcap_comp_cons);
	rentalcap_comp_cons.setNames("rentalcap_comp_cons");
	rentalcap_comp_cons.end();
	
	//========================== Idle Capacity Complementary Cons ==========================
	IloRangeArray idlecap_comp_cons(basic.env);

	for (int k = 0; k < basic.nb_firms; k++){
		for (int i = 0; i < initial.nb_points; i++){
			for (int t = 0; t <= initial.T - 1; t++){
				for (int s = 0; s < initial.nb_nodes; s++){
					
					idlecap_comp_cons.add(primal.flow[k][i][i][t][2][s] - primal.park[k][i]
						+ bigm * dual.z_etaI[k][i][t][s] >= 0);
					idlecap_comp_cons.add(primal.flow[k][i][i][t][2][s] - primal.park[k][i]
						- bigm * dual.z_etaI[k][i][t][s] <= 0);
					idlecap_comp_cons.add(dual.etaI[k][i][t][s]
						+ bigm * (1 - dual.z_etaI[k][i][t][s]) >= 0);
					idlecap_comp_cons.add(dual.etaI[k][i][t][s]
						- bigm * (1 - dual.z_etaI[k][i][t][s]) <= 0);
					
					idlecap_comp_cons.add(dual.etaI[k][i][t][s] == 0);
				}
			}
		}
	}

	basic.model.add(idlecap_comp_cons);
	idlecap_comp_cons.setNames("idlecap_comp_cons");
	idlecap_comp_cons.end();
	//========================== Allocation Variable Complementary Cons ==========================
	IloRangeArray allocationvar_comp_cons(basic.env);

	for (int k = 0; k < basic.nb_firms; k++) {
		for (int i = 0; i < initial.nb_points; i++){
			allocationvar_comp_cons.add(primal.park[k][i] + bigm * dual.z_kappa[k][i] >= 0);
			allocationvar_comp_cons.add(primal.park[k][i] - bigm * dual.z_kappa[k][i] <= 0);
			allocationvar_comp_cons.add(dual.kappa[k][i] + bigm * (1 - dual.z_kappa[k][i]) >= 0);
			allocationvar_comp_cons.add(dual.kappa[k][i] - bigm * (1 - dual.z_kappa[k][i]) <= 0);
		}
	}

	for (int k = 0; k < basic.nb_firms; k++) {
		for (int i = 0; i < initial.nb_points; i++) {
			allocationvar_comp_cons.add(dual.kappa[k][i] == 0);
		}
	}

	basic.model.add(allocationvar_comp_cons);
	allocationvar_comp_cons.setNames("allocationvar_comp_cons");
	allocationvar_comp_cons.end();
	//========================== Rental Variable Complementary Cons ==========================
	IloRangeArray rentalvar_comp_cons(basic.env);

	for (int k = 0; k < basic.nb_firms; k++){
		for (int i = 0; i < initial.nb_points; i++){
			for (int ii = 0; ii < initial.nb_points; ii++){
				for (int t = 1; t < initial.T; t++){
					if (t + basic.period[i][ii] <= initial.T){
						for (int s = 0; s < initial.nb_nodes; s++){

							rentalvar_comp_cons.add(primal.flow[k][i][ii][t][0][s] + bigm * dual.z_dwR[k][i][ii][t][t + basic.period[i][ii]][s] >= 0);
							rentalvar_comp_cons.add(primal.flow[k][i][ii][t][0][s] - bigm * dual.z_dwR[k][i][ii][t][t + basic.period[i][ii]][s] <= 0);
							rentalvar_comp_cons.add(dual.dwR[k][i][ii][t][t + basic.period[i][ii]][s]
								+ bigm * (1 - dual.z_dwR[k][i][ii][t][t + basic.period[i][ii]][s]) >= 0);
							rentalvar_comp_cons.add(dual.dwR[k][i][ii][t][t + basic.period[i][ii]][s]
								- bigm * (1 - dual.z_dwR[k][i][ii][t][t + basic.period[i][ii]][s]) <= 0);
							
							rentalvar_comp_cons.add(dual.dwR[k][i][ii][t][t + basic.period[i][ii]][s] == 0);

						}
					}
				}
			}
		}
	}

	basic.model.add(rentalvar_comp_cons);
	rentalvar_comp_cons.setNames("rentalvar_comp_cons");
	rentalvar_comp_cons.end();
	//========================== Relocation Variable Complementary Cons (Relax) ==========================
	
	IloRangeArray relocationvar_comp_cons(basic.env);

	for (int k = 0; k < basic.nb_firms; k++){
		for (int i = 0; i < initial.nb_points; i++){
			for (int ii = 0; ii < initial.nb_points; ii++){
				for (int t = 1; t < initial.T; t++){
					if (t + basic.period[i][ii] <= initial.T){
						for (int s = 0; s < initial.nb_nodes; s++){

							basic.model.add(IloConversion(basic.env, dual.z_dwL[k][i][ii][t][t + basic.period[i][ii]][s], ILOFLOAT));

							relocationvar_comp_cons.add(primal.flow[k][i][ii][t][1][s] + bigm * dual.z_dwL[k][i][ii][t][t + basic.period[i][ii]][s] >= 0);
							relocationvar_comp_cons.add(primal.flow[k][i][ii][t][1][s] - bigm * dual.z_dwL[k][i][ii][t][t + basic.period[i][ii]][s] <= 0);
							relocationvar_comp_cons.add(dual.dwL[k][i][ii][t][t + basic.period[i][ii]][s]
								+ bigm * (1 - dual.z_dwL[k][i][ii][t][t + basic.period[i][ii]][s]) >= 0);
							relocationvar_comp_cons.add(dual.dwL[k][i][ii][t][t + basic.period[i][ii]][s]
								- bigm * (1 - dual.z_dwL[k][i][ii][t][t + basic.period[i][ii]][s]) <= 0);
						}
					}
				}
			}
		}
	}

	basic.model.add(relocationvar_comp_cons);
	relocationvar_comp_cons.setNames("relocationvar_comp_cons");
	relocationvar_comp_cons.end();
	
	//========================== Idle Variable Complementary Cons ==========================
	IloRangeArray idlevar_comp_cons(basic.env);
	for (int k = 0; k < basic.nb_firms; k++){
		for (int i = 0; i < initial.nb_points; i++){
			for (int t = 0; t <= initial.T - 1; t++){
				for (int s = 0; s < initial.nb_nodes; s++){
					idlevar_comp_cons.add(primal.flow[k][i][i][t][2][s] + bigm * dual.z_dwI[k][i][t][s] >= 0);
					idlevar_comp_cons.add(primal.flow[k][i][i][t][2][s] - bigm * dual.z_dwI[k][i][t][s] <= 0);
					idlevar_comp_cons.add(dual.dwI[k][i][t][s] + bigm * (1 - dual.z_dwI[k][i][t][s]) >= 0);
					idlevar_comp_cons.add(dual.dwI[k][i][t][s] - bigm * (1 - dual.z_dwI[k][i][t][s]) <= 0);
				}
			}
		}
	}

	basic.model.add(idlevar_comp_cons);
	idlevar_comp_cons.setNames("idlevar_comp_cons");
	idlevar_comp_cons.end();
	//========================== Transfer Variable Complementary Cons ==========================
	if (basic.sharing_option == 1) {
		IloRangeArray transfervar_comp_cons(basic.env);
		for (int k = 0; k < basic.nb_firms; k++) {
			for (int i = 0; i < initial.nb_points; i++) {
				for (int t = 1; t <= initial.T - 1; t++) {
					for (int s = 0; s < initial.nb_nodes; s++) {
						transfervar_comp_cons.add(primal.flow[k][i][i][t][3][s] + bigm * dual.z_dwT[k][i][t][s] >= 0);
						transfervar_comp_cons.add(primal.flow[k][i][i][t][3][s] - bigm * dual.z_dwT[k][i][t][s] <= 0);
						transfervar_comp_cons.add(dual.dwT[k][i][t][s] + bigm * (1 - dual.z_dwT[k][i][t][s]) >= 0);
						transfervar_comp_cons.add(dual.dwT[k][i][t][s] - bigm * (1 - dual.z_dwT[k][i][t][s]) <= 0);
					}
				}
			}
		}

		basic.model.add(transfervar_comp_cons);
		transfervar_comp_cons.setNames("transfervar_comp_cons");
		transfervar_comp_cons.end();
	}
	//========================== Return Variable Complementary Cons ==========================
	if (basic.sharing_option == 1) {
		IloRangeArray return_comp_cons(basic.env);
		for (int k = 0; k < basic.nb_firms; k++) {
			for (int i = 0; i < initial.nb_points; i++) {
				for (int t = 1; t <= initial.T - 1; t++) {
					for (int d = 1; d <= std::min(2, initial.T - t); d++) {
						for (int s = 0; s < initial.nb_nodes; s++) {
							return_comp_cons.add(primal.re[k][i][t][d][s] + bigm * dual.z_dre[k][i][t][d][s] >= 0);
							return_comp_cons.add(primal.re[k][i][t][d][s] - bigm * dual.z_dre[k][i][t][d][s] <= 0);
							return_comp_cons.add(dual.dre[k][i][t][d][s] + bigm * (1 - dual.z_dre[k][i][t][d][s]) >= 0);
							return_comp_cons.add(dual.dre[k][i][t][d][s] - bigm * (1 - dual.z_dre[k][i][t][d][s]) <= 0);
						}
					}
				}
			}
		}

		basic.model.add(return_comp_cons);
		return_comp_cons.setNames("return_comp_cons");
		return_comp_cons.end();
	}
} // the end of void

void create_obj(initial_parameter initial, basic_parameter& basic, primal_variable primal){

	IloObjective objfunction(basic.env);

	//************************************ Min Total Cost (option = 1) ************************************
	IloExpr totalcost(basic.env);
	IloExpr obj1(basic.env); // stage 1
	IloExpr obj2(basic.env); // stage 2
	std::cout << "firms: " << basic.nb_firms << " Time: "<< initial.T << " Nodes: " << initial.nb_nodes << " Points: " << initial.nb_points << std::endl;
	for (int k = 0; k < basic.nb_firms; k++){
		//================ stage 1 ================

		//----- costs of allocations -----
		IloExpr sumx(basic.env);

		for (int j = 0; j < initial.nb_points; j++){
			sumx += primal.park[k][j];
		}

		obj1 += basic.cost_purchase[k] * sumx;

		sumx.end();
		
		//================ stage 2 ================

		for (int s = 0; s < initial.nb_nodes; s++){
			IloExpr sum_rental_cost(basic.env);
			//----- costs of rental arcs -----
			for (int i = 0; i < initial.nb_points; i++){
				for (int ii = 0; ii < initial.nb_points; ii++){
					for (int t = 1; t < initial.T; t++){
						if (i!=ii && t + basic.period[i][ii] <= initial.T){
							sum_rental_cost += -basic.rental_price[k] * primal.flow[k][i][ii][t][0][s];
							sum_rental_cost += basic.penalty[k] * (primal.demandvar[k][i][ii][t][s] - primal.flow[k][i][ii][t][0][s]);
						}
					}
				}
			}
			
			//----- costs of relocation arcs -----
			IloExpr sum_relocation_cost(basic.env);
			for (int i = 0; i < initial.nb_points; i++){
				for (int ii = 0; ii < initial.nb_points; ii++){
					for (int t = 1; t < initial.T; t++){
						if (i != ii && t + basic.period[i][ii] <= initial.T){
							sum_relocation_cost += basic.h[k][1] * primal.flow[k][i][ii][t][1][s];
						}
					}
				}
			}
			//----- costs of idle arcs -----
			IloExpr sum_idle_cost(basic.env);
			for (int i = 0; i < initial.nb_points; i++){
				for (int t = 0; t < initial.T; t++){
					sum_idle_cost += basic.h[k][2] * primal.flow[k][i][i][t][2][s];
				}
			}
			//----- costs of transfer arcs -----
			IloExpr sum_trans_cost(basic.env);
			for (int i = 0; i < initial.nb_points; i++){
				for (int t = 1; t <= initial.T - 1; t++){
					sum_trans_cost += -basic.trans_price[k] * primal.flow[k][i][i][t][3][s];
					sum_trans_cost += basic.trans_price[1 - k] * primal.flow[1 - k][i][i][t][3][s];
				}
			}
			//----- total costs of stage 2 -----
			obj2 += basic.probTheta[s] * (sum_rental_cost + sum_relocation_cost + sum_idle_cost + sum_trans_cost);

			sum_rental_cost.end();
			sum_relocation_cost.end();
			sum_idle_cost.end();
			sum_trans_cost.end();

		}
	}
	totalcost = obj1 + obj2;

	//************************************ Max Total Transfer (option = 2) ************************************

	IloExpr totaltrans(basic.env);
	for (int k = 0; k < basic.nb_firms; k++){
		for (int i = 0; i < initial.nb_points; i++){
			for (int t = 1; t <= initial.T - 1; t++){
				for (int s = 0; s < initial.nb_nodes; s++){
					totaltrans += primal.flow[k][i][i][t][3][s];
				}
			}
		}
	}

	//************************************ Min Unsatisfied Demands (option = 3) ************************************

	IloExpr totalunsatisfied(basic.env);
	for (int k = 0; k < basic.nb_firms; k++){
		for (int i = 0; i < initial.nb_points; i++){
			for (int ii = 0; ii < initial.nb_points; ii++){
				for (int t = 1; t < initial.T; t++){
					if (i != ii && t + basic.period[i][ii] <= initial.T){
						for (int s = 0; s < initial.nb_nodes; s++){
							totalunsatisfied += basic.probTheta.at(s) * (primal.demandvar[k][i][ii][t][s] - primal.flow[k][i][ii][t][0][s]);
						}
					}
				}
			}
		}
	}

	//************************************ Min Total Allocations (option = 4) ************************************

	IloExpr totalallocations(basic.env);
	for (int k = 0; k < basic.nb_firms; k++) {
		for (int i = 0; i < initial.nb_points; i++) {
			totalallocations += primal.park[k][i];
		}
	}

	//************************************ Add OBJ ************************************
	switch (basic.obj_option){
		case 1: objfunction = IloMinimize(basic.env, totalcost); 
		break;
		case 2: objfunction = IloMaximize(basic.env, totaltrans);
		break;
		case 3: objfunction = IloMinimize(basic.env, totalunsatisfied);
		break;
		case 4: objfunction = IloMinimize(basic.env, totalallocations);
	}
	basic.model.add(objfunction);

	totalcost.end();
	totaltrans.end();
	totalunsatisfied.end();
	totalallocations.end();
}

void resolve_problem(initial_parameter initial, basic_parameter& basic, primal_variable primal) {
	vector<vector<double> > all_val(basic.nb_firms, vector<double>(initial.nb_points, 0));
	for (int k = 0; k < basic.nb_firms; k++) {
		for (int i = 0; i < initial.nb_points; i++) {
			all_val.at(k).at(i) = basic.cplex.getValue(primal.park[k][i]);
		}
	}

	basic.cplex.clear();

	IloRangeArray allocationcons(basic.env);

	for (int k = 0; k < basic.nb_firms; k++) {
		for (int i = 0; i < initial.nb_points; i++) {
			allocationcons.add(primal.park[k][i] - all_val.at(k).at(i) == 0);
		}
	}
	basic.model_final.add(allocationcons);
	allocationcons.end();

	//************************************ Min Total Cost (option = 1) ************************************
	IloExpr totalcost(basic.env);
	IloExpr obj1(basic.env); // stage 1
	IloExpr obj2(basic.env); // stage 2
	for (int k = 0; k < basic.nb_firms; k++) {
		//================ stage 1 ================

		//----- costs of allocations -----
		IloExpr sumx(basic.env);

		for (int j = 0; j < initial.nb_points; j++) {
			sumx += primal.park[k][j];
		}

		obj1 += basic.cost_purchase[k] * sumx;

		sumx.end();

		//================ stage 2 ================

		for (int s = 0; s < initial.nb_nodes; s++) {
			IloExpr sum_rental_cost(basic.env);
			//----- costs of rental arcs -----
			for (int i = 0; i < initial.nb_points; i++) {
				for (int ii = 0; ii < initial.nb_points; ii++) {
					for (int t = 1; t < initial.T; t++) {
						if (i != ii && t + basic.period[i][ii] <= initial.T) {
							sum_rental_cost += -basic.rental_price[k] * primal.flow[k][i][ii][t][0][s];
							sum_rental_cost += basic.penalty[k] * (primal.demandvar[k][i][ii][t][s] - primal.flow[k][i][ii][t][0][s]);
						}
					}
				}
			}

			//----- costs of relocation arcs -----
			IloExpr sum_relocation_cost(basic.env);
			for (int i = 0; i < initial.nb_points; i++) {
				for (int ii = 0; ii < initial.nb_points; ii++) {
					for (int t = 1; t < initial.T; t++) {
						if (i != ii && t + basic.period[i][ii] <= initial.T) {
							sum_relocation_cost += basic.h[k][1] * primal.flow[k][i][ii][t][1][s];
						}
					}
				}
			}
			//----- costs of idle arcs -----
			IloExpr sum_idle_cost(basic.env);
			for (int i = 0; i < initial.nb_points; i++) {
				for (int t = 0; t < initial.T; t++) {
					sum_idle_cost += basic.h[k][2] * primal.flow[k][i][i][t][2][s];
				}
			}
			//----- costs of transfer arcs -----
			IloExpr sum_trans_cost(basic.env);
			for (int i = 0; i < initial.nb_points; i++) {
				for (int t = 1; t <= initial.T - 1; t++) {
					sum_trans_cost += -basic.trans_price[k] * primal.flow[k][i][i][t][3][s];
					sum_trans_cost += basic.trans_price[1 - k] * primal.flow[1 - k][i][i][t][3][s];
				}
			}
			//----- total costs of stage 2 -----
			obj2 += basic.probTheta[s] * (sum_rental_cost + sum_relocation_cost + sum_idle_cost + sum_trans_cost);

			sum_rental_cost.end();
			sum_relocation_cost.end();
			sum_idle_cost.end();
			sum_trans_cost.end();

		}
	}
	totalcost = obj1 + obj2;

	basic.model_final.add(IloMinimize(basic.env, totalcost));

	basic.cplex.extract(basic.model_final);

	basic.cplex.solve();
}

void create_no_competition(initial_parameter initial, basic_parameter& basic, primal_variable primal) {
	//************************************ Min Total Cost (option = 1) ************************************
	IloExpr totalcost(basic.env);
	IloExpr obj1(basic.env); // stage 1
	IloExpr obj2(basic.env); // stage 2
	for (int k = 0; k < basic.nb_firms; k++) {
		//================ stage 1 ================

		//----- costs of allocations -----
		IloExpr sumx(basic.env);

		for (int j = 0; j < initial.nb_points; j++) {
			sumx += primal.park[k][j];
		}

		obj1 += basic.cost_purchase[k] * sumx;

		sumx.end();

		//================ stage 2 ================

		for (int s = 0; s < initial.nb_nodes; s++) {
			IloExpr sum_rental_cost(basic.env);
			//----- costs of rental arcs -----
			for (int i = 0; i < initial.nb_points; i++) {
				for (int ii = 0; ii < initial.nb_points; ii++) {
					for (int t = 1; t < initial.T; t++) {
						if (i != ii && t + basic.period[i][ii] <= initial.T) {
							sum_rental_cost += -basic.rental_price[k] * primal.flow[k][i][ii][t][0][s];
							sum_rental_cost += basic.penalty[k] * (primal.demandvar[k][i][ii][t][s] - primal.flow[k][i][ii][t][0][s]);
						}
					}
				}
			}

			//----- costs of relocation arcs -----
			IloExpr sum_relocation_cost(basic.env);
			for (int i = 0; i < initial.nb_points; i++) {
				for (int ii = 0; ii < initial.nb_points; ii++) {
					for (int t = 1; t < initial.T; t++) {
						if (i != ii && t + basic.period[i][ii] <= initial.T) {
							sum_relocation_cost += basic.h[k][1] * primal.flow[k][i][ii][t][1][s];
						}
					}
				}
			}
			//----- costs of idle arcs -----
			IloExpr sum_idle_cost(basic.env);
			for (int i = 0; i < initial.nb_points; i++) {
				for (int t = 0; t < initial.T; t++) {
					sum_idle_cost += basic.h[k][2] * primal.flow[k][i][i][t][2][s];
				}
			}
			//----- total costs of stage 2 -----
			obj2 += basic.probTheta[s] * (sum_rental_cost + sum_relocation_cost + sum_idle_cost);

			sum_rental_cost.end();
			sum_relocation_cost.end();
			sum_idle_cost.end();

		}
	}

	totalcost = obj1 + obj2;

	basic.model_independent.add(IloMinimize(basic.env, totalcost));

	//************************************ Constraints ************************************

	//----------- Budget Cons ----------
	IloRangeArray budgetcons(basic.env);

	for (int i = 0; i < basic.nb_firms; i++) {
		IloExpr totalallocationexpr(basic.env);
		for (int j = 0; j < initial.nb_points; j++) {
			totalallocationexpr += primal.park[i][j];
		}
		budgetcons.add(totalallocationexpr - basic.budget[i] <= 0);
		totalallocationexpr.end();
	}

	basic.model_independent.add(budgetcons);
	budgetcons.setNames("budgetcons");
	budgetcons.end();
	//----------- Allocation Cons -----------
	IloRangeArray allocationcons(basic.env);

	vector<vector<int> > all_val(basic.nb_firms, vector<int>(initial.nb_points, 0));
	for (int j = 0; j < initial.nb_points; j++) {
		all_val.at(0).at(j) = static_cast<int>(basic.totallots.at(j) * basic.alpha.at(0) / (basic.alpha.at(0) + basic.alpha.at(1)));
		all_val.at(1).at(j) = basic.totallots.at(j) - all_val.at(0).at(j);
	}

	for (int k = 0; k < basic.nb_firms; k++) {
		for (int j = 0; j < initial.nb_points; j++) {
			allocationcons.add(primal.park[k][j] <= all_val.at(k).at(j));
		}
	}
	basic.model_independent.add(allocationcons);
	allocationcons.setNames("allocationcons");
	allocationcons.end();
	//========================== Second Stage ==========================

	//----------- Demand Cons -----------
	double sum_total_allocation = 0;
	IloExpr sum_allocationvar_firm1(basic.env);
	IloExpr sum_allocationvar_firm2(basic.env);
	for (int i = 0; i < initial.nb_points; i++) {
		sum_total_allocation += basic.totallots[i];
		sum_allocationvar_firm1 += primal.park[0][i];
		sum_allocationvar_firm2 += primal.park[1][i];
	}
	IloRangeArray demandcons(basic.env);

	for (int k = 0; k < basic.nb_firms; k++) {
		for (int i = 0; i < initial.nb_points; i++) {
			for (int ii = 0; ii < initial.nb_points; ii++) {
				for (int t = 1; t <= initial.T; t++) {
					for (int s = 0; s < initial.nb_nodes; s++) {
						if (i != ii && t + basic.period[i][ii] <= initial.T) {
							if (k == 0) {
								demandcons.add(primal.demandvar[k][i][ii][t][s] - basic.Y1[i][ii][t][t + basic.period[i][ii]][s] * sum_allocationvar_firm1 / sum_total_allocation
									- basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] == 0);
							}
							else {
								demandcons.add(primal.demandvar[k][i][ii][t][s] - basic.Y1[i][ii][t][t + basic.period[i][ii]][s] * sum_allocationvar_firm2 / sum_total_allocation
									- basic.Y[k][i][ii][t][t + basic.period[i][ii]][s] == 0);
							}
						}
					}
				}
			}
		}
	}

	sum_allocationvar_firm1.end();
	sum_allocationvar_firm2.end();

	basic.model_independent.add(demandcons);
	demandcons.setNames("demandcons");
	demandcons.end();
	//----------- Node Balance Cons -----------
	IloRangeArray flowbalancecons(basic.env);

	// s=[1,T-1]
	for (int i = 0; i < basic.nb_firms; i++) {
		for (int n = 0; n < initial.nb_nodes; n++) {
			for (int k = 0; k < initial.nb_points; k++) {
				for (int s = 1; s < initial.T; s++) {
					IloExpr sumin(basic.env);
					IloExpr sumout(basic.env);
					// sumin
					// rental & relocation
					for (int j = 0; j < initial.nb_points; j++) {
						if (j != k && basic.period[j][k] < s) {
							sumin += primal.flow[i][j][k][s - basic.period[j][k]][0][n] + primal.flow[i][j][k][s - basic.period[j][k]][1][n];
						}
					}
					// idle
					sumin += primal.flow[i][k][k][s - 1][2][n];
					// sumout
					// rental & relocation
					for (int j = 0; j < initial.nb_points; j++) {
						if (j != k) {
							sumout += primal.flow[i][k][j][s][0][n] + primal.flow[i][k][j][s][1][n];
						}
					}
					// idle
					sumout += primal.flow[i][k][k][s][2][n];

					flowbalancecons.add(sumin - sumout == 0);

					sumin.end();
					sumout.end();
				}
			}
		}
	}
	// s=0
	for (int i = 0; i < basic.nb_firms; i++) {
		for (int n = 0; n < initial.nb_nodes; n++) {
			for (int j = 0; j < initial.nb_points; j++) {
				flowbalancecons.add(primal.flow[i][j][j][0][2][n] - primal.park[i][j] == 0);
			}
		}
	}
	// s=T
	for (int i = 0; i < basic.nb_firms; i++) {
		for (int n = 0; n < initial.nb_nodes; n++) {
			for (int k = 0; k < initial.nb_points; k++) {
				IloExpr suminT(basic.env);
				// suminT
				// rental & relocation
				for (int j = 0; j < initial.nb_points; j++) {
					if (j != k && basic.period[j][k] < initial.T) {
						suminT += primal.flow[i][j][k][initial.T - basic.period[j][k]][0][n] + primal.flow[i][j][k][initial.T - basic.period[j][k]][1][n];
					}
				}
				// idle
				suminT += primal.flow[i][k][k][initial.T - 1][2][n];

				flowbalancecons.add(suminT - primal.park[i][k] == 0);

				suminT.end();
			}
		}
	}

	basic.model_independent.add(flowbalancecons);
	flowbalancecons.setNames("flowbalancecons");
	flowbalancecons.end();
	//----------- # of rental bikes constraints -----------
	IloRangeArray rentalcapacitycons(basic.env);

	for (int i = 0; i < basic.nb_firms; i++) {
		for (int j = 0; j < initial.nb_points; j++) {
			for (int k = 0; k < initial.nb_points; k++) {
				for (int s = 1; s < initial.T; s++) {
					for (int n = 0; n < initial.nb_nodes; n++) {
						if (k != j && s + basic.period[j][k] <= initial.T) {
							rentalcapacitycons.add(primal.flow[i][j][k][s][0][n] - primal.demandvar[i][j][k][s][n] <= 0);
						}
					}
				}
			}
		}
	}

	basic.model_independent.add(rentalcapacitycons);
	rentalcapacitycons.setNames("rentalcapacitycons");
	rentalcapacitycons.end();
	//----------- inventory level constraints -----------
	IloRangeArray inventorycapacitycons(basic.env);

	for (int i = 0; i < basic.nb_firms; i++) {
		for (int j = 0; j < initial.nb_points; j++) {
			for (int s = 1; s <= initial.T; s++) {
				for (int n = 0; n < initial.nb_nodes; n++) {
					inventorycapacitycons.add(primal.flow[i][j][j][s - 1][2][n] - primal.park[i][j] <= 0);
				}
			}
		}
	}

	inventorycapacitycons.setNames("inventorycapacitycons");
	basic.model_independent.add(inventorycapacitycons);
	inventorycapacitycons.end();
	
}
