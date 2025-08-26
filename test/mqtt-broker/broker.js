const aedes = require('aedes')();
const http = require('http');
const ws = require('websocket-stream');

const PORT = process.env.PORT || 3000;

// Crear servidor HTTP con respuesta básica
const server = http.createServer((req, res) => {
  res.writeHead(200, { 'Content-Type': 'text/plain' });
  res.end('Broker MQTT (WebSocket) activo\n');
});

// Enlazar WebSocket al broker MQTT
ws.createServer({ server }, aedes.handle);

// Iniciar servidor
server.listen(PORT, '0.0.0.0', () => {
  console.log(`🚀 Broker MQTT (WebSocket) escuchando en puerto ${PORT}`);
});

// Logs de clientes conectados
aedes.on('client', (client) => {
  console.log('📡 Cliente conectado:', client ? client.id : 'Desconocido');
});

// Logs de mensajes publicados
aedes.on('publish', (packet, client) => {
  if (client) {
    console.log(`📩 Mensaje publicado en '${packet.topic}' por '${client.id}': ${packet.payload.toString()}`);
  }
});

// Logs de desconexiones
aedes.on('clientDisconnect', (client) => {
  console.log('❌ Cliente desconectado:', client ? client.id : 'Desconocido');
});
