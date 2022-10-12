#include "jssinst.h"


#define matriz vector<vector<int> >

using namespace std;

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
	cout<<"nOperations:"<<nOperations<<endl;
	processingTime.resize(nOperations + 1);
	dueDate.resize(nOperations + 1);
	earliness.resize(nOperations + 1);
	tardiness.resize(nOperations + 1);
	job.resize(nOperations + 1);
	machine.resize(nOperations + 1);
	startTime.resize(nOperations + 1);
	
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

matriz
JIT_JSS::GifflerThompson(matriz instance)
{
	//instance = processing orders
	
	matriz schedule;
	schedule.resize(nMachines);

	vector<int>G;
	vector<int>machineStart(nMachines,0),jobStart(nJobs,0);

	//add all the schedulable operations, i.e., the first operation of each job and
	//initialize the earliest start and completion times for the operations in G
	
	for(int i = 0;i<nJobs;i++)
	{
		int op = instance[i][0];
		startTime[op] = 0;
		G.push_back(op);
	}

	while(!G.empty())
	{
		//Find the earliest completion time C∗ and its associated machine M∗ among all the operations in G
		int Cstar = INT_MAX;
		int Mstar;

		for(int op : G)
		{
			if(startTime[op] + processingTime[op] < Cstar)
			{
				Cstar = startTime[op] + processingTime[op];
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
			if(startTime[op] < Cstar)G1.push_back(op);
		}	

		//Select op ∈ G1 to be scheduled next (by randomly picking a op in G1)
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
		schedule[Mstar].push_back(selected);

		startTime[selected] = max(machineStart[Mstar],jobStart[job[selected]]);

		machineStart[Mstar] = startTime[selected] + processingTime[selected];
		jobStart[job[selected]] = startTime[selected] + processingTime[selected];

		//Remove op from G and add its job successor (if any) to G;
		int selectedId = 0;
		for(int i = 0;i<G.size();i++)
		{
			if(selected == G[i])
			{
				selectedId = i;
			}
		}

		G[selectedId] = G[G.size() - 1];
		G.pop_back();

		int selectedJob = job[selected];

		for(int i = 0;i<jobOps[selectedJob].size() - 1;i++)
		{
			if(jobOps[selectedJob][i] == selected)
			{
				int next = jobOps[selectedJob][i + 1];
				startTime[next] = max(machineStart[machine[next]],jobStart[job[next]]);
				G.push_back(next);
			}
		}

		//update start times for operations in G
		for(int op : G)
		{
			startTime[op] = (startTime[op] < max(machineStart[machine[op]],jobStart[job[op]]) ? max(machineStart[machine[op]],jobStart[job[op]]) : 0);
		}
	}

	return schedule;
	
}

matriz
JIT_JSS::EarliestDeadlineFirst(matriz instance)
{
	matriz schedule;
	schedule.resize(nMachines);

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

	return schedule;

}


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


vector<pair<matriz,vector<int> > >
JIT_JSS::Swap(matriz schedule)
{

		//{schedule,startTime}
		vector<pair<matriz,vector<int> > >neighborhood;

		for(int i = 0;i<schedule.size();i++)
		{
			for(int j = 1;j<schedule[i].size();j++)
			{
				matriz neighbor = schedule;
				vector<int> neighborStartTime = startTime;
				int a = schedule[i][j-1];
				int b = schedule[i][j];
				
				int Bcompletiontime = startTime[b] + processingTime[b];

				neighborStartTime[b] = startTime[a];
				neighborStartTime[a] = Bcompletiontime - processingTime[a];

				neighbor[i][j - 1] = b;
				neighbor[i][j] = a;
				
				if(isScheduleCorrect(neighbor,neighborStartTime) && isProcessingOrderKept(neighbor,neighborStartTime))
				{
					// cout<<"TROCA VALIDA:"<<a<<" por "<<b<<endl;
					// for(int x = 0;x<neighbor.size();x++)
					// {
					// 	cout<<"M"<<x<<":\n";
					// 	for(int y = 0;b<neighbor[x].size();y++)
					// 	{
					// 	    cout<<"O"<<job[neighbor[x][y]] + 1<<machine[neighbor[x][y]]<<" ";
					// 		cout<<machine[neighbor[x][y]]<<" ";
					// 		cout<<processingTime[neighbor[x][y]]<<' ';
					// 		cout<<dueDate[neighbor[x][y]]<<' ';
					// 		cout<<earliness[neighbor[x][y]]<<' ';
					// 		cout<<tardiness[neighbor[x][y]]<<' ';
					// 		cout<<" start:"<<neighborStartTime[neighbor[x][y]]<<',';
					// 		cout<<" completion:"<<neighborStartTime[neighbor[x][y]] + processingTime[neighbor[x][y]]<<" ";
					// 		cout<<"   id:"<<neighbor[x][y]<<endl;
        
					// 	}
					// }
					neighborhood.push_back({neighbor,neighborStartTime});
					// valid.push_back(i);
				}

			}
		}

		return neighborhood;
}

pair<matriz,int>
JIT_JSS::LocalSearch(matriz instance,int ITER_MAX)
{
	matriz schedule = EarliestDeadlineFirst(instance);
	
	vector<double> initialPenalties = SchedulePenalties(schedule,startTime);
	
	int iter = 1;
	while(iter <= ITER_MAX)
	{
		vector< pair<matriz,vector<int> > > neighborhood = Swap(schedule);
		
		bool improvement = 0;
		for(auto neighbor : neighborhood)
		{
			vector<double> neighborPenalties = SchedulePenalties(neighbor.first,neighbor.second);
			vector<double> currentSchedulePenalties = SchedulePenalties(schedule,startTime);

			if(neighborPenalties[0] < currentSchedulePenalties[0])
			{
				improvement = 1;
				schedule = neighbor.first;
				startTime = neighbor.second;
			}
		}
		if(!improvement)
		{
			//local max
			break;
		}
		iter++;	
	}

	//cout<<"iteracoes:"<<iter<<endl;
	return {schedule,iter};

}




vector<int> JIT_JSS::CriticalPath(matriz schedule){

	//g[0] : source node
	//g[nOperations + 1] = sink node
	int sinkID = nOperations + 1;
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

	for(int i = 0;i<schedule.size();i++)
	{
		for(int j = 0;j<schedule[i].size();j++)
		{
			int current = schedule[i][j];
			for(int k = j + 1;k<schedule[i].size();k++)
			{
				g[current].push_back(schedule[i][k]);
			}
		}
	}

	
	//calculating longest path from source (0) to sink (nOperations + 1)

	vector<bool>visited(nOperations + 2,0);
	stack<int>stack;

	for(int i = 0;i<nOperations + 2;i++)
	{
		if(!visited[i])TopologicalSort(i,visited,stack,g);
	}

	vector<int>dist(nOperations + 2,INT_MIN);
	dist[0] = 0;

	vector<int>parent(nOperations + 2);

	parent[0] = -1;

	cout<<"topological order:\n";



	while(!stack.empty())
	{
		int u = stack.top();
		cout<<u<<" ";
		stack.pop();

		if(dist[u] != INT_MIN)
		{
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

	cout<<endl;
	vector<int>criticalPath;

	//retrieve path
	int current = sinkID;

	while(current != -1)
	{
		criticalPath.push_back(current);
		current = parent[current];
	}

	reverse(criticalPath.begin(),criticalPath.end());

	int makespan = 0;
	for(int i : criticalPath)
	{
		if(i != 0 && i != sinkID)makespan+=processingTime[i];
	}

	

	cout<<"G:\n";
	for(int i = 0;i<g.size();i++)
	{
		if(i != 0 && i != sinkID)cout<<i<<"(O"<<job[i]+1<<machine[i]<<"):\n";
		else cout<<i<<":\n";
		for(int j = 0;j<g[i].size();j++)
		{
			if(g[i][j] == sinkID)
			{
				cout<<"sink ";
			}else cout<<"O"<<job[g[i][j]] + 1<<machine[g[i][j]]<<" ";
		}
		cout<<endl;
	}	
	// cout<<"critical path:\n";
	// for(int i : criticalPath)
	// {
	// 	if(i != 0 && i != sinkID)cout<<i<<"(O"<<job[i] + 1<<machine[i]<<") ";
	// 	else if(i == 0)cout<<"source ";
	// 	else cout<<"sink ";
	// }
	// cout<<endl<<"makespan:"<<makespan<<endl;

	//take out source and sink nodes
	criticalPath.erase(criticalPath.begin());
	criticalPath.pop_back();

	cout<<"critical path:\n";
	for(int i : criticalPath)
	{
		if(i != 0 && i != sinkID)cout<<i<<"(O"<<job[i] + 1<<machine[i]<<") ";
		else if(i == 0)cout<<"source ";
		else cout<<"sink ";
	}
	cout<<endl<<"makespan:"<<makespan<<endl;

	return criticalPath;

}


void
JIT_JSS::TopologicalSort(int v,vector<bool>&visited,stack<int>&Stack,vector< vector<int> >&g)
{
	visited[v] = 1;
	for(int i = 0;i<g[v].size();i++)
	{
		int u = g[v][i];
		if(!visited[u])TopologicalSort(u,visited,Stack,g);
	}

	Stack.push(v);

}

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


//
pair<matriz,vector<int> >
JIT_JSS::insertion1(matriz schedule, int block_op,int block_first_op)
{
	//cout<<"putting O"<<job[block_op] + 1<<machine[block_op]<<" in the beginning of the block:\n";
			matriz neighbor = schedule;
			vector<int>nstart = startTime;
			//machine where block_op is processed
			int iMachine = machine[block_op];
			// cout<<"iMachine sequence:\n";
			// for(int j = 0;j<schedule[iMachine].size();j++)
			// {
			// 	cout<<"O"<<job[schedule[iMachine][j]]+1<<machine[schedule[iMachine][j]]<<" ";
			// }
			// cout<<endl;
			map<int,int>gap;
			
			//find the gaps between op and its predecessor in machine
			for(int j = 0;j<schedule[iMachine].size();j++)
			{
				int op = schedule[iMachine][j];
				if(j == 0)gap[op] = 0;
				else
				{
					int prev = schedule[iMachine][j - 1];
					gap[op] = startTime[op] - (startTime[prev] + processingTime[prev]);
				}
			}

			//new sequence for machine iMachine
			vector<int>nMachine;
			int current = 0,last = 0,idx = 0;
			while(schedule[iMachine][current] != block_first_op)
			{
				nMachine.push_back(schedule[iMachine][current]);
				last = schedule[iMachine][current];
				current++;
				idx++;
			}

			//inserts block_op in the sequence;
			nMachine.push_back(block_op);
			//if there is a gap before block_first_op, put block_op after the gap
			int prev_completion = nstart[last] + processingTime[last];
			if(gap[schedule[iMachine][current]])
			{
				nstart[block_op] = prev_completion + gap[schedule[iMachine][current]];
				gap[schedule[iMachine][current]] = 0;
			}else nstart[block_op] = prev_completion;

			last = block_op;

			//put other operations in the sequence while shifting their start times
			for(current;current < schedule[iMachine].size();current++)
			{
				if(schedule[iMachine][current] == block_op)continue;
				nMachine.push_back(schedule[iMachine][current]);
				//start = completion time of prev op + gap
				nstart[schedule[iMachine][current]] = nstart[last] + processingTime[last] + gap[schedule[iMachine][current]];
				last = schedule[iMachine][current];
			}

			//change iMachine sequence in the neighbor
			neighbor[iMachine] = nMachine;
			
			// cout<<"result:\n";
			// for(int j = 0;j<neighbor[iMachine].size();j++)
			// {
			// 	int opstart = nstart[neighbor[iMachine][j]];
			// 	int opproctime = processingTime[neighbor[iMachine][j]];
			// 	cout<<"O"<<job[neighbor[iMachine][j]]+1<<machine[ neighbor[iMachine][j]]<<"("<<opstart<<","<<opstart + opproctime<<") ";
			// }
			// cout<<endl;

		return {neighbor,nstart};

}

pair<matriz,vector<int> >
 JIT_JSS::insertion2(matriz schedule,int block_op, int block_first_op)
 {
	//cout<<"putting O"<<job[block_op] + 1<<machine[block_op]<<" in the END of the block:\n";
			matriz neighbor = schedule;
			vector<int>nstart = startTime;
			//machine where block_op is processed
			int iMachine = machine[block_op];
			// cout<<"iMachine sequence:\n";
			// for(int j = 0;j<schedule[iMachine].size();j++)
			// {
			// 	cout<<"O"<<job[schedule[iMachine][j]]+1<<machine[schedule[iMachine][j]]<<" ";
			// }
			// cout<<endl;
			map<int,int>gap;
			
			//find the gaps between op and its predecessor in machine
			for(int j = 0;j<schedule[iMachine].size();j++)
			{
				int op = schedule[iMachine][j];
				if(j == 0)gap[op] = 0;
				else
				{
					int prev = schedule[iMachine][j - 1];
					gap[op] = startTime[op] - (startTime[prev] + processingTime[prev]);
				}
			}

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

			// //inserts block_op in the sequence;
			// nMachine.push_back(block_op);

			//if there is a gap before block_op, it will be considered in the sucessor of block_op
			//at this point current == block_op
			int prev_completion = nstart[last] + processingTime[last];
			if(gap[schedule[iMachine][current]])
			{
				gap[schedule[iMachine][current + 1]] += gap[schedule[iMachine][current]];
				//nstart[block_op] = prev_completion + gap[schedule[iMachine][current]];
				gap[schedule[iMachine][current]] = 0;
			}
			
			current++;
			//keep putting other operations in the sequence while shifting their start times
			for(current;current < schedule[iMachine].size();current++)
			{
				if(schedule[iMachine][current] == block_op)continue;
				nMachine.push_back(schedule[iMachine][current]);
				//start = completion time of prev op + gap
				nstart[schedule[iMachine][current]] = nstart[last] + processingTime[last] + gap[schedule[iMachine][current]];
				last = schedule[iMachine][current];
			}

			//now put block_op in the end of sequence
			nMachine.push_back(block_op);
			nstart[block_op] = nstart[last] + processingTime[last];

			//change iMachine sequence in the neighbor
			neighbor[iMachine] = nMachine;
			

		return {neighbor,nstart};
 
 }

//block_op:first operation of the block
//prev_op:operation in wich block_op will be placed after
pair<matriz,vector<int> >
JIT_JSS::insertion3(matriz schedule,int prev_op,int block_op){

	cout<<"inserting O"<<job[block_op] + 1<<machine[block_op]<<" after O"<<job[prev_op] + 1<<machine[prev_op]<<endl;
	matriz neighbor = schedule;
	vector<int>nstart = startTime;

	int iMachine = machine[block_op];

	cout<<"iMachine sequence:\n";
	for(int j = 0;j<schedule[iMachine].size();j++)
	{
		cout<<"O"<<job[schedule[iMachine][j]]+1<<machine[schedule[iMachine][j]]<<"("<<startTime[schedule[iMachine][j]]<<","<<startTime[schedule[iMachine][j]] + processingTime[schedule[iMachine][j]]<<") ";
	}
	cout<<endl;
	map<int,int>gap;
	
	//find the gaps between op and its predecessor in machine
	for(int j = 0;j<schedule[iMachine].size();j++)
	{
		int op = schedule[iMachine][j];
		if(j == 0)gap[op] = 0;
		else
		{
			int prev = schedule[iMachine][j - 1];
			gap[op] = startTime[op] - (startTime[prev] + processingTime[prev]);
		}
	}

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

	//if there is a gap before block_op, it will be considered in the sucessor of block_op
	//at this point current == block_op
	//int prev_completion = nstart[last] + processingTime[last];
	if(gap[schedule[iMachine][current]])
	{
		gap[schedule[iMachine][current + 1]] += gap[schedule[iMachine][current]];
		//nstart[block_op] = prev_completion + gap[schedule[iMachine][current]];
		gap[schedule[iMachine][current]] = 0;
	}


	current++; 

	//keep inserting other operations in the sequence while shifting their start times
	for(current;current < schedule[iMachine].size();current++)
	{
		if(schedule[iMachine][current] == block_op)continue;
		nMachine.push_back(schedule[iMachine][current]);
		//start = completion time of prev op + gap
		nstart[schedule[iMachine][current]] = nstart[last] + processingTime[last] + gap[schedule[iMachine][current]];
		last = schedule[iMachine][current];

		//insert the first operation of block after prev_op
		if(last == prev_op)
		{
			nMachine.push_back(block_op);
			nstart[block_op] = nstart[last] + processingTime[last];
			last = block_op;
		}
	}

	cout<<"sequence after change:\n";
	for(int j = 0;j<nMachine.size();j++)
	{
		cout<<"O"<<job[nMachine[j]]+1<<machine[nMachine[j]]<<"("<<nstart[nMachine[j]]<<","<<nstart[nMachine[j]] + processingTime[nMachine[j]]<<") ";
	}
	cout<<endl;

	//change sequence
	neighbor[iMachine] = nMachine;

	return {neighbor,nstart};

}



vector< pair<matriz,vector<int> > >
JIT_JSS::N7(matriz schedule)
{
	vector<int> criticalPath = CriticalPath(schedule);
	vector< vector<int> > blocks = CriticalBlocks(criticalPath);

	vector< pair<matriz,vector<int> > > neighborhood;

	for(vector<int> block : blocks)
	{
		if(block.size() == 2)continue;

		cout<<"block atual:\n";
		for(int b : block)cout<<"O"<<job[b]+1<<machine[b]<<" ";
		cout<<endl;

		//inserting operation at the beginning and at the end of the block
		for(int i = 1;i< block.size() - 1;i++)
		{
			//inserting operation in the beginning of the block
			pair<matriz,vector<int> > neighbor = insertion1(schedule,block[i],block[0]);
			if(isScheduleCorrect(neighbor.first,neighbor.second) && isProcessingOrderKept(neighbor.first,neighbor.second))
			{
				neighborhood.push_back(neighbor);
			}
			
			//inserting operation in the end of the block;
			neighbor = insertion2(schedule,block[i],block[0]);
			if(isScheduleCorrect(neighbor.first,neighbor.second) && isProcessingOrderKept(neighbor.first,neighbor.second))
			{
				neighborhood.push_back(neighbor);
			}
		}

		//inserting first and last operations inside the block
		for(int  i = 1;i<block.size();i++)
		{
			pair<matriz,vector<int> > neighbor = insertion3(schedule,block[i],block[0]);
			if(isScheduleCorrect(neighbor.first,neighbor.second) && isProcessingOrderKept(neighbor.first,neighbor.second))
			{
				neighborhood.push_back(neighbor);
			}

		}

		//cout<<endl<<endl;


		

	}

	return neighborhood;

}

matriz
JIT_JSS::TabuSearch(matriz instance, int MAX_ITER, int TABU_TENURE)
{


}

