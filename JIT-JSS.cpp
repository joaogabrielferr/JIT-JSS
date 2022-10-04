// JIT-JSS.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include "JSSinst.h"

int main()
{
    JSSinst jss;
    pair<sequence, pair<int, int> >r;
    jss.parseInstance("instance.txt");
    

    double best = 10000000.0;
    sequence schedule;
    for(int i = 0;i<500;i++)
    {
        sequence s = jss.GifflerThompson(jss.processingOrder);
        vector<double> results = jss.SchedulePenalties(s);
        if(results[0] < best)
        {
            best = results[0];
            schedule = s;
        }
    }

    // sequence schedule = jss.EarliestDeadlineFirst(jss.processingOrder);
    cout<<schedule.size()<<endl;
    cout<<"SCHEDULE:\n";
    for(int i = 0;i<schedule.size();i++)
    {
        cout<<"M"<<i<<":\n";
        for(int j = 0;j<schedule[i].size();j++)
        {
            schedule[i][j].print(2);
        }
        cout<<endl;
    }


    cout<<"penalties:"<<best<<endl;

    



}

// Executar programa: Ctrl + F5 ou Menu Depurar > Iniciar Sem Depuração
// Depurar programa: F5 ou menu Depurar > Iniciar Depuração

// Dicas para Começar: 
//   1. Use a janela do Gerenciador de Soluções para adicionar/gerenciar arquivos
//   2. Use a janela do Team Explorer para conectar-se ao controle do código-fonte
//   3. Use a janela de Saída para ver mensagens de saída do build e outras mensagens
//   4. Use a janela Lista de Erros para exibir erros
//   5. Ir Para o Projeto > Adicionar Novo Item para criar novos arquivos de código, ou Projeto > Adicionar Item Existente para adicionar arquivos de código existentes ao projeto
//   6. No futuro, para abrir este projeto novamente, vá para Arquivo > Abrir > Projeto e selecione o arquivo. sln
