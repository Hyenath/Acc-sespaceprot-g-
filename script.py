import paho.mqtt.client as mqtt
import mysql.connector

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("topic/serrure")

def on_message(client, userdata, msg):
    data = str(msg.payload.decode("utf-8"))
    print("Received RFID tag:", data)
    try:
        save_to_database(data)
    except Exception as e:
        print("Error saving data to database:", e)

def save_to_database(rfid_tag):
    print("Trying to connect to the database...")
    try:
        connection = mysql.connector.connect(
            host="ip",
            user="user",
            password="password",
            database="database"
        )
        print("Connected to the database successfully!")
        cursor = connection.cursor()
        query = "INSERT INTO access_control (rfid_tag) VALUES (%s)"
        cursor.execute(query, (rfid_tag,))
        connection.commit()
        cursor.close()
        connection.close()
        print("Data inserted successfully!")
    except Exception as e:
        print("An error occurred:", e)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)
client.loop_forever()


