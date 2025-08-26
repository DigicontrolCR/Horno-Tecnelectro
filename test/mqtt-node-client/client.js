const mqtt = require('mqtt');

// Conectar al broker WebSocket (usar wss si Render tiene SSL)
const client = mqtt.connect('wss://horno-tecnelectro.onrender.com', {
  reconnectPeriod: 2000,
  connectTimeout: 30 * 1000
});

client.on('connect', () => {
  console.log('✅ Cliente Node.js conectado al broker');

  // Suscribirse a tópicos
  client.subscribe('esp32/out');
  client.subscribe('node/out');

  // Publicar cada 5 segundos
  setInterval(() => {
    const msg = "Hola desde Node.js!";
    client.publish('node/out', msg);
    console.log("📤 Publicado en node/out:", msg);
  }, 5000);
});

client.on('message', (topic, message) => {
  console.log(`📩 Mensaje recibido en [${topic}]: ${message.toString()}`);
});

client.on('error', (err) => {
  console.error('❌ Error del cliente MQTT:', err.message);
});
