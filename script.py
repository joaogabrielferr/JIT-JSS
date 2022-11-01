import subprocess
from csv import writer

# names = ['tight-equal','loose-equal','loose-tard','tight-tard']
# sizes = [['10x2','10x5','10x10'],['15x2','15x5','15x10'],['20x2','20x5','20x10']]

names = ['tight-equal','loose-equal','tight-tard','loose-tard']
sizes = [['10x2','10x5','10x10'],['15x2','15x5','15x10'],['20x2','20x5','20x10']]

with open('output.txt','r+') as file:
    for i in range(0,1): #len(sizes)
        for k in range(0,1): #len(names)
            for j in range(0,len(sizes)): #len(sizes)
                instance_size = sizes[i][j]
                
                for ins in range(1,3):
                    instance_name = names[k]
                    print("NOVA EXECUCAO,", instance_name,",",instance_size,",",ins)
                    file_name = 'C:\\Users\\joaog\Desktop\\JIT-JSS\\JIT-JSS\\instances\\' + str(instance_name) + '\\test' + str(ins) + '_' + str(instance_size) + '.txt'
                    p = subprocess.run(['tcc',file_name],stdout = file,text=True);
                    file.seek(0)
                    data = file.readlines()
                    # if len(data) != 7:
                    #     print("WRONG OUTPUT")
                    #     exit()
                    print(data)
                    # s = float(data[0])
                    # s0 = float(data[1])
                    # iter = int(data[2])
                    # earliness0 = float(data[3])
                    # tardiness0 = float(data[4])
                    # earliness = float(data[5])
                    # tardiness = float(data[6])
                    # print(s," ",s0," ",iter," ",earliness0," ",tardiness0," ",earliness," ",tardiness)
                    file.seek(0)
                    file.truncate()
                    
                    #sequence: S,S0,iteracoes,earliness0,tardiness0,earliness,tardiness

                    with open('resultados.csv','a',newline='') as planilha:
                        nome_instancia = instance_name + '_' + instance_size + '_' + str(ins); 
                        linha = [nome_instancia,s,'_',s0,iter,earliness0,tardiness0,earliness,tardiness,0,0,0]
                        w = writer(planilha)
                        # w.writerow(header)
                        w.writerow(linha)
                        # w.writerow(linha2)
                        # w.writerow(linha3)
                        planilha.close()
        

# p1 = subprocess.run(['tcc','C:\\Users\\joaog\Desktop\\JIT-JSS\\JIT-JSS\\instances\\tight-equal\\test1_10x2.txt'],
# capture_output=True)

# linha1 = ['nome instancia 1',1000,0,0,0,0,777.10,789.77,10000,100000,100000]
# linha2 = ['nome instancia 2',1000,0,0,0,0,777.10,789.77,10000,100000,100000]
# linha3 = ['nome instancia 3',1000,0,0,0,0,777.10,789.77,10000,100000,100000]
# with open('resultados.csv','a',newline='') as file:
#     w = writer(file)
#     # w.writerow(header)
#     w.writerow(linha1)
#     # w.writerow(linha2)
#     # w.writerow(linha3)
#     file.close()

# # print(p1.stdout.decode())




