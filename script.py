import subprocess
from csv import writer
import pandas as pd


def oneN():

    names = ['tight-equal','loose-equal','tight-tard','loose-tard']
    sizes = [['10x2','10x5','10x10'],['15x2','15x5','15x10'],['20x2','20x5','20x10']]

    with open('output.txt','r+') as file:
        for i in range(0,len(sizes)): #len(sizes)
            for k in range(0,len(names)): #len(names)
                for j in range(0,len(sizes)): #len(sizes)
                    instance_size = sizes[i][j]
                    
                    for ins in range(1,3):
                        instance_name = names[k]
                        print("NOVA EXECUCAO,", instance_name,",",instance_size,",",ins)
                        file_name = 'C:\\Users\\joaog\Desktop\\JIT-JSS\\JIT-JSS\\instances\\' + str(instance_name) + '\\test' + str(ins) + '_' + str(instance_size) + '.txt'
                        p = subprocess.run(['tcc',file_name],stdout = file,text=True);
                        file.seek(0)
                        data = file.readlines()
                        #sequence: S,S0,iteracoes,earliness0,tardiness0,earliness,tardiness
                        print(data)
                        if len(data) != 7:
                            print("WRONG OUTPUT")
                            exit()
                        s = float(data[0])
                        s0 = float(data[1])
                        iter = int(data[2])
                        earliness0 = float(data[3])
                        tardiness0 = float(data[4])
                        earliness = float(data[5])
                        tardiness = float(data[6])
                        print(s," ",s0," ",iter," ",earliness0," ",tardiness0," ",earliness," ",tardiness)
                        
                        file.seek(0)
                        file.truncate()
                        
                        
                        with open('resultados.csv','a',newline='') as planilha:
                            nome_instancia = instance_name + '_' + instance_size + '_' + str(ins); 
                            linha = [nome_instancia,s,'_',s0,iter,earliness0,tardiness0,earliness,tardiness,0,0,0]
                            w = writer(planilha)
                            w.writerow(linha)
                            planilha.close()
                            s = None
                            s0 = None
                            iter = None
                            earliness0 = None
                            earliness = None
                            tardiness0 = None
                            tardiness = None


def twoN():
    names = ['tight-equal','loose-equal','tight-tard','loose-tard']
    sizes = [['10x2','10x5','10x10'],['15x2','15x5','15x10'],['20x2','20x5','20x10']]

    with open('output.txt','r+') as file:
        for i in range(0,len(sizes)): #len(sizes)
            for k in range(0,len(names)): #len(names)
                for j in range(0,len(sizes)): #len(sizes)
                    instance_size = sizes[i][j]
                    
                    for ins in range(1,3):
                        instance_name = names[k]
                        print("NOVA EXECUCAO,", instance_name,",",instance_size,",",ins)
                        file_name = 'C:\\Users\\joaog\Desktop\\JIT-JSS\\JIT-JSS\\instances\\' + str(instance_name) + '\\test' + str(ins) + '_' + str(instance_size) + '.txt'
                        p = subprocess.run(['tcc',file_name],stdout = file,text=True);
                        file.seek(0)
                        data = file.readlines()
                        #sequence n5 + n7: n5,n7,S0,iteracoes,earliness0,tardiness0,earliness,tardiness
                        if len(data) != 8:
                            print("WRONG OUTPUT")
                            exit()
                        print(data)
                        n5 = float(data[0])
                        n7 = float(data[1])
                        s0 = float(data[2])
                        iter = int(data[3])
                        earliness0 = float(data[4])
                        tardiness0 = float(data[5])
                        earliness = float(data[6])
                        tardiness = float(data[7])
                        print(n5," ",n7," ",s0," ",iter," ",earliness0," ",tardiness0," ",earliness," ",tardiness)
                        
                        file.seek(0)
                        file.truncate()
                        

                        with open('resultados.csv','a',newline='') as planilha:
                            nome_instancia = instance_name + '_' + instance_size + '_' + str(ins); 
                            linha = [nome_instancia,n5,n7,'_',s0,iter,earliness0,tardiness0,earliness,tardiness,0,0,0]
                            w = writer(planilha)
                            w.writerow(linha)
                            planilha.close()


