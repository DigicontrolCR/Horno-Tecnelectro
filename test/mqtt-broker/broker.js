const aedes = require('aedes')();
const http = require('http');
const ws = require('websocket-stream');
const WebSocket = require('ws');
const fs = require('fs');
const path = require('path');

const PORT = process.env.PORT || 3000;

let clients = []; // conexiones SSE (navegadores escuchando)



// Servidor HTTP
const server = http.createServer((req, res) => {
  // Habilitar CORS para todas las rutas
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

  // Manejar preflight OPTIONS
  if (req.method === 'OPTIONS') {
    res.writeHead(200);
    res.end();
    return;
  }

  const publicPath = path.join(__dirname, 'public');

  // Manejar archivos estáticos
  if (req.url.match(/\.(css|js|png|jpg|jpeg|gif|svg|ico)$/i)) {
    const filePath = path.join(publicPath, req.url);
    const ext = path.extname(filePath);

    const contentTypes = {
      '.css': 'text/css',
      '.js': 'application/javascript',
      '.png': 'image/png',
      '.jpg': 'image/jpeg',
      '.jpeg': 'image/jpeg',
      '.gif': 'image/gif',
      '.svg': 'image/svg+xml'
    };

    fs.readFile(filePath, (err, data) => {
      if (err) {
        res.writeHead(404);
        res.end('Archivo no encontrado');
      } else {
        res.writeHead(200, {
          'Content-Type': contentTypes[ext] || 'application/octet-stream'
        });
        res.end(data);
      }
    });
    return; // Importante: salir después de servir el archivo estático
  }

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
      'Access-Control-Allow-Origin': '*'
    });
    res.write('\n');
    clients.push(res);

    req.on('close', () => {
      clients = clients.filter(client => client !== res);
    });
  } else if (req.url === '/status') {
    // Endpoint de estado
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({
      status: 'online',
      clients: aedes.connectedClients,
      timestamp: new Date().toISOString(),
      environment: process.env.NODE_ENV || 'development'
    }));
  } else if (req.url === '/api/message' && req.method === 'POST') {
    // ===== NUEVO ENDPOINT PARA ESP32 =====
    let body = '';
    req.on('data', chunk => {
      body += chunk.toString();
    });

    req.on('end', () => {
      try {
        console.log('📨 Mensaje recibido via API:', body);

        const data = JSON.parse(body);

        if (data.topic && data.message) {
          // Publicar en el broker MQTT
          aedes.publish({
            topic: data.topic,
            payload: data.message,
            qos: 0,
            retain: false
          });

          // ===== MODIFICACIÓN: Preparar respuesta para ESP32 =====
          let responseMessage = data.message;

          // Si es un comando para ESP32, enviarlo de forma que lo detecte
          if (data.topic === 'esp32/control' &&
            (data.message === 'led_on' || data.message === 'led_off' || data.message === 'led_toggle')) {
            responseMessage = `COMMAND:${data.message}`; // Prefijo especial
            console.log(`🔧 Enviando comando ESP32: ${responseMessage}`);
          }
          // ===== FIN DE MODIFICACIÓN =====

          res.writeHead(200, {
            'Content-Type': 'application/json',
            'Access-Control-Allow-Origin': '*'
          });

          res.end(JSON.stringify({
            status: 'success',
            message: 'Mensaje publicado en MQTT',
            topic: data.topic,
            received: responseMessage // ← Usar el mensaje modificado
          }));

          console.log(`📤 Publicado en ${data.topic}: ${data.message}`);

        } else {
          res.writeHead(400, {
            'Content-Type': 'application/json',
            'Access-Control-Allow-Origin': '*'
          });
          res.end(JSON.stringify({
            status: 'error',
            message: 'Formato inválido. Use: {"topic":"x","message":"y"}'
          }));
        }
      } catch (error) {
        res.writeHead(500, {
          'Content-Type': 'application/json',
          'Access-Control-Allow-Origin': '*'
        });
        res.end(JSON.stringify({
          status: 'error',
          message: 'Error procesando JSON: ' + error.message
        }));
      }
    });
  } else {
    res.writeHead(404, {
      'Content-Type': 'application/json',
      'Access-Control-Allow-Origin': '*'
    });
    res.end(JSON.stringify({
      status: 'error',
      message: 'Endpoint no encontrado',
      availableEndpoints: ['/', '/events', '/status', '/api/message']
    }));
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

wss.on('connection', function connection(ws, req) {
  const clientId = `simple_${Math.random().toString(36).substr(2, 9)}`;
  const clientIp = req.socket.remoteAddress;
  const msg = `📡 Cliente WebSocket simple conectado: ${clientId} desde ${clientIp}`;
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

      // Responder al cliente
      ws.send(JSON.stringify({
        status: 'received',
        clientId: clientId,
        message: data,
        timestamp: new Date().toISOString()
      }));

    } catch (error) {
      console.error('❌ Error procesando mensaje:', error);
    }
  });

  ws.on('close', function () {
    const msg = `❌ Cliente WebSocket desconectado: ${clientId}`;
    console.log(msg);
    broadcastLog(msg);
  });

  // Enviar mensaje de bienvenida
  ws.send(JSON.stringify({
    status: 'connected',
    clientId: clientId,
    message: 'Bienvenido al broker MQTT WebSocket',
    endpoints: {
      mqtt: `wss://${req.headers.host}`,
      simple: `wss://${req.headers.host}/simple`,
      api: `https://${req.headers.host}/api/message`
    }
  }));
});

