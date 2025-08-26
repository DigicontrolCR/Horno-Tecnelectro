const aedes = require('aedes')();
const http = require('http');
const ws = require('websocket-stream');
const WebSocket = require('ws'); // ← Añadir esta dependencia
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
  } else if (req.url === '/status') {
    // Endpoint de estado para verificar que el broker funciona
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({
      status: 'online',
      clients: aedes.connectedClients,
      timestamp: new Date().toISOString()
    }));
  } else {
    res.writeHead(404);
    res.end("Not found");
  }
});

// ===== SOPORTE DUAL =====

// 1. MQTT ESTÁNDAR sobre WebSocket (para clientes Node.js, Python, etc.)
ws.createServer({ server }, aedes.handle);

// 2. WEB SOCKET SIMPLE (para ESP32 y clientes básicos)
const wss = new WebSocket.Server({ 
  server,
  path: '/simple'  // Path especial para clientes simples
});

wss.on('connection', function connection(ws) {
  const clientId = `simple_${Math.random().toString(36).substr(2, 9)}`;
  const msg = `📡 Cliente WebSocket simple conectado: ${clientId}`;
  console.log(msg);
  broadcastLog(msg);

  ws.on('message', function incoming(message) {
    try {
      const data = message.toString();
      const msg = `📩 Mensaje simple de ${clientId}: ${data}`;
      console.log(msg);
      broadcastLog(msg);

      // Procesar diferentes formatos de mensaje
      if (data.startsWith('{')) {
        // Formato JSON: {"topic": "mi/topico", "message": "mi mensaje"}
        try {
          const parsed = JSON.parse(data);
          if (parsed.topic && parsed.message) {
            aedes.publish({
              topic: parsed.topic,
              payload: parsed.message,
              qos: 0,
              retain: false
            });
          }
        } catch (e) {
          console.log('❌ Error parseando JSON:', e.message);
        }
      } else if (data.includes('|')) {
        // Formato: "topico|mensaje"
        const [topic, payload] = data.split('|');
        aedes.publish({
          topic: topic.trim(),
          payload: payload.trim(),
          qos: 0,
          retain: false
        });
      } else {
        // Mensaje simple - publicar en tópico por defecto
        aedes.publish({
          topic: 'esp32/messages',
          payload: data,
          qos: 0,
          retain: false
        });
      }

      // Responder al cliente ESP32
      ws.send(JSON.stringify({
        status: 'received',
        clientId: clientId,
        message: data
      }));

    } catch (error) {
      console.error('❌ Error procesando mensaje:', error);
    }
  });

  ws.on('close', function() {
    const msg = `❌ Cliente WebSocket desconectado: ${clientId}`;
    console.log(msg);
    broadcastLog(msg);
  });

  // Enviar mensaje de bienvenida
  ws.send(JSON.stringify({
    status: 'connected',
    clientId: clientId,
    message: 'Bienvenido al broker MQTT WebSocket'
  }));
});

// ===== FUNCIONES DE LOGGING =====

function broadcastLog(message) {
  clients.forEach(client => {
    client.write(`data: ${message}\n\n`);
  });
}

// Logs de clientes MQTT conectados
aedes.on('client', (client) => {
  const msg = `📡 Cliente MQTT conectado: ${client ? client.id : 'Desconocido'}`;
  console.log(msg);
  broadcastLog(msg);
});

// Logs de mensajes publicados
aedes.on('publish', (packet, client) => {
  if (client) {
    const msg = `📩 Mensaje MQTT en '${packet.topic}' por '${client.id}': ${packet.payload.toString()}`;
    console.log(msg);
    broadcastLog(msg);
  } else {
    // Mensajes de sistema o de clientes simples
    const msg = `📩 Mensaje en '${packet.topic}': ${packet.payload.toString()}`;
    console.log(msg);
    broadcastLog(msg);
  }
});

// Logs de desconexiones
aedes.on('clientDisconnect', (client) => {
  const msg = `❌ Cliente MQTT desconectado: ${client ? client.id : 'Desconocido'}`;
  console.log(msg);
  broadcastLog(msg);
});

// Manejo de errores
aedes.on('clientError', (client, err) => {
  const msg = `❌ Error en cliente ${client.id}: ${err.message}`;
  console.log(msg);
  broadcastLog(msg);
});

// ===== INICIAR SERVIDOR =====

server.listen(PORT, '0.0.0.0', () => {
  console.log(`
🚀 Broker MQTT WebSocket ejecutándose en puerto ${PORT}
=======================================================

🔗 Endpoints disponibles:
- MQTT WebSocket estándar: ws://localhost:${PORT}
- WebSocket simple (ESP32): ws://localhost:${PORT}/simple
- Página web: http://localhost:${PORT}
- SSE Logs: http://localhost:${PORT}/events
- Status API: http://localhost:${PORT}/status

📡 Esperando conexiones de clientes...
`);
});

// Manejo de cierre graceful
process.on('SIGINT', () => {
  console.log('\n🛑 Cerrando broker gracefulmente...');
  server.close(() => {
    console.log('✅ Broker cerrado correctamente');
    process.exit(0);
  });
});
