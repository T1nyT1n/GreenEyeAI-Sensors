from flask import Flask, request, jsonify
from threading import Lock

app = Flask(__name__)

# Текущее значение для нагрузки (0-255)
load_value = 0
lock = Lock()

# Хранилище последних показаний датчиков
sensor_data = []
sensor_lock = Lock()

# Эндпоинт для ESP (получить текущее значение нагрузки)
@app.route('/api/load', methods=['GET'])
def get_load():
    with lock:
        return jsonify(value=load_value)

# Эндпоинт для изменения нагрузки (POST-запрос)
@app.route('/api/load', methods=['POST'])
def set_load():
    data = request.get_json()
    if not ((type(data) is dict) and ("value" in data)):
        return jsonify(error='missing value'), 400

    try:
        ival = int(data['value'])
        if ival < 0:
            ival = 0
        if ival > 255:
            ival = 255
    except (ValueError, TypeError):
        return jsonify(error='invalid value'), 400

    with lock:
        global load_value
        load_value = ival

    return jsonify(status='ok', value=load_value)

# Эндпоинт для приёма показаний с датчиков (ESP отправляет POST)
@app.route('/api/sensors', methods=['POST'])
def post_sensors():
    data = request.get_json()
    bad = jsonify(status='error'), 400
    if type(data) is not dict: return bad
    if "value" not in data: return bad
    if type(data["value"]) is not float: return bad

    with sensor_lock:
        sensor_data.append(data)
        if len(sensor_data) > 1000:
            del sensor_data[0]
        return jsonify(status='ok')

# Для просмотра последних показаний (веб-интерфейс)
@app.route('/api/sensors', methods=['GET'])
def get_sensors():
    with sensor_lock:
        # последние 10 записей
        return jsonify(sensor_data[-10:])

if __name__ == '__main__':
    #(Запуск без чтения из stdin) - Было "threading.Thread(target=console_input, daemon=True).start()"
    app.run(host='0.0.0.0', port=13000, debug=False)
