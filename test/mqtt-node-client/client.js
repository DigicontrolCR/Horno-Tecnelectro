const express = require('express');
const axios = require('axios');
const cors = require('cors');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3001;

// Configuración del broker principal
const BROKER_URL = 'https://horno-tecnelectro.onrender.com';
const API_ENDPOINT = `${BROKER_URL}/api/message`;

// Almacenamiento de datos del sensor
let sensorData = {
  fotoValue: '--',
  ledState: '--',
  wifiRssi: '--',
  lastUpdate: '--'
};

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.static('public'));

// Servir la interfaz web
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// API Proxy para enviar mensajes al broker principal
app.post('/api/send-message', async (req, res) => {
  try {
    const { topic, message } = req.body;

    console.log(`📤 Enviando mensaje al broker principal: ${message}`);

    const response = await axios.post(API_ENDPOINT, {
      topic: topic,
      message: message
    }, {
      headers: {
        'Content-Type': 'application/json'
      },
      timeout: 10000
    });

    res.json({
      success: true,
      data: response.data,
      message: 'Mensaje enviado correctamente'
    });

  } catch (error) {
    console.error('Error enviando mensaje:', error.message);
    res.status(500).json({
      success: false,
      message: error.response?.data || error.message
    });
  }
});

// Obtener datos del sensor
app.get('/api/sensor-data', (req, res) => {
  res.json({
    success: true,
    data: sensorData,
    timestamp: new Date().toISOString()
  });
});

// Verificar estado del broker principal
app.get('/api/broker-status', async (req, res) => {
  try {
    const response = await axios.get(`${BROKER_URL}/status`, { timeout: 5000 });
    res.json({
      success: true,
      status: 'online',
      data: response.data
    });
  } catch (error) {
    res.json({
      success: false,
      status: 'offline',
      message: error.message
    });
  }
});

// Endpoint para logs (SSE proxy) - Conectar al broker principal
app.get('/api/events', async (req, res) => {
  try {
    const response = await axios.get(`${BROKER_URL}/events`, {
      responseType: 'stream',
      timeout: 0
    });

    res.setHeader('Content-Type', 'text/event-stream');
    res.setHeader('Cache-Control', 'no-cache');
    res.setHeader('Connection', 'keep-alive');
    res.setHeader('Access-Control-Allow-Origin', '*');

    response.data.on('data', (chunk) => {
      const data = chunk.toString();
      
      // Procesar datos del ESP32
      if (data.includes('esp32/sensors')) {
        processSensorData(data);
      }
      
      // Reenviar datos al cliente
      res.write(`data: ${data}\n\n`);
    });

    req.on('close', () => {
      response.data.destroy();
    });

  } catch (error) {
    console.error('Error conectando a SSE:', error);
    res.status(500).end();
  }
});

// Procesar datos del sensor ESP32
function processSensorData(eventData) {
  try {
    const messageMatch = eventData.match(/"message": "([^"]+)"/);
    if (messageMatch && messageMatch[1]) {
      const message = messageMatch[1];
      const params = new URLSearchParams(message);
      
      sensorData = {
        fotoValue: params.get('foto') || '--',
        ledState: params.get('led') === '1' ? 'Encendido' : 'Apagado',
        wifiRssi: `${params.get('rssi') || '--'} dBm`,
        lastUpdate: new Date().toLocaleTimeString()
      };

      console.log('📊 Datos del sensor actualizados:', sensorData);
    }
  } catch (error) {
    console.error('Error procesando datos del sensor:', error);
  }
}

// Iniciar servidor
app.listen(PORT, () => {
  console.log(`🚀 Cliente web ejecutándose en http://localhost:${PORT}`);
  console.log(`🔗 Conectado al broker principal: ${BROKER_URL}`);
  console.log('✅ Listo para recibir datos del ESP32');
});
