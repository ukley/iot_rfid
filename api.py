from flask import Flask, jsonify, request
from tinydb import TinyDB, Query
from datetime import datetime, date, time, timedelta
import paho.mqtt.client as mqtt
import requests



app = Flask(__name__)

# Exemplo de dados para simular o banco de dados
data = {
    "users": [
        {"tag":"4A:92:BC:3F","proprietario": "Felipe Costa", "car_placa": "ABC1234", "car_modelo": "Fiat Uno","cor":"Azul"},
        {"tag":"1E:34:89:7A","proprietario": "Carlos Eduardo da Silva ", "car_placa": "PWL1120", "car_modelo": "Fiat Palio","cor":"Preto"},
        {"tag":"D5:56:4C:A1","proprietario": "Mariana Oliveira Santos", "car_placa": "FOO3290", "car_modelo": "Ford Ka","cor":"Branco"},
    ]
}


@app.route('/api/users/placa', methods=['GET'])
def get_dados_veiculo():
    # Recuperar o valor do parâmetro de consulta
    
    plate = request.args.get('placa')
    print(plate)

    if not plate:
        return jsonify({
            "status": "error",
            "message": "Parâmetro 'plate' é obrigatório"
        }), 400

    # Procurar o usuário com base na placa
    user = next((user for user in data["users"] if user["car_placa"] == plate), None)

    if not user:
        return jsonify({
            "status": "error",
            "message": "Usuário não encontrado com a placa fornecida"
        }), 404

    # Retornar os dados do usuário
    return jsonify({       
        "data": user
    })
    
def receber_placa(placa):
    db = TinyDB('db.json')
                                    
    db.insert({ 'placa': placa, 'data_hora': str(datetime.now())})       

# Define as funções de callback
def on_connect(client, userdata, flags, rc):
    print(f"Conectado com código {rc}")
    client.subscribe("veiculosro/placa")  # Substitua pelo seu tópico

def on_message(client, userdata, msg):
    print(f"{msg.payload.decode()}")
    placa = msg.payload.decode()
    
    receber_placa(placa)     
    
# Cria um cliente MQTT
client = mqtt.Client()

# Configura as funções de callback
client.on_connect = on_connect
client.on_message = on_message

# Conecta-se ao broker
client.connect("broker.emqx.io", 1883, 60)  # Substitua pelo endereço e porta do seu broker

# Mantém o cliente em execução para receber mensagens
client.loop_forever()


if __name__ == '__main__':
    app.run(debug=True)
