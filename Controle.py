
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
    