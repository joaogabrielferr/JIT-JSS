// JIT-jit.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include "jssinst.h"


int main()
{
    JIT_JSS jit;
    jit.parseInstance("instance.txt");
   
    matriz schedule = jit.EarliestDeadlineFirst(jit.processingOrder);

     cout<<"schedule:\n";
    for(int i = 0;i<schedule.size();i++)
    {
        cout<<"M"<<i<<":\n";
        for(int j = 0;j<schedule[i].size();j++)
        {
            cout<<"O"<<jit.job[schedule[i][j]] + 1<<jit.machine[schedule[i][j]]<<" ";
            cout<<jit.machine[schedule[i][j]]<<" ";
            cout<<jit.processingTime[schedule[i][j]]<<' ';
            cout<<jit.dueDate[schedule[i][j]]<<' ';
            cout<<jit.earliness[schedule[i][j]]<<' ';
            cout<<jit.tardiness[schedule[i][j]]<<' ';
            cout<<" start:"<<jit.startTime[schedule[i][j]]<<',';
            cout<<" completion:"<<jit.startTime[schedule[i][j]] + jit.processingTime[schedule[i][j]]<<" ";
            cout<<"   id:"<<schedule[i][j]<<endl;
        }
    }
    // vector<int> criticalPath = jit.CriticalPath(schedule);

    // vector< vector<int> >blocks = jit.CriticalBlocks(criticalPath);

    // for(vector<int> v : blocks)
    // {
    //     cout<<"block:\n";
    //     for(int i : v)cout<<i<<" ";
    //     cout<<endl;
    // }

    	vector< pair<matriz,vector<int> > > neighborhood = jit.N7(schedule);


    return 0;

    // pair<matriz,int> r = jit.LocalSearch(jit.processingOrder,500);
    // matriz schedule = r.first;

    // // matriz schedule = jit.EarliestDeadlineFirst(jit.processingOrder);

    // cout<<"schedule:\n";
    // for(int i = 0;i<schedule.size();i++)
    // {
    //     cout<<"M"<<i<<":\n";
    //     for(int j = 0;j<schedule[i].size();j++)
    //     {
    //         cout<<"O"<<jit.job[schedule[i][j]] + 1<<jit.machine[schedule[i][j]]<<" ";
    //         cout<<jit.machine[schedule[i][j]]<<" ";
    //         cout<<jit.processingTime[schedule[i][j]]<<' ';
    //         cout<<jit.dueDate[schedule[i][j]]<<' ';
    //         cout<<jit.earliness[schedule[i][j]]<<' ';
    //         cout<<jit.tardiness[schedule[i][j]]<<' ';
    //         cout<<" start:"<<jit.startTime[schedule[i][j]]<<',';
    //         cout<<" completion:"<<jit.startTime[schedule[i][j]] + jit.processingTime[schedule[i][j]]<<" ";
    //         cout<<"   id:"<<schedule[i][j]<<endl;
    //     }
    // }
    // auto r2 = jit.SchedulePenalties(schedule,jit.startTime);
    // // cout<<"penalidades inital:"<<r2[0]<<endl;
    // cout<<"penalidades :"<<r2[0]<<endl;
    // cout<<"iterations:"<<r.second<<endl;

    // jit.CriticalPath(schedule);


    // //vector< pair<matriz,vector<int> > > n = jit.Swap(schedule);

}
