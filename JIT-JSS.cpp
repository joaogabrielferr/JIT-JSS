// JIT-jit.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include "jssinst.h"

template <
    class result_t   = std::chrono::milliseconds,
    class clock_t    = std::chrono::steady_clock,
    class duration_t = std::chrono::milliseconds
>
auto since(std::chrono::time_point<clock_t, duration_t> const& start)
{
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}


int main(int argc,char **argv)
{

    // JIT_JSS c;
    // c.parseInstance("instance.txt");

    // Schedule ini = c.EarliestDeadlineFirst(c.processingOrder);
    // vector<double>p1 = c.SchedulePenalties(ini.first,ini.second);
    // cout<<"schedule:\n";
    // for(int i = 0;i<ini.first.size();i++)
    // {
    //     cout<<i<<":";
    //     for(int j = 0;j<ini.first[i].size();j++)
    //     {
    //         cout<<"O"<<c.job[ini.first[i][j]] + 1<<c.machine[ini.first[i][j]]<<"("<<ini.second[ini.first[i][j]]<<","<<ini.second[ini.first[i][j]] + c.processingTime[ini.first[i][j]]<<") ";
    //     }
    //     cout<<endl;

    // }

    // ini.second = c.Calc2(ini.first,ini.second);
    // vector<double>p2 = c.SchedulePenalties(ini.first,ini.second);
    // cout<<"ini:"<<p1[0]<<endl<<"ini with calc:"<<p2[0]<<endl;
    // cout<<"schedule:\n";
    // for(int i = 0;i<ini.first.size();i++)
    // {
    //     cout<<i<<":";
    //     for(int j = 0;j<ini.first[i].size();j++)
    //     {
    //         cout<<"O"<<c.job[ini.first[i][j]] + 1<<c.machine[ini.first[i][j]]<<"("<<ini.second[ini.first[i][j]]<<","<<ini.second[ini.first[i][j]] + c.processingTime[ini.first[i][j]]<<") ";
    //     }
    //     cout<<endl;

    // }




    //return 0;

    if(argc == 0)
    {
        cout<<"no argument provided\n";
        return -1;
    }

    JIT_JSS jit;
    string file_path(argv[1]);
    string tabu_tenure(argv[2]);
    jit.parseInstance(file_path);
    //jit.parseInstance("instance.txt");
    int size1 = jit.nJobs;
    int size2 = jit.nMachines;
    int max_time = 0;
    if(size1 == 10 && size2 == 2)max_time = 19;//19
    if(size1 == 10 && size2 == 5)max_time = 32;//32
    if(size1 == 10 && size2 == 10)max_time = 43;//43
    if(size1 == 15 && size2 == 2)max_time = 44;//44
    if(size1 == 15 && size2 == 5)max_time = 90; //90
    if(size1 == 15 && size2 == 10)max_time = 103;//103
    if(size1 == 20 && size2 == 2)max_time = 86;//86
    if(size1 == 20 && size2 == 5)max_time = 176;//176
    if(size1 == 20 && size2 == 10)max_time = 233;//233

    auto start = std::chrono::steady_clock::now();
    //pair<Schedule,int> output = jit.LocalSearch(jit.processingOrder,max_time,0,"N7");
    pair<Schedule,int> output = jit.TabuSearch(jit.processingOrder,max_time,stoi(tabu_tenure));
    auto finish = since(start).count();

    //pair<pair<Schedule,int>,long long> output = jit.TabuSearch(jit.processingOrder,max_time,stoi(tabu_tenure));
    //pair<Schedule,int> output = jit.TabuSearch(jit.processingOrder,max_time,8);
    Schedule schedule = output.first;

    vector<double>penalties = jit.SchedulePenalties(schedule.first,schedule.second);

    //penalidade inicial
    Schedule inicial;
    //vector<double>pi = jit.SchedulePenalties(inicial.first,inicial.second);
    Schedule inicial1 = jit.EarliestDeadlineFirst(jit.processingOrder);
    Schedule inicial2 = jit.GifflerThompson(jit.processingOrder);
    vector<double>pi1 = jit.SchedulePenalties(inicial1.first,inicial1.second);
    vector<double>pi2 = jit.SchedulePenalties(inicial2.first,inicial2.second);
    vector<double>pi;
    if(pi1[0] <= pi2[0])
    {
        inicial = inicial1;
        pi = pi1;
    }else
    {
        inicial = inicial2;
        pi = pi2;
    }
    //cout<<penalties[0];

    //return 0;

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
    //tempo total
    double tempo = finish/1000.0;
    cout<<fixed<<setprecision(1)<<tempo<<endl;

    return 0;

}
