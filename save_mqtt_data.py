from datetime import datetime
import paho.mqtt.client as mqtt

broker_address = "127.0.0.1"
port = 1883
buffer = []


def save_file(filename):
    with open(f"save/{filename}.csv", "w") as file:
        file.write("x,y,z\n")
        file.write("".join(buffer))

    buffer.clear()


def on_message(client, userdata, message):
    topic = message.topic
    payload = message.payload.decode("utf-8")
    filename = userdata['config']["filename"]

    if payload == "end":
        save_file(filename)
        client.disconnect()
    else:
        buffer.append(payload)


def user_input(text, default=""):
    result = input(text)
    if len(result) == 0:
        result = default

    return result


def main():
    client = mqtt.Client()
    client.on_message = on_message
    client.connect(broker_address, port)
    time_now = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = user_input(
        "檔案名稱(file name)： ", f'{time_now}')
    freq = int(user_input("Sensor收集頻率(Freqency)： ", 1000))
    second = float(user_input("Sensor總收集秒數(Seconds)： ", 5))
    topic = f"{freq},{second},{time_now.split('_')[1]}"
    config_data = {'filename': filename}
    client.user_data_set({'config': config_data})
    client.subscribe(time_now.split('_')[1])
    client.publish("task", topic)

    print(
        f"Start write {filename} with topic {time_now.split('_')[1]}")
    client.loop_forever()
    print(f"Finished {filename}")


if __name__ == "__main__":
    main()
