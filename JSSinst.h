#pragma once

#include<iostream>
#include<string>
#include<vector>
#include<sstream>
#include<fstream>
#include<algorithm>
#include<stack>
#include<map>
#include<set>

using namespace std;

//matriz: sequence of ops in the machines
#define matriz vector<vector<int> >

//Schedule: sequence of ops in the machines + start times
#define Schedule pair<matriz,vector<int> >


//Neighbor: Schedule + move that created the neighbor
#define Neighbor pair< Schedule, vector<int> >

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

Schedule
EarliestDeadlineFirst(matriz instance);

matriz
GifflerThompson(matriz instance);

//output[0] = total penalties,output[1] = total earliness,output[2] = total tardiness
vector<double>
SchedulePenalties(matriz schedule,vector<int>scheduleStartTimes);

//check if there is two ops from the same job being executed at the same time
bool
isScheduleCorrect(matriz s,vector<int> neighborStartTime);

//check if the processing order of the jobs are manteined in the scheduled found
bool
isProcessingOrderKept(matriz schedule,vector<int>scheduleStartTime);

//check if the start times are correct i.e if the start time of a op is >= the start time of its predecessor
bool
isSequenceCorrect(matriz schedule,vector<int>scheduleStartTime);

//vector < pair < sequence , startTimes > >
vector<Neighbor>
Swap(matriz schedule,vector<int> startTimes);

vector<Neighbor>
Swap2(matriz sequence);

matriz
SwapOperations(matriz sequence,int op1,int op2,int op1idx,int op2idx);

//op1 will be inserted before op2
matriz
InsertOperationBefore(matriz sequence,int op1,int op2);

//op1 will be inserted after op2
matriz
InsertOperationAfter(matriz sequence,int op1,int op2);

pair<Schedule,int>
LocalSearchTest(matriz instance,int MAX_ITER);

//find start time for operations in a sequence
vector<int>
calcStartTimes(matriz sequence,int z);

// pair <sequence , iterations >
pair<Schedule,int>
LocalSearch(matriz instance,int MAX_ITER,int flag,string nflag,Schedule schedule = {});

vector<int>
CriticalPath(matriz schedule);

void
TopologicalSort(int v,vector<bool>&visited,std::stack<int>&Stack,vector< vector<int> >&g);

vector< vector<int> >
CriticalBlocks(vector<int> criticalPath);

pair<vector<int>,vector<int> >
SwapAdj(vector<int>sequence,vector<int>stimes,int op1,int op2);

vector<Neighbor>
N7(matriz schedule,vector<int> scheduleStartTimes);

vector<Neighbor>
N7_2(matriz schedule);

vector<Neighbor>
N5(matriz schedule,vector<int> scheduleStartTimes);

pair<matriz,vector<int> >
insertion1(matriz schedule, int block_op,int block_first_op, vector<int> scheduleStartTimes,int z);

pair<matriz,vector<int> >
insertion2(matriz schedule,int block_op,int last_op, vector<int> scheduleStartTimes);

pair<matriz,vector<int> >
insertion3(matriz schedule,int prev_op,int block_op, vector<int> scheduleStartTimes);

pair<matriz,vector<int> >
insertion4(matriz schedule,int next_op,int block_op, vector<int> scheduleStartTimes);

pair<Schedule,int>
TabuSearch(matriz instance, int MAX_ITER, int TABU_TENURE);

Schedule
DelayOperations(Schedule s);

vector<int>
Calc(matriz sequence,vector<int>starttime);

vector<int>
Calc2(matriz sequence,vector<int>starttime);

vector< vector<int> >
createDAG(matriz sequence,int sinkID);

vector< vector<int> >
TransposeDAG(vector<vector<int> >g);

int
FindJobSucessor(int op);

int
FindMachineSucessor(int op,matriz sequence);

void
PrintSchedule(matriz sequence,vector<int>time);

void
printOperation(int op);

void
PrintSequence(matriz sequence);

};






