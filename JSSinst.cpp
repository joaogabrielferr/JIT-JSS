#include "jssinst.h"

#define matriz vector<vector<int> >

using namespace std;

template <
    class result_t   = std::chrono::seconds,
    class clock_t    = std::chrono::steady_clock,
    class duration_t = std::chrono::seconds
>
auto since(std::chrono::time_point<clock_t, duration_t> const& start)
{
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}


vector<double>
JIT_JSS::readLine(string line)
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

void
JIT_JSS::parseInstance(string path)
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
	
	nJobs = (int)numbers[0];
	nMachines = (int)numbers[1];

	//OpTojob.resize((this->nJobs*this->nMachines) + 1);
	nOperations = nJobs*nMachines;
	//resizing operation vectors with nOperations + 1 because the operations start from 1
	//cout<<nOperations<<endl;
	processingTime.resize(nOperations + 1);
	dueDate.resize(nOperations + 1);
	earliness.resize(nOperations + 1);
	tardiness.resize(nOperations + 1);
	job.resize(nOperations + 1);
	machine.resize(nOperations + 1);
	
	//machine and job start from 0
	jobOps.resize(nJobs);
	machineOps.resize(nMachines);

	int opID = 1;
	int jobID = 0;
	//cout<<"jobOps size:"<<jobOps.size()<<endl;
	while(getline(file,line))
	{
		numbers = readLine(line);
		// cout<<"line:";
		// for(auto i : numbers)cout<<i<<" ";
		// cout<<endl;
		vector<int> _job;
		for(int i = 0;i<numbers.size();i+=5)
		{
			int opmachine = (int)numbers[i];
			int opprocessingtime = (int)numbers[i + 1];
			int opduedate = (int)numbers[i + 2];
			double opearliness = numbers[i + 3];
			double optardiness = numbers[i + 4];
			job[opID] = jobID;
			machine[opID] = opmachine;
			processingTime[opID] = opprocessingtime;
			dueDate[opID] = opduedate;
			earliness[opID] = opearliness;
			tardiness[opID] = optardiness;

			_job.push_back(opID);

			jobOps[jobID].push_back(opID);
			machineOps[opmachine].push_back(opID);
			opID++;
		}
		processingOrder.push_back(_job);
		jobID++;
	}

	//this->processingOrder = processingOrder;
	file.close();

}

/////////////////////////////////////////////////////////////////////////////////////////

vector<double>
JIT_JSS::SchedulePenalties(matriz s,vector<int>scheduleStartTimes)
{
	
	double total = 0.0, totalearliness = 0.0,totaltardiness = 0;

	for(int i = 0;i<s.size();i++)
	{
		for(int j = 0;j<s[i].size();j++)
		{
			int opcompletionTime = scheduleStartTimes[s[i][j]] + processingTime[s[i][j]];
			double ea = (opcompletionTime<= dueDate[s[i][j]] ? (dueDate[s[i][j]] - opcompletionTime)*earliness[s[i][j]]  :  0);
			double ta = (opcompletionTime>= dueDate[s[i][j]] ? (opcompletionTime- dueDate[s[i][j]])*tardiness[s[i][j]]  :  0);
			totalearliness+=ea;
			totaltardiness+=ta;
			total += (ea + ta);
		}
	}

	return {total,totalearliness,totaltardiness};

}



// /////////////////////////////////////////////////////////////////////////////////////////

Schedule
JIT_JSS::GifflerThompson(matriz instance)
{
	//instance = processing order
	
	matriz sequence;
	sequence.resize(nMachines);
	vector<int>time(nOperations + 1);

	set<int>G;
	vector<int>machineStart(nMachines,0),jobStart(nJobs,0);

	//add all the schedulable operations, i.e., the first operation of each job and
	//initialize the earliest start and completion times for the operations in G
	
	for(int i = 0;i<nJobs;i++)
	{
		int op = instance[i][0];
		time[op] = 0;
		G.insert(op);
	}

	while(!G.empty())
	{
		//Find the earliest completion time C∗ and its associated machine M∗ among all the operations in G
		int Cstar = INT_MAX;
		int Mstar;

		for(int op : G)
		{
			if(time[op] + processingTime[op] < Cstar)
			{
				Cstar = time[op] + processingTime[op];
				Mstar = machine[op];
			}
		}
		//let G0 ⊆ G denote the operations processed on machine M∗;
		vector<int>G0;
		for(int op : G)
		{
			if(machine[op] == Mstar)G0.push_back(op);
		}
		//G1 = {Op ∈ G0|op.startTime < C∗} (building the conflict set);
		vector<int>G1;
		for(int op : G0)
		{
			if(time[op] < Cstar)G1.push_back(op);
		}	

		//Select op ∈ G1 to be scheduled next (by using earliest due date dispatching rule)
		int selected = G1[0];
		for(int i = 1;i<G1.size();i++)
		{
			if(dueDate[G1[i]] < dueDate[selected])
			{
				selected = G1[i];
			}
		}

		//schedule selected op
		sequence[Mstar].push_back(selected);

		time[selected] = max(machineStart[Mstar],jobStart[job[selected]]);

		machineStart[Mstar] = time[selected] + processingTime[selected];
		jobStart[job[selected]] = time[selected] + processingTime[selected];

		//Remove op from G and add its job successor (if any) to G;
		G.erase(selected);

		int jobSucessor = FindJobSucessor(selected);

		if(jobSucessor > 0)G.insert(jobSucessor);

		//update start times for operations in G
		for(int op : G)
		{
			time[op] = (time[op] < max(machineStart[machine[op]],jobStart[job[op]]) ? max(machineStart[machine[op]],jobStart[job[op]]) : 0);
		}
	}

	vector<int>starttime = calcStartTimes(sequence,1);

	return {sequence,starttime};
	
}

/////////////////////////////////////////////////////////////////////////////////////////


Schedule
JIT_JSS::EarliestDeadlineFirst(matriz instance)
{
	matriz schedule;
	vector<int>startTime;
	schedule.resize(nMachines);
	startTime.resize(nOperations + 1);

	vector<int>ready;
	vector<int>machineStart(nMachines,0),jobStart(nJobs,0);
	for(int i = 0;i<nJobs;i++)
	{
		ready.push_back(instance[i][0]);
	}

	while(!ready.empty())
	{
		int selected = ready[0];
		int selectedId = 0;
		for(int i = 1;i<ready.size();i++)
		{
			if(dueDate[ready[i]] < dueDate[selected])
			{
				selected = ready[i];
				selectedId = i;
			}
		}

		ready[selectedId] = ready[ready.size() - 1];
		ready.pop_back();

		schedule[machine[selected]].push_back(selected);

		startTime[selected] = max(machineStart[machine[selected]],jobStart[job[selected]]);

		machineStart[machine[selected]] = startTime[selected] + processingTime[selected];
		jobStart[job[selected]] = startTime[selected] + processingTime[selected];

		int selectedJob = job[selected];
		for(int i = 0;i<jobOps[selectedJob].size() - 1;i++)
		{
			if(selected == jobOps[selectedJob][i])
			{
				int next = jobOps[selectedJob][i + 1];
				ready.push_back(next);
			}
		}

	}

	return {schedule,startTime};

}

/////////////////////////////////////////////////////////////////////////////////////////