// ===== FUNCIONES DE LOGGING =====

function broadcastLog(message) {
  const timestamp = new Date().toISOString();
  const logMessage = `[${timestamp}] ${message}`;

  clients.forEach(client => {
    client.write(`data: ${logMessage}\n\n`);
  });
}

// Logs de clientes MQTT conectados
aedes.on('client', (client) => {
  const msg = `📡 Cliente MQTT conectado: ${client ? client.id : 'Desconocido'}`;
  console.log(msg);
  broadcastLog(msg);
});

// Logs de mensajes publicados
// Logs de mensajes publicados
aedes.on('publish', (packet, client) => {
  // ===== FILTRO: Ignorar mensajes del sistema $SYS/ =====
  if (packet.topic.startsWith('$SYS/')) {
    return; // No procesar mensajes del sistema
  }
  // ===== FIN DEL FILTRO =====

  if (client) {
    const msg = `📩 Mensaje MQTT en '${packet.topic}' por '${client.id}': ${packet.payload.toString()}`;
    console.log(msg);
    broadcastLog(msg);
  } else {
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

// ===== INICIAR SERVIDOR =====

server.listen(PORT, '0.0.0.0', () => {
  const domain = process.env.RENDER_EXTERNAL_HOSTNAME || `localhost:${PORT}`;

  console.log(`
🚀 Broker MQTT WebSocket ejecutándose en puerto ${PORT}
=======================================================

🔗 Endpoints disponibles:
- MQTT WebSocket estándar: wss://${domain}
- WebSocket simple: wss://${domain}/simple
- API HTTP POST: https://${domain}/api/message
- Página web: https://${domain}
- SSE Logs: https://${domain}/events
- Status: https://${domain}/status

📡 Protocolos soportados:
1. MQTT sobre WebSocket (clientes avanzados)
2. WebSocket simple (ESP32, mensajes de texto)
3. HTTP POST API (ESP32, HTTPS seguro)

🌐 Ambiente: ${process.env.NODE_ENV || 'development'}
🕐 Iniciado: ${new Date().toISOString()}

✅ Listo para recibir conexiones...
`);
});

// Manejo de errores no capturados
process.on('uncaughtException', (error) => {
  console.error('❌ Error no capturado:', error);
});

process.on('unhandledRejection', (reason, promise) => {
  console.error('❌ Promise rechazada:', reason);
});

// Manejo de cierre graceful
process.on('SIGINT', () => {
  console.log('\n🛑 Cerrando broker gracefulmente...');
  server.close(() => {
    console.log('✅ Broker cerrado correctamente');
    process.exit(0);
  });
});
