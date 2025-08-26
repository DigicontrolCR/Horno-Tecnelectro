const aedes = require('aedes')();
const http = require('http');
const ws = require('websocket-stream');

const PORT = process.env.PORT || 3000;

// Crear servidor HTTP
const server = http.createServer();

// Enlazar WebSocket al broker
ws.createServer({ server }, aedes.handle);

server.listen(PORT, '0.0.0.0', () => {
  console.log(`🚀 Broker MQTT (WebSocket) escuchando en puerto ${PORT}`);
});

// Logs
aedes.on('client', (client) => {
  console.log('📡 Cliente conectado:', client ? client.id : 'Desconocido');
});

aedes.on('publish', (packet, client) => {
  if (client) {
    console.log(`📩 Mensaje publicado en '${packet.topic}': ${packet.payload.toString()}`);
  }
});
