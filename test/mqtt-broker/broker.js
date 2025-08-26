const aedes = require('aedes')();
const http = require('http');
const ws = require('websocket-stream');
const fs = require('fs');
const path = require('path');

const PORT = process.env.PORT || 3000;

let clients = []; // conexiones SSE (navegadores escuchando)

// Servidor HTTP
const server = http.createServer((req, res) => {
  if (req.url === '/') {
    // Página HTML
    const filePath = path.join(__dirname, 'public', 'index.html');
    fs.readFile(filePath, (err, data) => {
      if (err) {
        res.writeHead(500);
        res.end("Error cargando index.html");
      } else {
        res.writeHead(200, { "Content-Type": "text/html" });
        res.end(data);
      }
    });
  } else if (req.url === '/events') {
    // Endpoint SSE
    res.writeHead(200, {
      'Content-Type': 'text/event-stream',
      'Cache-Control': 'no-cache',
      'Connection': 'keep-alive',
    });
    res.write('\n');
    clients.push(res);

    req.on('close', () => {
      clients = clients.filter(client => client !== res);
    });
  } else {
    res.writeHead(404);
    res.end("Not found");
  }
});

// Enlazar WebSocket al broker MQTT
ws.createServer({ server }, aedes.handle);

server.listen(PORT, '0.0.0.0', () => {
  console.log(`🚀 Broker MQTT (WebSocket) escuchando en puerto ${PORT}`);
});

// === Función para enviar logs a todos los navegadores conectados ===
function broadcastLog(message) {
  clients.forEach(client => {
    client.write(`data: ${message}\n\n`);
  });
}

// Logs de clientes conectados
aedes.on('client', (client) => {
  const msg = `📡 Cliente conectado: ${client ? client.id : 'Desconocido'}`;
  console.log(msg);
  broadcastLog(msg);
});

// Logs de mensajes publicados
aedes.on('publish', (packet, client) => {
  if (client) {
    const msg = `📩 Mensaje en '${packet.topic}' por '${client.id}': ${packet.payload.toString()}`;
    console.log(msg);
    broadcastLog(msg);
  }
});

// Logs de desconexiones
aedes.on('clientDisconnect', (client) => {
  const msg = `❌ Cliente desconectado: ${client ? client.id : 'Desconocido'}`;
  console.log(msg);
  broadcastLog(msg);
});
