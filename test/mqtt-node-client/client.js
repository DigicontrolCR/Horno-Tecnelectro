const axios = require('axios');

// Configuración
const BROKER_URL = 'https://horno-tecnelectro.onrender.com';
const API_ENDPOINT = `${BROKER_URL}/api/message`;
const TOPICS = {
  publish: 'node/out',
  subscribe: ['esp32/out', 'esp32/messages']
};

let messageCount = 0;

// Función para enviar mensajes via HTTP POST
async function sendMessage(message) {
  try {
    const payload = {
      topic: TOPICS.publish,
      message: message
    };

    const response = await axios.post(API_ENDPOINT, payload, {
      headers: {
        'Content-Type': 'application/json'
      },
      timeout: 10000
    });

    console.log('📤 Mensaje enviado:', message);
    console.log('✅ Respuesta del broker:', response.data);
    
    return response.data;
  } catch (error) {
    if (error.response) {
      console.error('❌ Error del servidor:', error.response.status, error.response.data);
    } else if (error.request) {
      console.error('❌ No se recibió respuesta del servidor');
    } else {
      console.error('❌ Error:', error.message);
    }
    return null;
  }
}

// Función para verificar el estado del broker
async function checkBrokerStatus() {
  try {
    const response = await axios.get(`${BROKER_URL}/status`, { timeout: 5000 });
    console.log('📊 Estado del broker:', response.data);
    return true;
  } catch (error) {
    console.error('❌ Broker no disponible:', error.message);
    return false;
  }
}

// Función principal
async function main() {
  console.log('🚀 Cliente Node.js para Broker HTTP API');
  console.log('🔗 URL:', BROKER_URL);
  console.log('----------------------------------------');

  // Verificar estado del broker
  const isBrokerOnline = await checkBrokerStatus();
  if (!isBrokerOnline) {
    console.log('⏳ Intentando reconexión en 5 segundos...');
    setTimeout(main, 5000);
    return;
  }

  // Enviar mensaje inicial
  await sendMessage('Node.js client iniciado');

  // Enviar mensajes periódicamente
  setInterval(async () => {
    messageCount++;
    const timestamp = new Date().toLocaleTimeString();
    const message = `Mensaje #${messageCount} desde Node.js - ${timestamp}`;
    
    await sendMessage(message);
  }, 5000);
}

// Manejar cierre graceful
process.on('SIGINT', () => {
  console.log('\n🛑 Cerrando cliente...');
  process.exit(0);
});

// Iniciar la aplicación
main().catch(console.error);

