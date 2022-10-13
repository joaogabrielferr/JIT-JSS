#pragma once

#include<iostream>
#include<string>
#include<vector>
#include<sstream>
#include<fstream>
#include<algorithm>
#include<stack>
#include <map>

using namespace std;


#define matriz vector<vector<int> >

#define Neighbor pair< pair<matriz, vector<int> >, vector<int> >

class JIT_JSS {

//operations start from 1
//jobs and machines start  from 0 (the machines are 0-indexed in the instances of Baptiste et. al(2008))
	
public:
	//number of jobs in the instance
	int nJobs;
	//number of machines in the instance
	int nMachines;
	//number of operations
	int nOperations;
	//processingTime[i] = processing time of operation i
	vector<int>processingTime;
	//duedDate[i] = due date of operation i
	vector<int>dueDate;
	//earliness[i] = earliness penalty of operation i
	vector<double>earliness;
	//tardiness[i] = tardiness penalty of operation i
	vector<double>tardiness;
	//job[i] = job of operation i (job index starts from 0)
	vector<int>job;
	//machine[i] = machine of job i (machine index starts from 0)
	vector<int>machine;
	//startTime[i] = start time of operation i in the main schedule
	vector<int> startTime;
	//jobOps[i] = operations of job i
	matriz jobOps;
	//machineOps[i] = operations of machine i
	matriz machineOps;
	//processingOrder[i] = processing order of job i
	matriz processingOrder;


vector<double>
readLine(string line);

void
parseInstance(string path);

matriz
EarliestDeadlineFirst(matriz instance);

matriz
GifflerThompson(matriz instance);

//output[0] = total penalties,output[1] = total earliness,output[2] = total tardiness
vector<double>
SchedulePenalties(matriz schedule,vector<int>scheduleStartTimes);

bool
isScheduleCorrect(matriz s,vector<int> neighborStartTime);

bool
isProcessingOrderKept(matriz schedule,vector<int>scheduleStartTime);

//vector < pair < sequence , startTimes > >
vector<pair<matriz,vector<int> > >
Swap(matriz schedule);

// pair <sequence , iterations >
pair<matriz,int>
LocalSearch(matriz instance,int MAX_ITER);


vector<int>
CriticalPath(matriz schedule);

void
TopologicalSort(int v,vector<bool>&visited,stack<int>&Stack,vector< vector<int> >&g);

vector< vector<int> >
CriticalBlocks(vector<int> criticalPath);

vector<Neighbor>
N7(matriz schedule,vector<int> scheduleStartTimes);

pair<matriz,vector<int> >
insertion1(matriz schedule, int block_op,int block_first_op, vector<int> scheduleStartTimes);


pair<matriz,vector<int> >
insertion2(matriz schedule,int block_op, int block_first_op, vector<int> scheduleStartTimes);

pair<matriz,vector<int> >
insertion3(matriz schedule,int prev_op,int block_op, vector<int> scheduleStartTimes);

pair<matriz,vector<int> >
insertion4(matriz schedule,int next_op,int block_op, vector<int> scheduleStartTimes);

matriz
TabuSearch(matriz instance, int MAX_ITER, int TABU_TENURE);

};