def addDifference(file_name):
    pd.options.display.max_rows = 9999
    df = pd.read_csv(file_name + '.csv')
    df = df.reset_index()  # make sure indexes pair with number of rows
    
    for index,row in df.iterrows():
        earliness0 = float(row['earliness0'])
        tardiness0 = float(row['tardiness0'])
        earliness = float(row['earliness'])
        tardiness = float(row['tardiness'])

        #percentage increase = (increase or decrease) ÷ original number × 100
        EAdifference = float(format(((abs(earliness0 - earliness))/earliness0)*100,'.2f'))
        TDdifference = float(format(((abs(tardiness0 - tardiness))/tardiness0)*100,'.2f'))
        if earliness0 > earliness:EAdifference = -EAdifference
        if tardiness0 > tardiness: TDdifference = -TDdifference
        EAdifference = str(EAdifference) + '%'
        TDdifference = str(TDdifference) + '%'

        with open('resultados.csv','a',newline='') as planilha:
            #nome_instancia = instance_name + '_' + instance_size + '_' + str(ins); 
            linha = [EAdifference,TDdifference]
            w = writer(planilha)
            w.writerow(linha)
            planilha.close()            






def convert(file_name):    
    pd.options.display.max_rows = 9999
    df = pd.read_csv(file_name + '.csv')
    df = df.reset_index()  # make sure indexes pair with number of rows
    
    for index,row in df.iterrows():
        paper1 = row['S Dos Santos(2010)']
        paper2 = row['S Wang & Li(2014)']
        paper3 = row['S Ahmadian (2021)']
        paper1 = float(paper1)
        paper2 = float(paper2)
        paper3 = float(paper3)

        best = 100000000
        best = min(paper1,paper2,paper3)

        if file_name == "LSN5N7":
            n5 = float(row['N5'])
            n7 = float(row['N7 (apos N5)'])
        else:
            s = float(row['S'])
        s0 = float(row['S0'])
        earliness0 = float(row['earliness0'])
        tardiness0 = float(row['tardiness0'])
        earliness = float(row['earliness'])
        tardiness = float(row['tardiness'])

        if file_name == "LSN5N7":
            n5 = ((best - n5)/best)*100
            n7 = ((best - n7)/best)*100
        else:
            s = ((best - s)/best)*100
        s0 = ((best - s0)/best)*100
        earliness0 = ((best - earliness0)/best)*100
        earliness = ((best - earliness)/best)*100
        tardiness0 = ((best - tardiness0)/best)*100
        tardiness = ((best - tardiness)/best)*100
        paper1 = ((best - paper1)/best)*100
        paper2 = ((best - paper2)/best)*100
        paper3 = ((best - paper3)/best)*100

        if file_name == "LSN5N7":
            df.at[index,'N5'] = "{:.2f}".format(n5) + '%'
            df.at[index,'N7 (apos N5)'] = "{:.2f}".format(n7) + '%'
        else:
            df.at[index,'S'] = "{:.2f}".format(s) + '%'
        df.at[index,'S0'] = "{:.2f}".format(s0) + '%'
        df.at[index,'earliness0'] = "{:.2f}".format(earliness0) + '%'
        df.at[index,'earliness'] = "{:.2f}".format(earliness) + '%'
        df.at[index,'tardiness0'] = "{:.2f}".format(tardiness0) + '%'
        df.at[index,'tardiness'] = "{:.2f}".format(tardiness) + '%'
        df.at[index, 'S Dos Santos(2010)'] = "{:.2f}".format(paper1) + '%'
        df.at[index, 'S Wang & Li(2014)'] = "{:.2f}".format(paper2) + '%'    
        df.at[index,'S Ahmadian (2021)'] = "{:.2f}".format(paper3) + '%'

        df.to_csv('a.csv',encoding = 'utf-8',index=False)



def compare():
    pd.options.display.max_rows = 9999
    df = pd.read_csv('compare.csv')
    df = df.reset_index()  # make sure indexes pair with number of rows
    
    i = 1
    for index,row in df.iterrows():
        v1 = row['S']
        v2 = row['S1']
        v1 = float(v1)
        v2 = float(v2)
        d = v1 - v2
        with open('resultadosdiferenca.csv','a',newline='') as planilha:
            linha = [d]
            w = writer(planilha)
            w.writerow(linha)
            planilha.close()


            
def main():
    compare()

if __name__ == '__main__':
    main()





