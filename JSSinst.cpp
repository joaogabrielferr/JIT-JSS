#include "JSSinst.h"


#define sequence vector<vector<Operation> >

using namespace std;

vector<double>
JSSinst::readLine(string line)
{
	vector<double>numbers;
	stringstream ss;
	ss << line;
	string string_temp;
	double double_temp;
	while (!ss.eof())
	{
		ss >> string_temp;
		if (stringstream(string_temp) >> double_temp)
		{
			numbers.push_back(double_temp);
		}
	}
	//last number is added twice
	numbers.pop_back();
	return numbers;
}

/////////////////////////////////////////////////////////////////////////////////////////

//returns <processing order, number of jobs and machines >
void
JSSinst::parseInstance(string path)
{
	ifstream file(path);
	if (!file.is_open())
	{
		cout << "FILE COULDNT BE OPENED\n";
		abort();
	}

	vector<double>numbers;
	string line;
	getline(file, line);
	numbers = readLine(line);
	//nJobs = (int)numbers[0];
	//nMachines = (int)numbers[1];

	nJobs = (int)numbers[0];
	nMachines = (int)numbers[1];

	OpTojob.resize((this->nJobs*this->nMachines) + 1);

	
	int opID = 1;
	int jobCounter = 0;
	while(getline(file,line))
	{
		numbers = readLine(line);
		cout<<"line:";
		for(auto i : numbers)cout<<i<<" ";
		cout<<endl;
		vector<Operation>job;
		for(int i = 0;i<numbers.size();i+=5)
		{
			Operation op(jobCounter,(int)numbers[i],(int)numbers[i + 1],(int)numbers[i + 2],numbers[i + 3],numbers[i + 4],opID);
			job.push_back(op);
			OpTojob[opID] = jobCounter;
			opID++;
		}
		processingOrder.push_back(job);
		jobCounter++;
	}

	//this->processingOrder = processingOrder;
	file.close();

}

/////////////////////////////////////////////////////////////////////////////////////////

vector<double>
JSSinst::SchedulePenalties(sequence s)
{
	
	double total = 0.0, totalearliness = 0.0,totaltardiness = 0;

	for(int i = 0;i<s.size();i++)
	{
		for(int j = 0;j<s[i].size();j++)
		{
			double ea = (s[i][j].completionTime <= s[i][j].dueDate ? (s[i][j].dueDate - s[i][j].completionTime)*s[i][j].earliness  :  0);
			double ta = (s[i][j].completionTime >= s[i][j].dueDate ? (s[i][j].completionTime - s[i][j].dueDate)*s[i][j].tardiness  :  0);
			totalearliness+=ea;
			totaltardiness+=ta;
			total += (ea + ta);
		}
	}

	return {total,totalearliness,totaltardiness};

}



/////////////////////////////////////////////////////////////////////////////////////////

