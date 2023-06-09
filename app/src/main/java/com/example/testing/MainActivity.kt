package com.example.testing

//variables predeterminadas
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Button
import android.widget.Toast

//variables del proyecto
import android.content.Context
import android.util.Log
import android.widget.EditText
import org.eclipse.paho.android.service.MqttAndroidClient
import org.eclipse.paho.client.mqttv3.IMqttActionListener
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken
import org.eclipse.paho.client.mqttv3.IMqttToken
import org.eclipse.paho.client.mqttv3.MqttCallback
import org.eclipse.paho.client.mqttv3.MqttConnectOptions
import org.eclipse.paho.client.mqttv3.MqttException
import org.eclipse.paho.client.mqttv3.MqttMessage



// Verificar las variables publicas y privadas (que no interfiera con la llamada de modulos)

class MainActivity : AppCompatActivity() {

    // La variable del cliente se declara primero, fuera del onCreate

    private lateinit var mqttClient: MqttAndroidClient
    private lateinit var TopicInput: EditText

    // TAG
    companion object {
        const val TAG = "AndroidMqttClient"
    }

    private val options = MqttConnectOptions() // es privada, ver si no necesita ser global



    // Comienzo de onCreate

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        //Esto es un boton. Se encuentra la funcion por ID y lo que este despues
        // de setOnClickListener se ejecuta para el usuario (POR AHORA SOLO UN TOAST SENCILLO)

        connect(this) // llama a connect

        val buttonconnect = findViewById<Button>(R.id.button_connect)

        buttonconnect.setOnClickListener {
            Toast.makeText(this@MainActivity, "You clicked connect.", Toast.LENGTH_SHORT).show()
        }

        val text = TopicInput.text //Getting the text entered by user using the name of my edittext (TopicInput)

        subscribe(this text)  //quiero relacionar text con topic, asi el nombre que ingreso a traves del edittext se lo toma como topic, y se subscribe a este


        TopicInput = findViewById(R.id.TopicInput) // Encuentra el EditText por Id



        val buttonsubscribe = findViewById<Button>(R.id.button_subscribe) // Busca Id del boton

        buttonsubscribe.setOnClickListener {
            Toast.makeText(this@MainActivity, "You clicked subscribe.", Toast.LENGTH_SHORT).show()

            // Showing the user input
            Toast.makeText(this, text, Toast.LENGTH_SHORT).show()
        }


        val serverURI =
            "tcp://broker.emqx.io:1883" //se declaran las variables estaticas, sino las funciones no pueden usarlas
        mqttClient = MqttAndroidClient(this, serverURI, "kotlin_client")

    }

    // buscar que es toString


    // Fuera de onCreate, se establecen las funciones

    private fun connect(context: Context) {

        mqttClient.setCallback(object : MqttCallback {
            override fun messageArrived(topic: String?, message: MqttMessage?) {
                Log.d(TAG, "Receive message: ${message.toString()} from topic: $topic")
            }

            override fun connectionLost(cause: Throwable?) {
                Log.d(TAG, "Connection lost ${cause.toString()}")
            }

            override fun deliveryComplete(token: IMqttDeliveryToken?) {

            }
        })
        try {
            mqttClient.connect(options, context, object : IMqttActionListener {
                override fun onSuccess(asyncActionToken: IMqttToken?) {
                    Log.d(TAG, "Connection success")
                }

                override fun onFailure(asyncActionToken: IMqttToken?, exception: Throwable?) {
                    Log.d(TAG, "Connection failure")
                }
            })
        } catch (e: MqttException) {
            e.printStackTrace()
        }

    }

    private fun subscribe(topic: String, qos: Int = 1) { // topic: String, qos: Int = 1
        try {
            mqttClient.subscribe(topic, qos, null, object : IMqttActionListener {
                override fun onSuccess(asyncActionToken: IMqttToken?) {
                    Log.d(TAG, "Subscribed to $topic")
                }

                override fun onFailure(asyncActionToken: IMqttToken?, exception: Throwable?) {
                    Log.d(TAG, "Failed to subscribe $topic")
                }
            })
        } catch (e: MqttException) {
            e.printStackTrace()
        }


    }
}

















































