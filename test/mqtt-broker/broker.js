const aedes = require('aedes')();
const net = require('net');

// 1883 es el puerto estándar MQTT
const PORT = process.env.PORT || 1883;

// Crear servidor TCP que manejará las conexiones MQTT
const server = net.createServer(aedes.handle);

/*
server.listen(PORT, function () {
  console.log('🚀 Broker MQTT corriendo en puerto', PORT);
});
*/

server.listen(PORT, '0.0.0.0', () => {
  console.log('🚀 Broker MQTT corriendo en puerto', PORT);
});

// Logs básicos
aedes.on('client', (client) => {
  console.log('📡 Cliente conectado:', client ? client.id : 'Desconocido');
});

aedes.on('publish', (packet, client) => {
  if (client) {
    console.log(`📩 Mensaje recibido en tópico '${packet.topic}': ${packet.payload.toString()}`);
  }
});
