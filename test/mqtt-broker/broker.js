const aedes = require('aedes')();
const net = require('net');

const PORT = 1883; // Puerto estándar MQTT

// Crear servidor TCP que manejará las conexiones MQTT
const server = net.createServer(aedes.handle);

server.listen(PORT, function () {
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
