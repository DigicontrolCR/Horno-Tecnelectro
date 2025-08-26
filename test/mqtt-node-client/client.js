const mqtt = require('mqtt');

// Conectar al broker en Render usando WebSockets
const client = mqtt.connect('ws://horno-tecnelectro.onrender.com');

client.on('connect', () => {
  console.log('✅ Cliente Node.js conectado al broker');

  client.subscribe('esp32/out');
  client.subscribe('node/out');

  setInterval(() => {
    const msg = "Hola desde Node.js!";
    client.publish('node/out', msg);
    console.log("Publicado en node/out:", msg);
  }, 5000);
});

client.on('message', (topic, message) => {
  console.log(`📩 Mensaje recibido en [${topic}]: ${message.toString()}`);
});
