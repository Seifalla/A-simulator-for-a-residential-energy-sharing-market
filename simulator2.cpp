
#include <iostream>
#include <string>
#include <random>
#include <time.h>
#include <vector>
#include <math.h>

using namespace std;

#define MAX_NO_USERS 1000
#define MEAN 5
#define STDDEV 2
#define VOLTAGE 110

struct user{
	float l;
	float g;
	float purchases;
};

void insertion_sort(vector<int> &consumers, struct user u[]);
void shortest_path(float adj[][100], int s, int v, float d[], int pred[]);
void print_path(float adj[][100], int s, int v, int pred[]);


int main (int argc, char **argv){

	if( argc < 7) {

		printf("seven arguments expected\n");
		exit(1);
	}
	else{
		//printf("1st: %d\t2nd: %d\n", atoi(argv[1]), atoi(argv[2]));
	}

	  /* initialize random seed: */
	srand (time(NULL));
	struct user u[MAX_NO_USERS];
	int time = 0;
	int num_of_satisfied;
	float total, total_loss, produced_energy, total_energy_exchanged, average_energy, sum_percentage_DS, ave_percentage_DS, consumed_energy;
	float total_energy_produced, total_energy_consumed, average_energy_produced, average_energy_consumed;
	int numofusers = atoi(argv[1]) + atoi(argv[2]);
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator(seed);

	// consumption and generation are modeled as random variables with lognormal distributions

	lognormal_distribution<float> distribution[MAX_NO_USERS]; // more control
	lognormal_distribution<float> distribution1[MAX_NO_USERS]; // more control

	// assign different parameters to each user

	for (int i = 0; i <= numofusers - 1; i ++){

		
		distribution[i] = std::lognormal_distribution<float>(rand()%1, rand()%2+1); // ****
	}
	vector<int> producer;
	vector<int> consumers;

	// a user is identified by his ID
	// producer IDs are chosen at random

	for (int j = 0; j <= atoi(argv[1]) - 1; j ++){  

		int k = rand() % numofusers; 
		while(search_n(producer.begin(), producer.end(), 1, k) != producer.end())
			k = rand() % numofusers;
		producer.push_back(k);
		distribution1[k] = std::lognormal_distribution<float>(rand()%1, rand()%2+1);
	}

	// consumer IDs are chosen at random

	for (int j = 0; j <= numofusers - 1; j ++){

		if (search_n(producer.begin(), producer.end(), 1, j) == producer.end())

			consumers.push_back(j);
		else{}
	}

	// build the adjacency matrix

	float adj[100][100];
	int center = numofusers;
	for (int j = 0; j <= (numofusers); j ++){

		for (int k = 0; k <= (numofusers); k ++){

			adj[j][k] = 0;
		}
	}

	// the weights are randomly generated

	for (int j = 0; j <= (numofusers); j ++){

		if (j != center)
			adj[j][center] = rand() % 20 + 1;
	}
	for (int j = 0; j <= (numofusers); j ++){

		if (j != center)
			adj[center][j] = adj[j][center];
	}

	for (int j = 0; j <= numofusers; j ++){

		for (int k = 0; k <= numofusers; k ++){

			//cout<<adj[j][k]<<"  ";
		}
		//cout<<endl;
	}

	vector<int> deleted;

	total_energy_exchanged = 0;

	sum_percentage_DS = 0;

	total_energy_produced = 0;

	total_energy_consumed = 0;

	// the main loop. The number of iterations is passed to the program as a command-line argument

	for (int v = 0; v <= atoi(argv[3]) - 1; v ++){
	
		//cout<<"time step "<< time<<endl;
		total = 0;
		total_loss = 0;
		produced_energy = 0;
		consumed_energy = 0;
		num_of_satisfied = 0;

		// Choose an outcome randomly using the distribution

		for (int i = 0; i <= numofusers - 1; i ++){

			if (search_n(producer.begin(), producer.end(), 1, i) != producer.end()){

				//u[i].g = distribution1[i](generator);
				u[i].g = (rand()%(atoi(argv[5])-atoi(argv[4])+1)) + atoi(argv[4]);
				produced_energy = produced_energy + (u[i].g);
				//cout<<"net power at producer "<<i<<": "<<(u[i].g)<<endl;
			}
			else{

				//u[I].l = distribution[i](generator);
				u[i].l = (rand()%(atoi(argv[7])-atoi(argv[6])+1)) + atoi(argv[6]);
				consumed_energy = consumed_energy + u[i].l;
			}
			//cout<<"load at "<<i<<": "<<u[i].l<<endl;
		}

		deleted.clear();

		//sort consumers by decreasing abs(u[consumers[j]].g - u[consumers[j]].l) 

		insertion_sort(consumers, u);

		// consider users in order of decreasing shortage

		for (int j = 0; j <= consumers.size() - 1; j ++){

			//cout<<"({"<<consumers[j]<<"}, {";
			//cout<<"home "<<consumers[j]<<endl;
			vector<int> recommended;
			int satisfied = 0;
			int n = 0;
			float loss = 0;
			float d[100];
			int pred[100];
			
			// keep buying energy until your demand is fulfilled

			while(!satisfied && (n <= producer.size() - 1)){

				// find the nearest neighbor of consumer j

				shortest_path(adj, consumers[j], numofusers+1, d, pred);
				int index = -1, min = 1000;
				for(int h = 0; h <= producer.size() - 1; h ++){

					//cout<<d[producer[h]]<<endl;
					//cout<<d[producer[h]]<<" vs "<<min<<endl;
					//cout<<(d[producer[h]] < min)<<endl;
					//cout<<(search_n(recommended.begin(), recommended.end(), 1, producer[h]) == recommended.end())<<endl;
					if((d[producer[h]] < min) && (search_n(recommended.begin(), recommended.end(), 1, producer[h]) == recommended.end())
					&& (search_n(deleted.begin(), deleted.end(), 1, producer[h]) == deleted.end())){

						//cout<<"iteration "<<h<<endl;
						min = d[producer[h]];
						index = producer[h];
					}
				}


				int r = index;
							
				//cout<<"("<<consumers[j]<<", "<<producer[r]<<")"<<endl;

				// add this neighbor to the recommended producer set

				if(r != -1){
	
					if (u[consumers[j]].l < u[r].g){

						loss = (adj[r][numofusers] + adj[numofusers][consumers[j]]) * ( u[consumers[j]].l/VOLTAGE)*(u[consumers[j]].l/VOLTAGE);
						if (loss < (0.1 * u[consumers[j]].l)){
	
							num_of_satisfied ++;
							recommended.push_back(r);
							satisfied = 1;
						}
						else
							n ++;
					}
					else{

						loss = (adj[r][numofusers] + adj[numofusers][consumers[j]]) * ((u[r].g)/VOLTAGE)*((u[r].g)/VOLTAGE);

					// if the energy loss is greater than 10% of the energy transmitted,
					// the consumer does not buy

						if (loss < (0.1 * (u[r].g))){

							recommended.push_back(r);
							n ++;
						}
						else
							n ++;					

 					}
				}
				else 
					n ++;
			}

			// if sum of surpluses < demand of any one consumer, discard the consumer
			// these loc are not necessary anymore since we achieve balance between
			// total generation and consumption

			if (satisfied == 0){

				recommended.clear();
			}

			float energy_transfer = 0;

			//if(recommended.size() == 0)

				//cout<<"no recommendation"<<endl;

			// transmit the energy
			
			for (int f = 0; f < recommended.size(); f ++){
	
				//if(f != recommended.size() - 1)
				//	cout<<recommended[f]<<",";
				//else
					//cout<<recommended[f]<<"})"<<endl;
				//cout<<"net power at producer "<<recommended[f]<<": "<<(u[recommended[f]].g)<<endl;
				if (u[consumers[j]].l < u[recommended[f]].g){

					loss = (adj[recommended[f]][numofusers] + adj[numofusers][consumers[j]]) * ( u[consumers[j]].l/VOLTAGE)*(u[consumers[j]].l/VOLTAGE);
					//cout<<loss<<" vs "<< (0.1 * (abs(u[consumers[j]].g - u[consumers[j]].l)));
					energy_transfer = u[consumers[j]].l;
					u[consumers[j]].purchases = u[consumers[j]].purchases + energy_transfer;
					total = total + energy_transfer;
					u[recommended[f]].g = u[recommended[f]].g - energy_transfer;// dad
					//cout<<"net power at producer "<<recommended[f]<<": "<<(u[recommended[f]].g)<<endl;
					total_loss = total_loss + loss;
					energy_transfer = energy_transfer - loss;
					u[consumers[j]].l = u[consumers[j]].l - energy_transfer;
					//cout<<"net power at "<<consumers[j]<<":"<<u[consumers[j]].g - u[consumers[j]].l<<endl;					
				}
				else{

				// compute the loss

					loss = (adj[recommended[f]][numofusers] + adj[numofusers][consumers[j]]) * ((u[recommended[f]].g)/VOLTAGE)*((u[recommended[f]].g)/VOLTAGE);
					//cout<<loss<<" vs "<< (0.1 * (u[recommended[f]].g - u[recommended[f]].l));
					energy_transfer = u[recommended[f]].g;
					u[consumers[j]].purchases = u[consumers[j]].purchases + energy_transfer;
					total = total + energy_transfer;
					u[recommended[f]].g = 0;

					// the transaction is not executed, if sum of surpluses < demand of any one consumer. Therefore, we
					// should not remove anyone that runs out of energy in the matching phase
					vector<int>::iterator it;
					it = search_n(producer.begin(), producer.end(), 1, recommended[f]);
					deleted.push_back(recommended[f]);
					total_loss = total_loss + loss;
					energy_transfer = energy_transfer - loss;
					u[consumers[j]].l = u[consumers[j]].l - energy_transfer;
					//cout<<"net power at "<<consumers[j]<<":"<<u[consumers[j]].g - u[consumers[j]].l<<endl;				
				}
			}
			//cout<<endl;
	
		}
		float percentage_of_demand_satisfied = (float)num_of_satisfied/(float)consumers.size();
		float efficiency = total/((produced_energy) + total_loss);
		//cout<<time<<","<<total<<","<<efficiency<<","<<total_loss<<","<<percentage_of_demand_satisfied<<endl;
		//cout<<v<<","<<total<<","<<efficiency<<","<<total_loss<<","<<percentage_of_demand_satisfied<<endl;
		total_energy_exchanged = total_energy_exchanged + total;
		total_energy_produced = total_energy_produced + produced_energy;
		total_energy_consumed = total_energy_consumed + consumed_energy;
		sum_percentage_DS = sum_percentage_DS + percentage_of_demand_satisfied;
		time = time + 1;
				
	}
	average_energy = total_energy_exchanged/atoi(argv[3]);
	ave_percentage_DS = sum_percentage_DS/atoi(argv[3]);
	average_energy_produced = total_energy_produced/atoi(argv[3]);
	average_energy_consumed = total_energy_consumed/atoi(argv[3]);
	cout<<average_energy<<", "<<ave_percentage_DS<<", "<<average_energy_produced<<", "<<average_energy_consumed<<endl;
}


