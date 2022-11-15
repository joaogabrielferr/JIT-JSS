// JIT-jit.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include "jssinst.h"


int main(int argc,char **argv)
{

    // JIT_JSS c;
    // c.parseInstance("instance.txt");

    // Schedule ini = c.EarliestDeadlineFirst(c.processingOrder);
    // vector<double>p1 = c.SchedulePenalties(ini.first,ini.second);
    // cout<<"schedule:\n";
    // for(int i = 0;i<ini.first.size();i++)
    // {
    //     cout<<"O"<<c.job[i]+1<<c.machine[i]<<":";
    //     for(int j = 0;j<ini.first[i].size();j++)
    //     {
    //         cout<<"O"<<c.job[ini.first[i][j]] + 1<<c.machine[ini.first[i][j]]<<"("<<ini.second[ini.first[i][j]]<<","<<ini.second[ini.first[i][j]] + c.processingTime[ini.first[i][j]]<<") ";
    //     }
    //     cout<<endl;

    // }

    // ini.second = c.Calc(ini.first,ini.second);
    // vector<double>p2 = c.SchedulePenalties(ini.first,ini.second);
    // cout<<"ini:"<<p1[0]<<endl<<"ini with calc:"<<p2[0]<<endl;




    // return 0;

    if(argc == 0)
    {
        cout<<"no argument provided\n";
        return -1;
    }
    JIT_JSS jit;
    string file_path(argv[1]);
    jit.parseInstance(file_path);
    //jit.parseInstance("instance.txt");

    pair<Schedule,int> output = jit.LocalSearch(jit.processingOrder,1000,0,"SWAP");
    
    Schedule schedule = output.first;

    vector<double>penalties = jit.SchedulePenalties(schedule.first,schedule.second);
    
    //penalidade inicial
    Schedule inicial = jit.EarliestDeadlineFirst(jit.processingOrder);
    vector<double>pi = jit.SchedulePenalties(inicial.first,inicial.second);
    
    //penalidade total (S)
    cout<<penalties[0]<<endl;
    //penalidade inicial (S0)
    cout<<pi[0]<<endl;
    //iteracoes
    cout<<output.second<<endl;
    //earliness S0
    cout<<pi[1]<<endl;
    //tardiness S0
    cout<<pi[2]<<endl;
    //earliness S
    cout<<penalties[1]<<endl;
    //tardiness S
    cout<<penalties[2]<<endl;

    return 0;

}
