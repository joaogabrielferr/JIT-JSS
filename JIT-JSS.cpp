// JIT-jit.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include "jssinst.h"


int main()
{
    JIT_JSS jit;
    jit.parseInstance("instance.txt");
   
    // Schedule schedule = jit.EarliestDeadlineFirst(jit.processingOrder);
    // vector<Neighbor> n = jit.N7(schedule.first,schedule.second);
    
    // for(Neighbor ne : n)
    // {
    //     cout<<"NEIGHBOR:\n";
    //     cout<<"move: O"<<jit.job[ne.second[0]]+1<<jit.machine[ne.second[0]]<<",O"<<jit.job[ne.second[1]]+1<<jit.machine[ne.second[1]]<<endl;
    //     for(int i = 0;i<ne.first.first.size();i++)
    //     {
    //     cout<<"M"<<i<<":\n";
    //     for(int j = 0;j<ne.first.first[i].size();j++)
    //     {
    //         cout<<"O"<<jit.job[ne.first.first[i][j]] + 1<<jit.machine[ne.first.first[i][j]]<<"(";
    //         cout<<ne.first.second[ne.first.first[i][j]]<<",";
    //         cout<<ne.first.second[ne.first.first[i][j]] + jit.processingTime[ne.first.first[i][j]]<<") ";
    //     }
    //     cout<<endl;
    //     }

    // }

    // cout<<"NEIGHBORHOOD SIZE:"<<n.size()<<endl;

    // return 0;

    Schedule schedule = jit.TabuSearch(jit.processingOrder,1000,8);


    
    cout<<"schedule:\n";
    for(int i = 0;i<schedule.first.size();i++)
    {
        cout<<"M"<<i<<":\n";
        for(int j = 0;j<schedule.first[i].size();j++)
        {
            // cout<<"O"<<jit.job[schedule.first[i][j]] + 1<<jit.machine[schedule.first[i][j]]<<" ";
            // cout<<jit.machine[schedule.first[i][j]]<<" ";
            // cout<<jit.processingTime[schedule.first[i][j]]<<' ';
            // cout<<jit.dueDate[schedule.first[i][j]]<<' ';
            // cout<<jit.earliness[schedule.first[i][j]]<<' ';
            // cout<<jit.tardiness[schedule.first[i][j]]<<' ';
            // cout<<" start:"<<jit.startTime[schedule.first[i][j]]<<',';
            // cout<<" completion:"<<jit.startTime[schedule.first[i][j]] + jit.processingTime[schedule.first[i][j]]<<" ";
            // cout<<"   id:"<<schedule.first[i][j]<<endl;
            cout<<"O"<<jit.job[schedule.first[i][j]] + 1<<jit.machine[schedule.first[i][j]]<<"(";
            cout<<schedule.second[schedule.first[i][j]]<<","<<schedule.second[schedule.first[i][j]] + jit.processingTime[schedule.first[i][j]]<<") ";
        }
        cout<<endl;
    }

    
    Schedule inicial = jit.EarliestDeadlineFirst(jit.processingOrder);
    cout<<"solucao inicial:\n";
        for(int i = 0;i<inicial.first.size();i++)
    {
        cout<<"M"<<i<<":\n";
        for(int j = 0;j<inicial.first[i].size();j++)
        {
            // cout<<"O"<<jit.job[inicial.first[i][j]] + 1<<jit.machine[inicial.first[i][j]]<<" ";
            // cout<<jit.machine[inicial.first[i][j]]<<" ";
            // cout<<jit.processingTime[inicial.first[i][j]]<<' ';
            // cout<<jit.dueDate[inicial.first[i][j]]<<' ';
            // cout<<jit.earliness[inicial.first[i][j]]<<' ';
            // cout<<jit.tardiness[inicial.first[i][j]]<<' ';
            // cout<<" start:"<<jit.startTime[inicial.first[i][j]]<<',';
            // cout<<" completion:"<<jit.startTime[inicial.first[i][j]] + jit.processingTime[inicial.first[i][j]]<<" ";
            // cout<<"   id:"<<inicial.first[i][j]<<endl;
            cout<<"O"<<jit.job[inicial.first[i][j]] + 1<<jit.machine[inicial.first[i][j]]<<"(";
            cout<<inicial.second[inicial.first[i][j]]<<","<<inicial.second[inicial.first[i][j]] + jit.processingTime[schedule.first[i][j]]<<") ";
        }
        cout<<endl;
    }

    vector<int> path = jit.CriticalPath(schedule.first);
    int m = 0;
    cout<<"path:";
    for(int i : path)
    {
        cout<<"O"<<jit.job[i]+1<<jit.machine[i]<<" ";
        m+=jit.processingTime[i];
    }cout<<"\nmakespan:"<<m<<endl;


    cout<<"penalidades iniciais:";
    vector<double>pi = jit.SchedulePenalties(inicial.first,inicial.second);
    cout<<pi[0]<<endl<<"earliness:"<<pi[1]<<endl<<"tardiness:"<<pi[2]<<endl;

    cout<<"penalidades:";
    vector<double>v = jit.SchedulePenalties(schedule.first,schedule.second);
    cout<<v[0]<<endl;
    cout<<"earliness:"<<v[1]<<endl<<"tardiness:"<<v[2]<<endl;

    return 0;

}