void insertion_sort(vector<int> &consumers, struct user u[]){

	int k, l;
	for (int j = 1; j < consumers.size(); j ++){

		k = consumers[j];
		l = j - 1;
		while((l > -1) && (u[k].l) > u[consumers[l]].l){

			consumers[l + 1] = consumers[l];
			l --;
		}
		consumers[l + 1] = k;
	}
}
void shortest_path(float adj[][100], int s, int v, float d[], int pred[]){

	for (int j = 0; j <= (v - 1); j ++){

		d[j] = 1000;
		pred[j] = -1;
	}
	d[s] = 0;
	for (int j = 0; j <= (v - 1); j ++){

		for (int l = 0; l <= (v - 1); l ++){

			for (int k = 0; k <= (v - 1); k ++){

				if(adj[k][l] != 0){

					if(d[k] > (d[l] + adj[k][l])){
				
						d[k] = d[l] + adj[k][l];
						pred[k] = l;
					}
				}
			}
		}
	}
}

void print_path(float adj[][100], int s, int v, int pred[]){

	if (v == s)

		cout<<s<<endl;
	else if (pred[v] == -1)
		
		cout<<"no path between "<<s<<" and"<<v<<" exists"<<endl;
	else{

		print_path(adj, s, pred[v], pred);
		cout<<v<<endl;
	}
}