bool checkIntersection(int start1,int end1,int start2,int end2)
{
	if (start1 == start2) {
		return true;
	}
	else if (start1 < start2) {
		if (end1 > start2) {
			return true;
		}
	}
	else {//start2 < start1
		if (end2 > start1) {
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

void
JIT_JSS::printOperation(int op)
{
	cout<<"O"<<job[op]+1<<machine[op];
}

/////////////////////////////////////////////////////////////////////////////////////////

vector<int>
JIT_JSS::calcStartTimes(matriz sequence,int z)
{
	vector<int>time(nOperations + 1);
	vector<int>minStartJob(nJobs,0),minStartMachine(nMachines,0);
	
	int sinkID = nOperations + 1;
	vector< vector<int> > g = createDAG(sequence,sinkID);
	// cout<<"dag created\n";
	// cout<<"DAG:\n";
	// for(int i = 0;i<g.size();i++)
	// {
	// 	if(i == 0 || i == sinkID)cout<<i<<":";
	// 	else cout<<"O"<<job[i]+1<<machine[i]<<":";
	// 	for(int j = 0;j<g[i].size();j++)
	// 	{
	// 		if(g[i][j] == 0 || g[i][j] == sinkID)cout<<g[i][j]<<' ';
	// 		else cout<<"O"<<job[g[i][j]]+1<<machine[g[i][j]]<<" ";
	// 	}

	// 	cout<<endl;
	// }
	
	

	vector<bool>visited(nOperations + 2,0);
	std::stack<int> stk;
	//cout<<"before finding topological sort\n";
	for(int i = 0;i<nOperations + 2;i++)
	{
		if(!visited[i])TopologicalSort(i,visited,stk,g);
	}
	// cout<<"after founding\n";


	// if(z)cout<<"TOPOLOGICAL SORT:\n";
	
	while(!stk.empty())
	{
		int op = stk.top();
		stk.pop();
		// if(z && op > 0 && op != sinkID){printOperation(op);cout<<" ";}
		// else if(z) cout<<op<<' ';
		if(op <= 0 || op == sinkID)continue;
		int jobOp = job[op];
		int machineOp = machine[op];
		time[op] = max(minStartJob[jobOp],minStartMachine[machineOp]);
		minStartJob[jobOp] = time[op] + processingTime[op];
		minStartMachine[machineOp] = time[op] + processingTime[op];
		// if(z)cout<<"new start time:"<<time[op]<<endl;
		
	}
	// if(z)cout<<endl;
	return time;
	
}


/////////////////////////////////////////////////////////////////////////////////////////

bool
JIT_JSS::isScheduleCorrect(matriz s,vector<int> neighborStartTime)
{
	for(int op1 = 1;op1<= nOperations;op1++)
	{
		for(int op2 = 1;op2<=nOperations;op2++)
		{
			//|| machine[op1] == machine[op2]
			if(op1 != op2 && (job[op1] == job[op2]))
			{
				if(checkIntersection(neighborStartTime[op1],neighborStartTime[op1] + processingTime[op1],neighborStartTime[op2],neighborStartTime[op2] + processingTime[op2]))
				{
					return false;
				}
			}
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool
JIT_JSS::isSequenceCorrect(matriz schedule,vector<int>scheduleStartTime)
{
	for(int i = 0;i<schedule.size();i++)
	{
		for(int j = 1;j<schedule[i].size();j++)
		{
			int op1starttime = scheduleStartTime[schedule[i][j - 1]];
			int op1proctime = processingTime[schedule[i][j - 1]];
			int op2starttime = scheduleStartTime[schedule[i][j]];
			if(op2starttime < op1starttime + op1proctime)return false;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool
JIT_JSS::isProcessingOrderKept(matriz schedule,vector<int>scheduleStartTime)
{
	matriz scheduleOrder(nJobs);
	for(int i = 0;i<schedule.size();i++)
	{
		for(int j = 0;j<schedule[i].size();j++)
		{
			scheduleOrder[job[schedule[i][j]]].push_back(schedule[i][j]);
		}
	}

	for(int i = 0;i<scheduleOrder.size();i++)
	{
		sort(scheduleOrder[i].begin(),scheduleOrder[i].end(),[&scheduleStartTime](int &op1, int &op2){
			return scheduleStartTime[op1] < scheduleStartTime[op2];
		});
	}


	for(int i = 0;i<scheduleOrder.size();i++)
	{
		for(int j = 0;j<scheduleOrder[i].size();j++)
		{
			if(scheduleOrder[i][j] != processingOrder[i][j])return false;
		}
	}

	return true;

}

/////////////////////////////////////////////////////////////////////////////////////////

// vector<Neighbor>
// JIT_JSS::Swap(matriz schedule,vector<int> startTimes)
// {

// 		//{schedule,startTime}
// 		vector<Neighbor> neighborhood;

// 		for(int i = 0;i<schedule.size();i++)
// 		{
// 			for(int j = 1;j<schedule[i].size();j++)
// 			{
// 				matriz newSequence = schedule;
// 				vector<int> neighborStartTime = startTimes;
// 				int a = schedule[i][j-1];
// 				int b = schedule[i][j];
				
// 				int Bcompletiontime = neighborStartTime[b] + processingTime[b];

// 				neighborStartTime[b] = neighborStartTime[a];
// 				neighborStartTime[a] = Bcompletiontime - processingTime[a];

// 				newSequence[i][j - 1] = b;
// 				newSequence[i][j] = a;
				
// 				if(isScheduleCorrect(newSequence,neighborStartTime)
// 					&& isProcessingOrderKept(newSequence,neighborStartTime)
// 					&& isSequenceCorrect(newSequence,neighborStartTime))
// 				{
// 					Neighbor neighbor;
// 					neighbor.first.first = newSequence;
// 					neighbor.first.second = neighborStartTime;
// 					neighborhood.push_back(neighbor);
// 				}
// 			}
// 		}

// 		return neighborhood;
// }

/////////////////////////////////////////////////////////////////////////////////////////

void
JIT_JSS::PrintSequence(matriz sequence)
{
	for(int i = 0;i<sequence.size();i++)
	{
		cout<<i<<":";
		for(int j = 0;j<sequence[i].size();j++)
		{
			cout<<"O"<<job[sequence[i][j]]+1<<machine[sequence[i][j]]<<" ";
		}
		cout<<endl;
	}
	cout<<endl;

}

void
JIT_JSS::PrintSchedule(matriz sequence,vector<int>time)
{
	for(int i = 0;i<sequence.size();i++)
	{
		cout<<i<<":";
		for(int j = 0;j<sequence[i].size();j++)
		{
			cout<<"O"<<job[sequence[i][j]]+1<<machine[sequence[i][j]]<<"( "<< time[sequence[i][j]] <<","<<time[sequence[i][j]] + processingTime[sequence[i][j]] << ") ";
		}
		cout<<endl;
	}
	cout<<endl;
}


/////////////////////////////////////////////////////////////////////////////////////////

pair<Schedule,int>
JIT_JSS::LocalSearch(matriz instance, int MAX_ITER,int flag,string nflag, Schedule a)
{

	Schedule best = EarliestDeadlineFirst(instance);
	//clock_t inicioInicial = clock();
	//Schedule best = GifflerThompson(instance);
    //double finish = static_cast<double>(clock() - inicioInicial) / CLOCKS_PER_SEC;
	//cout<<"INITIAL SOLUTION:\n";
	//PrintSchedule(best.first,best.second);
	//best = DelayOperations(best);
	//best.second = Calc(best.first,best.second);
	if(flag)
	{
		best = a;
	}
	int iter = 1;

	auto start = std::chrono::steady_clock::now();
	//"Elapsed(ms)=" << since(start).count() = how much time has elapsed since 'start' have been initialized
	// 'since' function is defined in this file


	//run the search for a fixed number of time(MAX_ITER)
	while((since(start).count()) < MAX_ITER)
	{
		//cout<<"NEW ITERATION\n";
		vector<Neighbor> neighborhood;
		if(nflag == "N7")
		{
			neighborhood = N7_2(best.first);
		}else if(nflag == "N5")
		{
			neighborhood = N5(best.first,best.second);
		}else if(nflag == "SWAP")
		{
			neighborhood = Swap2(best.first);
			//neighborhood = SwapTest(best.first);
		}else
		{
			cout<<"NEIGHBORHOOD STRUCTURE NOT DEFINED\n";
			abort();	
		}	

		bool improvement = 0;

		for(Neighbor neighbor : neighborhood)
		{
			neighbor.first.second = Calc2(neighbor.first.first,neighbor.first.second);
			vector<double> neighborPenalties = SchedulePenalties(neighbor.first.first,neighbor.first.second);
			vector<double> bestPenalties = SchedulePenalties(best.first,best.second);
			if(neighborPenalties[0] < bestPenalties[0])
			{
				improvement = 1;
				best = neighbor.first;
			}
		}

		if(!improvement)
		{
			//local max
			break;
		}
		iter++;
		//best = DelayOperations(best);
	}

	long long end = since(start).count();

	return {best,iter};	
}

/////////////////////////////////////////////////////////////////////////////////////////

vector< vector<int> >
JIT_JSS::createDAG(matriz sequence,int sinkID)
{
	vector< vector<int> >g(nOperations + 2);

	//connect source node to first op of each job
	for(int i = 0;i<processingOrder.size();i++)
	{
		g[0].push_back(processingOrder[i][0]);
	}

	//adding directed (conjunctive) arcs
	//connect operation to its job succesor
	//if its the last op in job, connect it to sink node
	for(int i = 0;i<processingOrder.size();i++)
	{
		for(int j = 0;j<processingOrder[i].size();j++)
		{
			if(j < processingOrder[i].size() - 1)
			{
				g[processingOrder[i][j]].push_back(processingOrder[i][j + 1]);
			}else
			{
				//last op in job
				g[processingOrder[i][j]].push_back(sinkID);
			}
		}
	}

	//adding undirected (disjunctive) arcs
	//connect operation to all its machine sucessors

	for(int i = 0;i<sequence.size();i++)
	{
		for(int j = 0;j<sequence[i].size();j++)
		{
			int current = sequence[i][j];
			for(int k = j + 1;k<sequence[i].size();k++)
			{
				g[current].push_back(sequence[i][k]);
			}
		}
	}

	return g;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool dfs(vector<vector<int> >&g,int u,vector<bool>&visitado,vector<bool>&recstack)
{
	if(!visitado[u])
	{
		visitado[u] = 1;
		recstack[u] = 1;
		for(int i = 0;i<g[u].size();i++)
		{
			if(!visitado[g[u][i]] && dfs(g,g[u][i],visitado,recstack))
			{
				return true;
			}else if(recstack[g[u][i]])
			{
				return true;
			}
		}
	}
	recstack[u] = 0;
	return false;
}

bool isCiclico(vector<vector<int> >&g,int sinkID,int nOperations)
{
	vector<bool>visitado(nOperations + 2,0);
	vector<bool>recstack(nOperations + 2,0);

	for(int i = 1;i<=sinkID;i++)
	{
		if(!visitado[i] && dfs(g,i,visitado,recstack))
		{
			return true;
		}
	}
	return false;

}

vector<int> JIT_JSS::CriticalPath(matriz schedule){

	//g[0] : source node
	//g[nOperations + 1] = sink node
	// cout<<"dentro da funcao de criacao do critical path\n";
	int sinkID = nOperations + 1;
	vector< vector<int> >g = createDAG(schedule,sinkID);
	// cout<<"DAG criado\n";
	// cout<<"EH CICLICO? = ";
	// cout<<isCiclico(g,sinkID,nOperations + 2)<<endl;
	// //calculating longest path from source (0) to sink (nOperations + 1)

	vector<bool>visited(nOperations + 2,0);
	std::stack<int> stack;

	for(int i = 0;i<nOperations + 2;i++)
	{
		if(!visited[i])TopologicalSort(i,visited,stack,g);
	}
	// cout<<"topological sort encontrado\n";
	vector<int>dist(nOperations + 2,INT_MIN);
	dist[0] = 0;

	vector<int>parent(nOperations + 2);

	parent[0] = -1;

	
	while(!stack.empty())
	{
		int u = stack.top();
		// cout<<u<<" ";
		stack.pop();
		// cout<<"olhando operacao ";
		// printOperation(u);
		// cout<<" "<<u<<" "<<endl;
		if(dist[u] != INT_MIN)
		{
			// if(u == 201)
			// {
			// 	cout<<"olhando o sink\n";
			// 	cout<<"operacoes do sink:";
			// 	for(int i = 0;i<g[u].size();i++)
			// 	{
			// 		printOperation(g[u][i]);
			// 		cout<<" ";
			// 	}
			// 	cout<<endl;
			// 	cout<<"dist ";
			// 	printOperation(u);
			// 	cout<<":";
			// 	cout<<dist[u]<<endl;
			// }
			for(int i = 0;i<g[u].size();i++)
			{
				int v = g[u][i];
				
				if(dist[v] < dist[u] + processingTime[v])
				{
					dist[v] = dist[u] + processingTime[v];
					parent[v] = u;
				}
			}
		}
	}
	// cout<<"finalizou o processamento seguindo o topological sort\n";

	//cout<<endl;
	vector<int>criticalPath;

	//retrieve path
	int current = sinkID;

	//cout<<"sequencia de processamento:\n";
	while(current != -1)
	{
		//cout<<"current:";
		// printOperation(current);
		// cout<<endl;
		criticalPath.push_back(current);
		current = parent[current];
	}

	reverse(criticalPath.begin(),criticalPath.end());

	int makespan = 0;
	for(int i : criticalPath)
	{
		if(i != 0 && i != sinkID)makespan+=processingTime[i];
	}

	

	// cout<<"G:\n";
	// for(int i = 0;i<g.size();i++)
	// {
	// 	if(i != 0 && i != sinkID)cout<<i<<"(O"<<job[i]+1<<machine[i]<<"):\n";
	// 	else cout<<i<<":\n";
	// 	for(int j = 0;j<g[i].size();j++)
	// 	{
	// 		if(g[i][j] == sinkID)
	// 		{
	// 			cout<<"sink ";
	// 		}else cout<<"O"<<job[g[i][j]] + 1<<machine[g[i][j]]<<" ";
	// 	}
	// 	cout<<endl;
	// }	
	// cout<<"critical path:\n";
	// for(int i : criticalPath)
	// {
	// 	if(i != 0 && i != sinkID)cout<<i<<"(O"<<job[i] + 1<<machine[i]<<") ";
	// 	else if(i == 0)cout<<"source ";
	// 	else cout<<"sink ";
	// }
	// cout<<endl<<"makespan:"<<makespan<<endl;

	//take out source and sink nodes
	// cout<<"antes de remover os dummies\n";
	criticalPath.erase(criticalPath.begin());
	criticalPath.pop_back();
	// cout<<"apos remover os dummies\n";

	// cout<<"critical path:\n";
	// for(int i : criticalPath)
	// {
	// 	if(i != 0 && i != sinkID)cout<<i<<"(O"<<job[i] + 1<<machine[i]<<") ";
	// 	else if(i == 0)cout<<"source ";
	// 	else cout<<"sink ";
	// }
	// cout<<endl<<"makespan:"<<makespan<<endl;
	//cout<<"achou caminho critico.\n";
	return criticalPath;

}

/////////////////////////////////////////////////////////////////////////////////////////

void
JIT_JSS::TopologicalSort(int v,vector<bool>&visited,std::stack<int>&Stack,vector< vector<int> >&g)
{
	visited[v] = 1;
	for(int i = 0;i<g[v].size();i++)
	{
		int u = g[v][i];
		if(!visited[u])TopologicalSort(u,visited,Stack,g);
	}

	Stack.push(v);

}

/////////////////////////////////////////////////////////////////////////////////////////

vector< vector<int> >
JIT_JSS::CriticalBlocks(vector<int> criticalPath)
{
	vector< vector<int> >blocks;

	vector<int>ind(criticalPath.size());
	int count = 1;
	ind[0] = count;
	for(int i = 1;i<criticalPath.size();i++)
	{
		if(machine[criticalPath[i]] == machine[criticalPath[i - 1]])
		{
			ind[i] = count;
		}else
		{
			ind[i] = ++count;
		}
	}

	int current = 1;
	vector<int>block;
	for(int i = 0;i<criticalPath.size();i++)
	{
		if(ind[i] == current)
		{
			block.push_back(criticalPath[i]);
		}else
		{
			blocks.push_back(block);
			block.clear();
			current++;
			block.push_back(criticalPath[i]);
		}

		if(i == criticalPath.size() - 1)
		{
			blocks.push_back(block);
		}
	}

	return blocks;
}

/////////////////////////////////////////////////////////////////////////////////////////




//block_op:op to be inserted in the beginning of the block
pair<matriz,vector<int> >
JIT_JSS::insertion1(matriz schedule, int block_op,int block_first_op,vector<int>startTimes,int z)
{
	
	//new neighbor
	matriz neighbor = schedule;
	//new start
	vector<int>nstart = startTimes;
	//machine where block_op is processed
	int opMachine = machine[block_op];
	// if(z){
	// cout<<"original opMachine sequence:\n";
	// for(int j = 0;j<schedule[opMachine].size();j++)
	// {
	// 	cout<<"O"<<job[schedule[opMachine][j]]+1<<machine[schedule[opMachine][j]]<<" ";
	// }
	// cout<<endl;
	// }

	//new sequence for machine opMachine
	vector<int>nMachine;
	int current = 0,last = 0,idx = 0;
	//add operations until block_first_op
	while(schedule[opMachine][current] != block_first_op)
	{
		nMachine.push_back(schedule[opMachine][current]);
		last = schedule[opMachine][current];
		current++;
		idx++;
	}

	//inserts block_op in the sequence before block_first_op;
	nMachine.push_back(block_op);
	nstart[block_op] = nstart[last] + processingTime[last];

	last = block_op;


	//put other operations in the sequence while shifting their start times
	for(current;current < schedule[opMachine].size();current++)
	{
		if(schedule[opMachine][current] == block_op)continue;
		nMachine.push_back(schedule[opMachine][current]);
		//start = completion time of prev op
		nstart[schedule[opMachine][current]] = nstart[last] + processingTime[last];
		last = schedule[opMachine][current];
	}

	//change opMachine sequence in the neighbor
	neighbor[opMachine] = nMachine;
	
	// if(z){
	// cout<<"result:\n";
	// for(int j = 0;j<neighbor[opMachine].size();j++)
	// {
	// 	int opstart = nstart[neighbor[opMachine][j]];
	// 	int opproctime = processingTime[neighbor[opMachine][j]];
	// 	cout<<"O"<<job[neighbor[opMachine][j]]+1<<machine[ neighbor[opMachine][j]]<<"("<<opstart<<","<<opstart + opproctime<<") ";
	// }
	// cout<<endl;
	// }

	return {neighbor,nstart};

}

/////////////////////////////////////////////////////////////////////////////////////////

pair<matriz,vector<int> >
 JIT_JSS::insertion2(matriz schedule,int block_op,int last_op,vector<int>startTimes)
 {
	//cout<<"putting O"<<job[block_op] + 1<<machine[block_op]<<" in the END of the block:\n";
	matriz neighbor = schedule;
	vector<int>nstart = startTimes;
	//machine where block_op is processed
	int iMachine = machine[block_op];
	// cout<<"iMachine sequence:\n";
	// for(int j = 0;j<schedule[iMachine].size();j++)
	// {
	// 	cout<<"O"<<job[schedule[iMachine][j]]+1<<machine[schedule[iMachine][j]]<<" ";
	// }
	// cout<<endl;
	
	//new sequence for machine iMachine
	vector<int>nMachine;
	int current = 0,last = 0,idx = 0;
	while(schedule[iMachine][current] != block_op)
	{
		nMachine.push_back(schedule[iMachine][current]);
		last = schedule[iMachine][current];
		current++;
		idx++;
	}

	current++;
	//keep putting other operations in the sequence while shifting their start times
	for(current;current < schedule[iMachine].size();current++)
	{
		if(schedule[iMachine][current] == block_op)continue;
		nMachine.push_back(schedule[iMachine][current]);
		//start = completion time of prev op
		nstart[schedule[iMachine][current]] = nstart[last] + processingTime[last];
		last = schedule[iMachine][current];
		if(last == last_op)
		{
			//putting block_op after last op of the block
			nMachine.push_back(block_op);
			nstart[block_op] = nstart[last] + processingTime[last];
			last = block_op;
		}
	}


	//change iMachine sequence in the neighbor
	neighbor[iMachine] = nMachine;
	

return {neighbor,nstart};

 }

/////////////////////////////////////////////////////////////////////////////////////////

//block_op:first operation of the block
//prev_op:operation in wich block_op will be placed after
pair<matriz,vector<int> >
JIT_JSS::insertion3(matriz schedule,int prev_op,int block_op,vector<int> scheduleStartTimes){

	//cout<<"inserting O"<<job[block_op] + 1<<machine[block_op]<<" after O"<<job[prev_op] + 1<<machine[prev_op]<<endl;
	matriz neighbor = schedule;
	vector<int>nstart = scheduleStartTimes;

	int iMachine = machine[block_op];

	// cout<<"original sequence\n";
	// for(int i : schedule[iMachine])cout<<"O"<<job[i] + 1<<machine[i]<<"("<<nstart[i]<<","<<nstart[i] + processingTime[i]<<") ";
	// cout<<endl;

	//new sequence for iMachine;
	vector<int>nMachine;

	//put operations in nMachine until block_op(first operation of block);

	int current = 0;
	int last = 0;
	while(schedule[iMachine][current] != block_op)
	{
		nMachine.push_back(schedule[iMachine][current]);
		last = schedule[iMachine][current];
		current++;
	}

	current++; 

	//keep inserting other operations in the sequence while shifting their start times
	for(current;current < schedule[iMachine].size();current++)
	{
		//if(schedule[iMachine][current] == block_op)continue;
		nMachine.push_back(schedule[iMachine][current]);
		//start = completion time of prev op
		nstart[schedule[iMachine][current]] = nstart[last] + processingTime[last];
		last = schedule[iMachine][current];

		//insert the first operation of block after prev_op
		if(last == prev_op)
		{
			nMachine.push_back(block_op);
			nstart[block_op] = nstart[last] + processingTime[last];
			last = block_op;
		}
	}

	// cout<<"sequence after change:\n";
	// for(int j = 0;j<nMachine.size();j++)
	// {
	// 	cout<<"O"<<job[nMachine[j]]+1<<machine[nMachine[j]]<<"("<<nstart[nMachine[j]]<<","<<nstart[nMachine[j]] + processingTime[nMachine[j]]<<") ";
	// }
	// cout<<endl;

	//change sequence
	neighbor[iMachine] = nMachine;

	return {neighbor,nstart};

}

/////////////////////////////////////////////////////////////////////////////////////////

pair<matriz,vector<int> >
JIT_JSS::insertion4(matriz schedule,int next_op,int block_op,vector<int> scheduleStartTimes)
{
	matriz neighbor = schedule;
	vector<int> nstart = scheduleStartTimes;

	int iMachine = machine[block_op];
	
	//new sequence for iMachine;
	vector<int>nMachine;

	//put operations in nMachine until next_op;
	int current = 0;
	int last = 0;
	while(schedule[iMachine][current] != next_op)
	{
		nMachine.push_back(schedule[iMachine][current]);
		last = schedule[iMachine][current];
		current++;
	}

	nMachine.push_back(block_op);
	nstart[block_op] = nstart[last] + processingTime[last];

	last = block_op;


	//put other operations in the sequence while shifting their start times
	for(current;current < schedule[iMachine].size();current++)
	{
		if(schedule[iMachine][current] == block_op)continue;
		nMachine.push_back(schedule[iMachine][current]);
		//start = completion time of prev op
		nstart[schedule[iMachine][current]] = nstart[last] + processingTime[last];
		last = schedule[iMachine][current];
	}

	//change iMachine sequence in the neighbor
	neighbor[iMachine] = nMachine;

	//change sequence
	neighbor[iMachine] = nMachine;

	return {neighbor,nstart};



}

/////////////////////////////////////////////////////////////////////////////////////////

vector<Neighbor>
JIT_JSS::N7_2(matriz sequence)
{
	vector<int> criticalPath = CriticalPath(sequence);
	// cout<<"critical path gerado\n";
	vector< vector<int> > blocks = CriticalBlocks(criticalPath);
	// cout<<"blocks gerados\n";
	vector<Neighbor> neighborhood;

	for(vector<int> block : blocks)
	{
		if(block.size() < 2)continue;
		// cout<<"em um block:\n";
		// // cout<<"Current BLOCK:";
		// for(int i : block)
		// {
		// // 	printOperation(i);cout<<' ';
		// }
		// // cout<<endl;
		for(int i = 1;i< block.size() - 1;i++)
		{
			//inserting inner operations at the beginning of the block
			Neighbor neighbor;

			neighbor.first.first = InsertOperationBefore(sequence,block[i],block[0]);
			neighbor.first.second = calcStartTimes(neighbor.first.first,1);
			neighbor.second = {block[0],block[i]};
			if(isScheduleCorrect(neighbor.first.first,neighbor.first.second)
			&& isProcessingOrderKept(neighbor.first.first,neighbor.first.second)
			&& isSequenceCorrect(neighbor.first.first,neighbor.first.second))
				neighborhood.push_back(neighbor);
			
			//inserting inner operations at the end of the block 
			neighbor.first.first = InsertOperationAfter(sequence,block[i],block[block.size() - 1]);
			neighbor.first.second = calcStartTimes(neighbor.first.first,1);
			neighbor.second = {block[i],block[block.size() - 1]};
			if(isScheduleCorrect(neighbor.first.first,neighbor.first.second)
			&& isProcessingOrderKept(neighbor.first.first,neighbor.first.second)
			&& isSequenceCorrect(neighbor.first.first,neighbor.first.second))
				neighborhood.push_back(neighbor);
			
			//inserting first op of the block at the middle of the block
			neighbor.first.first = InsertOperationAfter(sequence,block[0],block[i]);
			neighbor.first.second = calcStartTimes(neighbor.first.first,1);
			neighbor.second = {block[i],block[0]};
			if(isScheduleCorrect(neighbor.first.first,neighbor.first.second)
			&& isProcessingOrderKept(neighbor.first.first,neighbor.first.second)
			&& isSequenceCorrect(neighbor.first.first,neighbor.first.second))
				neighborhood.push_back(neighbor);
			

			//inserting last op of the block at the middle of the block
			neighbor.first.first = InsertOperationBefore(sequence,block[block.size() - 1],block[i]);
			neighbor.first.second = calcStartTimes(neighbor.first.first,1);
			neighbor.second = {block[block.size() - 1],block[i]};
			if(isScheduleCorrect(neighbor.first.first,neighbor.first.second)
			&& isProcessingOrderKept(neighbor.first.first,neighbor.first.second)
			&& isSequenceCorrect(neighbor.first.first,neighbor.first.second))
				neighborhood.push_back(neighbor);
			
		}
	}
	// cout<<"fim da geracao, voltando pra tabu\n";
	return neighborhood;
}

/////////////////////////////////////////////////////////////////////////////////////////

vector<Neighbor>
JIT_JSS::N7(matriz schedule,vector<int> startTimes)
{
	// cout<<"inicio n7\n";
	vector<int> criticalPath = CriticalPath(schedule);
	vector< vector<int> > blocks = CriticalBlocks(criticalPath);

	vector<Neighbor> neighborhood;

	for(vector<int> block : blocks)
	{
		if(block.size() <= 2)continue;

		// cout<<"current block:\n";
		// for(int i : block)cout<<"O"<<job[i] + 1<<machine[i]<<"("<<startTimes[i]<<","<<startTimes[i] + processingTime[i]<<") ";
		// cout<<endl;

		//inserting operation from the middle of the block in the first and last pos in the block
		for(int i = 1;i< block.size() - 1;i++)
		{
			//inserting a operation in the beginning of the block
			Schedule newSchedule = insertion1(schedule,block[i],block[0],startTimes,0);
			Neighbor neighbor = {newSchedule, {block[i],block[0]}};
			if(isScheduleCorrect(neighbor.first.first,neighbor.first.second)
			&& isProcessingOrderKept(neighbor.first.first,neighbor.first.second)
			&& isSequenceCorrect(neighbor.first.first,neighbor.first.second))
			{
				neighborhood.push_back(neighbor);
			}
			
			// inserting a operation in the end of the block;
			newSchedule = insertion2(schedule,block[i],block[block.size() - 1],startTimes);
			neighbor = {newSchedule,{block[i],block[block.size() - 1]}};
			if(isScheduleCorrect(neighbor.first.first,neighbor.first.second)
			&& isProcessingOrderKept(neighbor.first.first,neighbor.first.second)
			&& isSequenceCorrect(neighbor.first.first,neighbor.first.second))
			{
				neighborhood.push_back(neighbor);
			}
		}

		//inserting first and last operations inside the block
		for(int  i = 1;i<block.size() - 1;i++)
		{
			//inserting first op of block in the middle of the block
			Schedule newSchedule = insertion3(schedule,block[i],block[0],startTimes);
			Neighbor neighbor = {newSchedule,{block[i],block[0]}};
			if(isScheduleCorrect(neighbor.first.first,neighbor.first.second)
			&& isProcessingOrderKept(neighbor.first.first,neighbor.first.second)
			&& isSequenceCorrect(neighbor.first.first,neighbor.first.second))
			{
				neighborhood.push_back(neighbor);
			}

		// 	//inserting last op of block in the middle of the block
			newSchedule = insertion4(schedule,block[i],block[block.size() - 1],startTimes);
			neighbor = {newSchedule,{block[i],block[block.size() - 1]}};
			if(isScheduleCorrect(neighbor.first.first,neighbor.first.second) && isProcessingOrderKept(neighbor.first.first,neighbor.first.second))
			{
				neighborhood.push_back(neighbor);
			}
		}

		// cout<<endl<<endl;


		

	}

	// cout<<"fim n7\n";
	return neighborhood;

}

/////////////////////////////////////////////////////////////////////////////////////////

vector<Neighbor>
JIT_JSS::Swap(matriz schedule,vector<int> startTimes){

	vector<Neighbor>neighborhood;

	for(int i = 0;i<schedule.size();i++)
	{
		for(int j = 1;j<schedule[i].size();j++)
		{
			Neighbor neighbor;
			int op1 = schedule[i][j-1];
			int op2 = schedule[i][j];
			pair<vector<int>,vector<int> > a = SwapAdj(schedule[i],startTimes,op1,op2);
			neighbor.first.first = schedule;
			neighbor.first.first[i] = a.first;
			neighbor.first.second = a.second;
			neighbor.second = {op1,op2};
			if(isScheduleCorrect(neighbor.first.first,neighbor.first.second)
			&& isProcessingOrderKept(neighbor.first.first,neighbor.first.second)
			&& isSequenceCorrect(neighbor.first.first,neighbor.first.second)
			)
			{
				neighborhood.push_back(neighbor);
			}
		}
	}

	return neighborhood;
}

/////////////////////////////////////////////////////////////////////////////////////////

matriz
JIT_JSS::SwapOperations(matriz sequence,int op1,int op2,int op1idx,int op2idx)
{
	int op1job = job[op1];
	int op1machine = machine[op1];
	int op2job = job[op2];
	int op2machine = machine[op2];
	// cout<<"Swapping ";
	// printOperation(op1);
	// cout<<" and ";
	// printOperation(op2);
	// cout<<endl;

	sequence[op1machine][op1idx] = op2;
	sequence[op2machine][op2idx] = op1;

	return sequence;
}

/////////////////////////////////////////////////////////////////////////////////////////

matriz
JIT_JSS::InsertOperationBefore(matriz sequence,int op1,int op2)
{
	//op1 will be inserted before op2

	//since the insertions are in a block, all the ops are in the same machine
	int machineOp = machine[op1];
	
	//new sequence of operations in machineOp
	vector<int>v;

	int idx;

	//put operations in v until op2 is found
	for(int i = 0;i<sequence[machineOp].size();i++)
	{
		if(sequence[machineOp][i] == op2)
		{
			//insert op1
			v.push_back(op1);
			idx = i;
			break;
		}else
		{
			v.push_back(sequence[machineOp][i]);
		}
	}

	//continue inserting the rest of the operations
	for(int i = idx;i<sequence[machineOp].size();i++){
		if(sequence[machineOp][i] != op1)
		{
			v.push_back(sequence[machineOp][i]);
		}
	}

	sequence[machineOp] = v;
	return sequence;

}

/////////////////////////////////////////////////////////////////////////////////////////

matriz
JIT_JSS::InsertOperationAfter(matriz sequence,int op1,int op2){

	//op1 will be inserted after op2
	//since the insertions happen in a block, all the ops are in the same machine
	int machineOp = machine[op1];
	
	//new sequence of operations in machineOp
	vector<int>v;

	int idx;


	for(int i = 0;i<sequence[machineOp].size();i++)
	{
		int curr = sequence[machineOp][i];
		if(curr != op1 && curr != op2)
		{
			v.push_back(curr);
		}
		else
		{
			if(curr == op1)continue;
			if(curr == op2)
			{
				v.push_back(op2);
				idx = i + 1;
				break;
			}
		}
	}

	v.push_back(op1);
	for(int i = idx;i<sequence[machineOp].size();i++)
	{
		v.push_back(sequence[machineOp][i]);
	}

	sequence[machineOp] = v;
	return sequence;
}


/////////////////////////////////////////////////////////////////////////////////////////

vector<Neighbor>
JIT_JSS::Swap2(matriz sequence){

	vector<Neighbor>neighborhood;
	//int counter = 0;
	for(int i = 0;i<sequence.size();i++)
	{
		for(int j = 1;j<sequence[i].size();j++)
		{
			Neighbor neighbor;
			int op1 = sequence[i][j-1];
			int op2 = sequence[i][j];
			//neighbor.first.first = SwapOperations(sequence,op1,op2,j-1,j);
			neighbor.first.first = sequence;
			neighbor.first.first[i][j-1] = op2;
			neighbor.first.first[i][j] = op1;
			neighbor.second = {op1,op2};

			neighbor.first.second = calcStartTimes(neighbor.first.first,1);
			if(isScheduleCorrect(neighbor.first.first,neighbor.first.second)
			&& isProcessingOrderKept(neighbor.first.first,neighbor.first.second)
			&& isSequenceCorrect(neighbor.first.first,neighbor.first.second)
			)
			{
				neighborhood.push_back(neighbor);
			}
		}
	}

	return neighborhood;
}

/////////////////////////////////////////////////////////////////////////////////////////

pair<Schedule,int>
JIT_JSS::TabuSearch(matriz instance, int MAX_ITER, int tabu_tenure)
{
	// cout<<"max iter:"<<MAX_ITER<<endl;
	// cout<<"tabu tenure:"<<tabu_tenure<<endl;
	Schedule x1 = EarliestDeadlineFirst(instance);
	Schedule x2 = GifflerThompson(instance);
	Schedule x;

	vector<double>x1penalties = SchedulePenalties(x1.first,x1.second);
	vector<double>x2penalties = SchedulePenalties(x2.first,x2.second);

	if(x1penalties[0] <= x2penalties[0])x = x1;
	else x = x2;

	//two ops representing the move in any iteration	
	int op1 = 0,op2 = 0;

	Schedule current = x;

	int tabuList[201][201];

	for(int i = 0;i<201;i++)
		for(int j = 0;j<201;j++)
			tabuList[i][j] = -tabu_tenure;

	int iter = 1;

	// auto start = std::chrono::steady_clock::now();
	clock_t start = clock();

	while(true)
	{
    	double tempo = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;
		if(tempo > MAX_ITER)break;
		if(tempo > 233)break;
		// cout<<since(start).count()<<endl;
		//cout<<iter<<endl;
		//vector<Neighbor> neighborhood = N5_2(current.first);
		//vector<Neighbor> neighborhood = N7_2(current.first);
		// cout<<"vizinhanca gerada, tam:"<<neighborhood.size()<<endl;
		vector<Neighbor> neighborhood = Swap2(current.first);
		
		if(!neighborhood.size())break;


		//finding best candidate among all neighbors
		Neighbor bestCandidate = neighborhood[0];
		for(Neighbor neighbor : neighborhood)
		{
			neighbor.first.second = Calc2(neighbor.first.first,neighbor.first.second);
			int opmove1 = neighbor.second[0];
			int opmove2 = neighbor.second[1];

			vector<double>penaltiesbest = SchedulePenalties(bestCandidate.first.first,bestCandidate.first.second);
			vector<double>penaltiescurr = SchedulePenalties(neighbor.first.first,neighbor.first.second);

			if(tabuList[opmove1][opmove2] + tabu_tenure <= iter
			&& penaltiescurr[0] < penaltiesbest[0])
			{
				// cout<<"move:";
				// printOperation(opmove1);
				// cout<<" -> ";
				// printOperation(opmove2);
				// cout<<endl;
				bestCandidate = neighbor;
				op1 = opmove1;
				op2 = opmove2;
			}
		}



		//check if candidate is better than the best solution found so far
		vector<double>penaltiesbest = SchedulePenalties(x.first,x.second);
		vector<double>penaltiescandidate = SchedulePenalties(bestCandidate.first.first,bestCandidate.first.second);

		if(penaltiescandidate[0] < penaltiesbest[0])x = bestCandidate.first;

		//cout<<"penalidade candidato:"<<penaltiescandidate[0]<<endl;

		//updates current anyway with best candidate found in neighborhood
		current = bestCandidate.first;

		//updates tabu list
		for(int i = 0;i<201;i++)
		{
			for(int j = 0;j<201;j++)
			{
				if(tabuList[i][j] + tabu_tenure <= iter)tabuList[i][j] = -tabu_tenure;
			}
		}

		tabuList[op1][op2] = iter;

		iter++;


	}

	//cout<<"ITERACOES:"<<iter<<endl;



	return {x,iter};

	
}

/////////////////////////////////////////////////////////////////////////////////////////

//returns sequence and start times
pair<vector<int>,vector<int> >
JIT_JSS::SwapAdj(vector<int>sequence,vector<int>stimes,int op1,int op2)
{
	//swap op1 and op2
	for(int i = 0;i<sequence.size();i++)
	{
		if(sequence[i] == op2)
		{
			//cout<<"change made\n";
			int op2completion = stimes[op2] + processingTime[op2];
			stimes[op2] = stimes[op1];
			stimes[op1] = op2completion - processingTime[op1];

			sequence[i] = op1;
			sequence[i - 1] = op2;
			break;
		}
	}

	return {sequence,stimes};
}

/////////////////////////////////////////////////////////////////////////////////////////

vector<Neighbor>
JIT_JSS::N5_2(matriz sequence)
{
	vector<int> criticalPath = CriticalPath(sequence);
	vector< vector<int> > blocks = CriticalBlocks(criticalPath);

	vector<Neighbor> neighborhood;

	for(vector<int> block : blocks)
	{
		if(block.size() < 2)continue;

		//cout<<"BLOCK:";
		// for(int i : block)
		// {
		// 	printOperation(i);cout<<' ';
		// }
		// cout<<endl;

		//swap first two and last two ops in the block
		Neighbor neighbor;
		
		neighbor.first.first = InsertOperationBefore(sequence,block[1],block[0]);
		// cout<<"inserting ";
		// printOperation(block[1]);
		// cout<<" before ";printOperation(block[0]);
		// cout<<endl;cout<<"sequence:\n";PrintSequence(neighbor.first.first);cout<<endl;
		neighbor.first.second = calcStartTimes(neighbor.first.first,1);
		neighbor.second = {block[0],block[1]};
		if(isScheduleCorrect(neighbor.first.first,neighbor.first.second)
			&& isProcessingOrderKept(neighbor.first.first,neighbor.first.second)
			&& isSequenceCorrect(neighbor.first.first,neighbor.first.second))
				neighborhood.push_back(neighbor);

		int n = block.size();
		neighbor.first.first = InsertOperationAfter(sequence,block[n - 2],block[n-1]);
		// cout<<"inserting ";printOperation(block[n-2]);cout<<endl<<" after ";printOperation(block[n-1]);cout<<endl;
		// cout<<"sequence:\n";
		// PrintSequence(neighbor.first.first);
		// cout<<endl;
		neighbor.first.second = calcStartTimes(neighbor.first.first,1);
		neighbor.second = {block[n - 2],block[n - 1]};
		if(isScheduleCorrect(neighbor.first.first,neighbor.first.second)
			&& isProcessingOrderKept(neighbor.first.first,neighbor.first.second)
			&& isSequenceCorrect(neighbor.first.first,neighbor.first.second))
				neighborhood.push_back(neighbor);

	}

	return neighborhood;

}

/////////////////////////////////////////////////////////////////////////////////////////

vector<Neighbor>
JIT_JSS::N5(matriz schedule,vector<int>scheduleStartTimes)
{
	vector<int> criticalPath = CriticalPath(schedule);
	vector< vector<int> > blocks = CriticalBlocks(criticalPath);

	vector<Neighbor> neighborhood;

	// for(int i = 0;i<schedule.size();i++)
	// 	{
	// 		cout<<"M"<<i<<":\n";
	// 		for(int j = 0;j<schedule[i].size();j++)
	// 		{
	// 			cout<<"O"<<job[schedule[i][j]] + 1<<machine[schedule[i][j]]<<"(";
	// 			cout<<scheduleStartTimes[schedule[i][j]]<<","<<scheduleStartTimes[schedule[i][j]] + processingTime[schedule[i][j]]<<") ";
	// 		}
	// 		cout<<endl;
	// 	}

	for(vector<int> block : blocks)
	{
		if(block.size() < 2)continue;
		// cout<<"current block:\n";
		// for(int i : block)cout<<"O"<<job[i] + 1<<machine[i]<<"("<<scheduleStartTimes[i]<<","<<scheduleStartTimes[i] + processingTime[i]<<") ";
		// cout<<endl;

		//swap first two and last two ops in block
		matriz sequence = schedule;
		vector<int>neighbortimes = scheduleStartTimes;

		//swapping first two ops in block
		int iMachine = machine[block[0]];
		//cout<<"iMachine"<<iMachine<<endl;
		pair<vector<int>,vector<int> > newSequence = SwapAdj(sequence[iMachine],neighbortimes,block[0],block[1]);
		sequence[iMachine] = newSequence.first;
		neighbortimes = newSequence.second;
		Neighbor neighbor = {{sequence,neighbortimes},{block[0],block[1]}};

		if(isScheduleCorrect(neighbor.first.first,neighbor.first.second)
		&& isProcessingOrderKept(neighbor.first.first,neighbor.first.second)
		&& isSequenceCorrect(neighbor.first.first,neighbor.first.second))
		{
			neighborhood.push_back(neighbor);
		}

		//swapping last two ops in block
		sequence = schedule;
		neighbortimes = scheduleStartTimes;
		int n = block.size();
		newSequence = SwapAdj(sequence[iMachine],neighbortimes,block[n - 2],block[n - 1]);
		sequence[iMachine] = newSequence.first;
		neighbortimes = newSequence.second;
		
		neighbor = {{sequence,neighbortimes},{block[n - 2],block[n - 1]}};
		if(isScheduleCorrect(neighbor.first.first,neighbor.first.second)
		&& isProcessingOrderKept(neighbor.first.first,neighbor.first.second)
		&& isSequenceCorrect(neighbor.first.first,neighbor.first.second))
		{
			neighborhood.push_back(neighbor);
		}

	}

	return neighborhood;
	
}

Schedule
JIT_JSS::DelayOperations(Schedule s)
{
	//for every operation i, delay i as much as possible
	//without delaying its sucessors and without incurring in tardiness penalties
	matriz schedule = s.first;
	vector<int>startTimes = s.second;

	for(int i = 0;i<schedule.size();i++)
	{
		for(int j = 0;j<schedule[i].size();j++)
		{
			//if operation's completion time is smaller than the operation's due date
			if(startTimes[schedule[i][j]] + processingTime[schedule[i][j]] < dueDate[schedule[i][j]])
			{
				//job of current operation
				int currJob = job[schedule[i][j]];
				
				int jobSucessor = -1,machineSucessor = -1;
				
				//find job sucessor
				for(int k = 0;k<jobOps[currJob].size();k++)
				{
					if(jobOps[currJob][k] == schedule[i][j] && k < jobOps[currJob].size() - 1)
					{
						jobSucessor = jobOps[currJob][k + 1];
						break;
					}					
				}
				//if(jobSucessor > -1)cout<<"job sucessor:O"<<job[jobSucessor]+1<<machine[jobSucessor]<<endl;

				//find machine sucessor
				if(j < schedule[i].size() - 1)
				{
					machineSucessor = schedule[i][j + 1];
				}
				// if(machineSucessor > -1)cout<<"machine sucessor:O"<<job[machineSucessor]+1<<machine[machineSucessor]<<endl;

				int minStart;
				if(jobSucessor > 0 && machineSucessor > 0)
				{
					//get min value between due date, jobSucessor and machineSucessor start times
					minStart = min(min(startTimes[jobSucessor],startTimes[machineSucessor]),dueDate[schedule[i][j]]);
				}else if(jobSucessor > 0)
				{
					minStart = min(startTimes[jobSucessor],dueDate[schedule[i][j]]);
				}else if(machineSucessor > 0)
				{
					minStart = min(startTimes[machineSucessor],dueDate[schedule[i][j]]);
				}
				
				//delay operation
				if(jobSucessor > 0 || machineSucessor > 0){
					startTimes[schedule[i][j]] = minStart - processingTime[schedule[i][j]];
					//cout<<"new start time:"<<startTimes[schedule[i][j]]<<endl;
				}
			}
		}
	}

	return {schedule,startTimes};
}

////////////////////////////////////////////////////////////////////

vector< vector<int> >
JIT_JSS::TransposeDAG(vector< vector<int> >g)
{
	vector< vector<int> >g2(nOperations + 2);

	for(int i = 0;i<g.size();i++)
	{
		for(int j = 0;j<g[i].size();j++)
		{
			g2[g[i][j]].push_back(i);
		}
	}
	
	return g2;
}

////////////////////////////////////////////////////////////////////

vector<int>
JIT_JSS::Calc2(matriz sequence,vector<int> starttime)
{
	//create DAG;
	int sinkID = nOperations + 1;
	vector<vector<int> >g = createDAG(sequence,sinkID);
	//invert g
	vector<vector<int> >g2 = TransposeDAG(g);

	//find topological sort

	//operations sorted by completion time	
	vector<int>sorted;

	for(int i = 1;i<=nOperations;i++)sorted.push_back(i);

	sort(sorted.begin(),sorted.end(),[&](const int &op1,const int &op2){
		return starttime[op1] + processingTime[op1] > starttime[op2] + processingTime[op2];
	});

	vector<int>newStartTime(nOperations + 1);

	//order: sequence of operations ordered by their completion time in descending order
	vector<bool>visited(nOperations + 1,0);
	visited[nOperations + 1] = 1;

	int processed = 1;

	//start with operation with greatest completion time
	int current = 1;
	for(int i = 0;i<sequence.size();i++)
	{
		int op = sequence[i][sequence[i].size() - 1];
		if(starttime[op] + processingTime[op] > starttime[current] + processingTime[current])
		{
			current = op;
		}
	}

	while(processed < nOperations + 2)
	{
		processed++;
		visited[current] = 1;
		if(current == 0 || current == sinkID)continue;
		//cout<<"current: O"<<job[current]+1<<machine[current]<<' ';

		int jobSucessor = FindJobSucessor(current);
		int machineSucessor = FindMachineSucessor(current,sequence);
		//cout<<"job sucessor:"<<jobSucessor<<", machine sucessor:"<<machineSucessor<<endl;

		int startJobSucessor,startMachineSucessor;
		if(jobSucessor > 0)startJobSucessor = starttime[jobSucessor];
		else startJobSucessor = INT_MAX;
		if(machineSucessor > 0)startMachineSucessor = starttime[machineSucessor];
		else startMachineSucessor = INT_MAX;


		int z = min(startJobSucessor - processingTime[current],startMachineSucessor - processingTime[current]);

		int x = min(dueDate[current],z);

		newStartTime[current] = max(starttime[current],x);
		// cout<<"old start time:"<<starttime[current]<<endl;
		// cout<<"new start time:"<<newStartTime[current]<<endl<<endl;

		//find next op to be processed
		bool found = 0;
		for(int i = 0;i<g2[current].size();i++)
		{
			bool ok = 1;
			for(int j = 0;j<g[g2[current][i]].size();j++)
			{
				if(!visited[g[g2[current][i]][j]])ok = 0;
			}

			if(ok)
			{
				current = g2[current][i];
				found = 1;
			}
			if(!found)
			{
				//get op with greatest completion time that doesnt have a new start time yet
				for(int a = 0;a<sorted.size();a++)
				{
					if(!visited[sorted[a]])
					{
						current = sorted[a];
						break;
					}
				}
			}
		}



	}
	//cout<<endl;
	return newStartTime;


}

////////////////////////////////////////////////////////////////////
vector<int>
JIT_JSS::Calc(matriz sequence,vector<int> starttime)
{
	//create dag
	int sinkID = nOperations + 1;
	vector< vector<int> > g = createDAG(sequence,sinkID);

	// cout<<"DAG:\n";
	// for(int i = 0;i<g.size();i++)
	// {
	// 	if(i == 0 || i == sinkID)cout<<i<<":";
	// 	else cout<<"O"<<job[i]+1<<machine[i]<<":";
	// 	for(int j = 0;j<g[i].size();j++)
	// 	{
	// 		if(g[i][j] == 0 || g[i][j] == sinkID)cout<<g[i][j]<<' ';
	// 		else cout<<"O"<<job[g[i][j]]+1<<machine[g[i][j]]<<" ";
	// 	}

	// 	cout<<endl;
	// }
	
	//transpose dag
	g = TransposeDAG(g);
	// cout<<"DAG INVERTIDO\n";
	// for(int i = 0;i<g.size();i++)
	// {
	// 	if(i == 0 || i == sinkID)cout<<i<<":";
	// 	else cout<<"O"<<job[i]+1<<machine[i]<<":";
	// 	for(int j = 0;j<g[i].size();j++)
	// 	{
	// 		if(g[i][j] == 0 || g[i][j] == sinkID)cout<<g[i][j]<<' ';
	// 		else cout<<"O"<<job[g[i][j]]+1<<machine[g[i][j]]<<" ";
	// 	}

	// 	cout<<endl;
	// }
	// cout<<endl<<endl;

	//find topological sort
	vector<bool>visited(nOperations + 2,0);
	std::stack<int> stack;

	for(int i = 0;i<nOperations + 2;i++)
	{
		if(!visited[i])TopologicalSort(i,visited,stack,g);
	}

	//calculate new start times for operations following the topological sort order
	
	// cout<<"topological sort:\n";

	vector<int> newStartTime(nOperations + 1);
	int op;
	while(!stack.empty())
	{
		op = stack.top();
		stack.pop();
		//if(op == sinkID || op == 0)cout<<op<<' ';
		// else cout<<"O"<<job[op]+1<<machine[op]<<' ';
		if(op == sinkID || op == 0)continue;
		//find sucessor of op in job and machine
		int jobSucessor = 0,machineSucessor = 0;
		int opJob = job[op];
		for(int i = 0;i<jobOps[opJob].size() - 1;i++)
		{
			if(jobOps[opJob][i] == op)
			{
				jobSucessor = jobOps[opJob][i+1];
			}
		}

		int opMachine = machine[op];
		for(int i = 0;i<machineOps[opMachine].size() - 1;i++)
		{
			if(machineOps[opMachine][i] == op)
			{
				machineSucessor = machineOps[opMachine][i+1];
			}
		}

		//z = min(start job sucessor - processing time, start machine sucessor - processing time)
		int startJobSucessor,startMachineSucessor;
		if(jobSucessor)startJobSucessor = starttime[jobSucessor];
		else startJobSucessor = INT_MAX;
		if(machineSucessor)startMachineSucessor = starttime[machineSucessor];
		else startMachineSucessor = INT_MAX;

		int z = min(startJobSucessor - processingTime[op],startMachineSucessor - processingTime[op]);
 
		int x = min(dueDate[op],z);

		newStartTime[op] = max(starttime[op],x);
	}
	// cout<<endl;
	cout<<"UE\n";
	return newStartTime;

}

int
JIT_JSS::FindJobSucessor(int op)
{
	int opJob = job[op];
	int jobSucessor = -1;
	int n = jobOps[opJob].size();
	if(jobOps[opJob][n - 1] == op)return -1;
	for(int i = 0;i<jobOps[opJob].size() - 1;i++)
	{
		if(jobOps[opJob][i] == op)
		{
			jobSucessor = jobOps[opJob][i+1];
		}
	}
	return jobSucessor;
}

int
JIT_JSS::FindMachineSucessor(int op,matriz sequence)
{
	int opMachine = machine[op];
	int machineSucessor = -1;
	int n = sequence[opMachine].size();
	if(sequence[opMachine][n - 1] == op)return -1;
	for(int i = 0;i<sequence[opMachine].size() - 1;i++)
	{
		if(sequence[opMachine][i] == op)
		{
			machineSucessor = sequence[opMachine][i+1];
		}
	}
	return machineSucessor;
}

 