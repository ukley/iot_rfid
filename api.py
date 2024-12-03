from flask import Flask, jsonify, request
from tinydb import TinyDB, Query
from datetime import datetime, date, time, timedelta


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
    
@app.route('/api/dado', methods=['GET'])
def receber_placa():
    db = TinyDB('db.json')
    try:
        # Obtém o parâmetro 'placa' da requisição
        plate = request.args.get('placa')
        print(f"Placa recebida: {plate}")      
                
        db.insert({ 'tag':"4A:92:BC:3F", 'placa': plate, 'data_hora': str(datetime.now())})

        # Verifica se o parâmetro 'placa' foi fornecido
        if not plate:
            return jsonify({
                "status": "error",
                "message": "Parametro 'placa' e obrigatorio"
            }), 400

        # render db.json
        # db = TinyDB('db.json')
        print(db.all())
        print(db.search(Query().placa == plate))
       
        
        # Procura o usuário com base na placa
        user = next((user for user in data["users"] if user["car_placa"] == plate), None)

        # Se não encontrar o usuário, retorna erro
        if not user:
            return jsonify({
                "status": "error",
                "message": "Placa não encontrado "
            }), 404

        # Retorna os dados do usuário
        return jsonify({           
            "data": user
        }), 200

    except Exception as e:
        # Em caso de erro, retorna uma mensagem com o erro
        return jsonify({
            "status": "error",
            "message": f"Erro no servidor: {str(e)}"
        }), 500    


if __name__ == '__main__':
    app.run(debug=True)
