import paho.mqtt.client as mqtt
import requests


# Define as funções de callback
def on_connect(client, userdata, flags, rc):
    print(f"Conectado com código {rc}")
    client.subscribe("veiculosro/placa")  # Substitua pelo seu tópico

def on_message(client, userdata, msg):
    print(f"{msg.payload.decode()}")
    placa = msg.payload.decode()
    print(f"Placa recebida: {placa}")
    
   # Ajuste na interpolação da URL
    url = f"http://127.0.0.1:5000/api/users/placa?placa={placa}"    
    
    # Fazer a requisição GET
    response = requests.get(url)

    # Exibir o conteúdo da resposta
    print(response.content)
    

# Cria um cliente MQTT
client = mqtt.Client()

# Configura as funções de callback
client.on_connect = on_connect
client.on_message = on_message

# Conecta-se ao broker
client.connect("broker.emqx.io", 1883, 60)  # Substitua pelo endereço e porta do seu broker

# Mantém o cliente em execução para receber mensagens
client.loop_forever()
