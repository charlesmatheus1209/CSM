import requests

class InterfaceWeb:
    te = int()
    
    def http_get(self, url, payload):
        try:
            resposta = requests.get(url, verify=False, params=payload)
            resposta.raise_for_status()  # Verifica se ocorreu algum erro na requisição

            print(f"Status code: {resposta.status_code}")
            print("Resposta:")
            print(resposta.text)  # Exibe o conteúdo da resposta no console
        except Exception as error:
            print("Erro na requisição: ", error)

    def http_post(self, url, obj):
        try:
            resposta = requests.post(url,json = obj, verify=False)
            resposta.raise_for_status()  # Verifica se ocorreu algum erro na requisição

            print(f"Status code: {resposta.status_code}")
            print("Resposta:")
            print(resposta.text)  # Exibe o conteúdo da resposta no console
        except Exception as error:
            print("Erro na requisição: ", error)
    