sequence
JSSinst::GifflerThompson(sequence instance)
{
	sequence newSchedule;
	newSchedule.resize(nMachines);

	vector<Operation>G;
	vector<int>machineStart(nMachines,0),jobStart(nJobs,0);

	//add all the schedulable operations, i.e., the first operation of each job and
	//initialize the earliest start and completion times for the operations in G
	for(int i = 0;i<nJobs;i++)
	{
		Operation op = instance[i][0];
		op.startTime = 0;
		op.completionTime = op.processingTime;	
		G.push_back(op);
	}

	//while an unscheduled operation exists do
	while(!G.empty())
	{
		//Find the earliest completion time C∗ and its associated machine M∗ among all the operations in G
		int Cstar = INT_MAX;
		int Mstar;
		
		for(Operation op : G)
		{
			if(op.completionTime < Cstar)
			{
				Cstar = op.completionTime;
				Mstar = op.machine;
			}
		}

		//let G0 ⊆ G denote the operations processed on machine M∗;
		vector<Operation>G0;
		for(Operation op : G)
		{
			if(op.machine == Mstar)G0.push_back(op);
		}

		//G1 = {Op ∈ G0|op.startTime < C∗} (building the conflict set);
		vector<Operation>G1;
		for(Operation op : G0)
		{
			if(op.startTime < Cstar)G1.push_back(op);
		}

		//Select op ∈ G1 to be scheduled next (by randomly picking a op in G1);
		srand(time(NULL));
		int lowest = 0;
		int highest = G1.size() - 1;
		int range = (highest - lowest) + 1;
		int random = lowest + rand() % range;
		Operation op = G1[random];
		//Select op ∈ G1 to be scheduled next (by using the earliest due date dispatching rule);
		// Operation op = G1[0];
		// for(int i = 1;i<G1.size();i++)
		// {
		// 	if(G1[i].dueDate < op.dueDate)
		// 	{
		// 		op = G1[i];
		// 	}
		// }

		//earliest possible start time for an operation:
		//max(earliet start in machine predecessor, earliest start in job precedessor)
		
		int opJob = OpTojob[op.id];

		if(machineStart[op.machine] >= jobStart[opJob])
		{
			op.startTime = machineStart[op.machine];
			op.completionTime = op.startTime + op.processingTime;
			machineStart[op.machine] = op.completionTime;
			jobStart[opJob] = op.completionTime;
		}else
		{
			op.startTime = jobStart[opJob];
			op.completionTime = op.startTime + op.processingTime;
			jobStart[opJob] = op.completionTime;
			machineStart[op.machine] = op.completionTime;
		} 

		//scheduling op
		newSchedule[op.machine].push_back(op);

		//Remove op from G and add its job successor (if any) to G;
		
		//removing op
		int Opidx = 0;
		for(int i = 0;i<G.size();i++)
		{
			if(G[i].id == op.id)
			{
				Opidx = i;
				break;
			}
		}
		Operation aux = G[G.size() - 1];
		G[G.size() - 1] = G[Opidx];
		G[Opidx] = aux;
		G.pop_back();
	
		//adding successor if any
		for(int i = 0;i<instance.size();i++)
		{
			for(int j = 0;j<instance[i].size() - 1;j++)
			{
				if(instance[i][j].id == op.id)
				{
					Operation successor = instance[i][j+1];
					successor.startTime = max(machineStart[successor.machine],jobStart[OpTojob[successor.id]]);
					successor.completionTime = successor.startTime + successor.processingTime;
					G.push_back(successor);
				}
			}
		}

		//Update the earliest start and completion times
		for(Operation o : G)
		{
			o.startTime = max(machineStart[o.machine],jobStart[OpTojob[o.id]]);
			o.completionTime = o.startTime + o.processingTime;
		}
	}


	return newSchedule;	
}

sequence
JSSinst::EarliestDeadlineFirst(sequence instance)
{
	sequence newSchedule;
	newSchedule.resize(nMachines);

	vector<Operation>scheduable;
	vector<int>machineStart(nMachines,0),jobStart(nJobs,0);
	
	for(int i = 0;i<nJobs;i++)
	{
		scheduable.push_back(instance[i][0]);
	}

	while(!scheduable.empty())
	{
		Operation op = scheduable[0];
		int opidx = 0;
		for(int i = 1;i<scheduable.size();i++)
		{
			if(scheduable[i].dueDate < op.dueDate)
			{
				op = scheduable[i];
				opidx = i;
			}
		}

		scheduable[opidx] = scheduable[scheduable.size() - 1];
		scheduable.pop_back();

		if(machineStart[op.machine] >= jobStart[op.job])
		{
			op.startTime = machineStart[op.machine];
			op.completionTime = op.startTime + op.processingTime;
			machineStart[op.machine] = op.completionTime;
			jobStart[op.job] = op.completionTime;
		}else
		{
			op.startTime = jobStart[op.job];
			op.completionTime = op.startTime + op.processingTime;
			jobStart[op.job] = op.completionTime;
			machineStart[op.machine] = op.completionTime;
		}

		newSchedule[op.machine].push_back(op);
		
		for(int i = 0;i<instance[op.job].size() - 1;i++)
		{
			if(op.id == instance[op.job][i].id)
			{
				scheduable.push_back(instance[op.job][i + 1]);
			}
		}
	}


	return newSchedule;
}


sequence
JSSinst::LocalSearch(sequence instance,int MAX_ITER){

	sequence x;
	double best = 100000000.0;

	//rs
	for(int i = 0;i<500;i++)
	{
		sequence aux = GifflerThompson(instance);
		vector<double>r = SchedulePenalties(aux);
		if(r[0] < best)
		{
			best = r[0];
			x = aux;
		}
	}


	


	return x;
}