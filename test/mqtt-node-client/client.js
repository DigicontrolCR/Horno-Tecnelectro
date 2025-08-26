const mqtt = require('mqtt');

// Conectar al broker local
const client = mqtt.connect('mqtt://localhost:1883');

// Al conectar
client.on('connect', () => {
  console.log('✅ Cliente Node.js conectado al broker');

  // Suscribirse a ambos tópicos
  client.subscribe('esp32/out');
  client.subscribe('node/out');

  // Publicar cada 5 segundos en node/out
  setInterval(() => {
    const msg = "Hola desde Node.js!";
    client.publish('node/out', msg);
    console.log("Publicado en node/out:", msg);
  }, 5000);
});

// Recibir mensajes
client.on('message', (topic, message) => {
  console.log(`📩 Mensaje recibido en [${topic}]: ${message.toString()}`);
});
