class Teclado:
    qtdLinhas = 0
    qtdColunas = 0
    pinLinhas = []
    pinColunas = []
    configuracaoTeclas = []
    
    def __init__(self):
        print("Iniciando Teclado")
        self.configuraTeclas()
    
    def configuraTeclas(self):
        try:
            # Abrindo o Arquivo de configuração das teclas
            file = open("./Configuracoes_Iniciais/Configuracao_Do_Teclado.txt", "r")

            # Lendo e armazenando os pinos de conexão das linhas
            linha = file.readline()
            array = linha.split("[")[1].strip().replace("];", "")
            for number in array:
                if number != " ":
                    self.pinLinhas.append(int(number))

            # Lendo e armazenando os pinos de conexão das colunas
            linha = file.readline()
            array = linha.split("[")[1].strip().replace("];", "")
            for number in array:
                if number != " ":
                    self.pinColunas.append(int(number))
            
            # Lendo e armazenando a quantidade de linhas
            linha = file.readline()
            self.qtdLinhas = int(linha.split(":")[1].strip().replace(";", ""))

            # Lendo e armazenando a quantidade de colunas
            linha = file.readline()
            self.qtdColunas = int(linha.split(":")[1].strip().replace(";", ""))

            # Lendo e armazenando a matriz de teclas correspondentes do teclado
            linha = file.readline()
            array = linha.split("[")[1].replace("]","").split(";")
            for l in array:
                if(l.split(" ") != [""] ):
                    self.configuracaoTeclas.append(l.split(" "))
            print("Teclado Inicializado com a seguinte configuração de teclas: ")
            print(self.configuracaoTeclas)
                
            
        except Exception:
            print("Exceção lançada na configuração do teclado")
    
    def mostraTeclado(self):
        print("Numero de linhas: ", self.qtdLinhas)
        print("Numero de colunas: ", self.qtdColunas)
        print("Pinos das linhas: ", self.pinLinhas)
        print("Pinos das colunas: ", self.pinColunas)
        print("Matriz de correspondencia: ", self.configuracaoTeclas)

