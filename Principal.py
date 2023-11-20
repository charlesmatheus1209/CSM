from Teclado import *
from Display import *
from Controle import *
import time
import threading
import random

def QuantidadeDePulsos():
    p = random.randrange(0, 200)
    print("Pulsos: ", p)
    return p

def CalculaVazao(pulsos, intervaloReal):
    # Frequencia de pulsos = 6,6 * Vazão
    # Vazão = Frequencia/6.6
    frequencia = (1/ intervaloReal) * pulsos
    print("Frequencia: ", frequencia, "pulsos/seg")
    
    vazao = frequencia/6.6
    print("Vazão: ", vazao, "L/min")
    return vazao
    
    
    
    
Tec = Teclado()
Tec.mostraTeclado()

Disp = Display()
Disp.mostraMensagem("Iniciando!")


# Configuração dos Pinos


# Variáveis gerais
pulsosDoIntervalo = 0
intervalo = 0.1 #s

# Inicia Loop
i = 0
inicio = time.time()
acumulador = 0
volumeDesejado = 50
while True:
    i += 1
    iniciaControle = True
    # 1 - Lê teclado
    # 2 - Escreve Display
    # 3 - Se tudo OK, Inicia o controle
    
    if(iniciaControle):
        time.sleep(intervalo)
        intervaloReal = time.time() - inicio
        print("Intervalo real: ", intervaloReal)
        inicio = time.time()
        pulsosDoIntervalo = QuantidadeDePulsos()
        Vazao = CalculaVazao(pulsosDoIntervalo, intervaloReal)
        acumulador += (Vazao/60) * intervaloReal
        print("Acumulador: ", acumulador)
        if acumulador >= volumeDesejado * 0.97:
            print("FIMMMM")
            break
        
        
        
    