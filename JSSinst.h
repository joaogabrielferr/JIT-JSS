#pragma once

#include<iostream>
#include<string>
#include<vector>
#include<sstream>
#include<fstream>

using namespace std;


#define sequence vector<vector<Operation> >

class Operation {
	
	public:
	int id, machine, processingTime, dueDate,job;
	double earliness, tardiness;
	
	int startTime;
	int completionTime;

	Operation() {};
	Operation(int id) {};
	Operation(int j,int m, int p, int d, double e, double t, int id)
	{
		this->job = j;
		this->machine = m;
		this->processingTime = p;
		this->dueDate = d;
		this->earliness = e;
		this->tardiness = t;
		this->id = id;

	}

	void print(int i){
		if(i == 1)cout<<this->machine<<" "<<this->processingTime<<" "<<this->dueDate<<" "<<this->earliness<<" "<<this->tardiness<<" id:"<<this->id<<endl;
		else if(i == 0) cout<<this->machine<<" "<<this->processingTime<<" "<<this->dueDate<<" "<<this->earliness<<" "<<this->tardiness<<" id:"<<this->id<<"    ";
		else cout<<"("<<this->id<<") O"<<this->job<<this->machine<<" startTime:"<<this->startTime<<",proc:"<<this->processingTime<<",comp:"<<this->completionTime<<endl;
	}
};

class JSSinst {
	
public:
	//number of jobs in the instance
	int nJobs;
	//number of machines in the instance
	int nMachines;

	//the processing order of the operations
	sequence processingOrder;
	//feasible schedule for the instance
	sequence schedule;
	//start time of all operations
	vector< vector<int> > startTimes;

	// OpToJob[i] = job of operation i
	vector<int>OpTojob;


vector<double>
readLine(string line);

void
parseInstance(string path);

sequence
EarliestDeadlineFirst(sequence instance);

sequence
GifflerThompson(sequence instance);

//output[0] = total penalties,output[1] = total earliness,output[2] = total tardiness
vector<double>
SchedulePenalties(sequence schedule);

vector < sequence >
Swap(sequence schedule);

sequence
LocalSearch(sequence instance,int MAX_ITER);

vector<Operation>
CriticalPath(sequence schedule);

vector< sequence >
N7(sequence schedule);

sequence
TabuSearch(sequence instance, int MAX_ITER, int TABU_TENURE);

};






