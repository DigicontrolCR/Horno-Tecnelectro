class ESP32Control {
  constructor() {
    this.isConnected = false;
    this.sensorData = {
      fotoValue: '--',
      ledState: '--',
      wifiRssi: '--',
      lastUpdate: '--'
    };
    this.init();
  }

  async init() {
    this.updateDisplay();
    await this.checkBrokerStatus();
    this.connectToEventStream();
    this.startAutoRefresh();
    
    // Verificar estado del broker periódicamente
    setInterval(() => this.checkBrokerStatus(), 30000);
  }

  async checkBrokerStatus() {
    try {
      const response = await fetch('/api/broker-status');
      const data = await response.json();
      
      const statusElement = document.getElementById('brokerStatus');
      if (data.success) {
        statusElement.textContent = 'Broker: Conectado';
        this.updateConnectionStatus(true);
      } else {
        statusElement.textContent = 'Broker: Desconectado';
        this.updateConnectionStatus(false);
      }
    } catch (error) {
      document.getElementById('brokerStatus').textContent = 'Broker: Error';
      this.updateConnectionStatus(false);
    }
  }

  connectToEventStream() {
    try {
      this.eventSource = new EventSource('/api/events');
      
      this.eventSource.onmessage = (event) => {
        this.processEvent(event.data);
      };

      this.eventSource.onerror = (error) => {
        console.log('❌ Error en conexión SSE, reconectando...');
        this.updateConnectionStatus(false);
        setTimeout(() => this.connectToEventStream(), 5000);
      };

      this.updateConnectionStatus(true);
      console.log('✅ Conectado al stream de eventos');

    } catch (error) {
      console.error('Error connecting to SSE:', error);
      setTimeout(() => this.connectToEventStream(), 5000);
    }
  }

  processEvent(eventData) {
    console.log('🎯 Evento recibido:', eventData);
    
    // Buscar datos del ESP32 en los eventos
    if (eventData.includes('esp32/sensors')) {
      console.log('✅ Contiene datos de sensor');
      this.parseSensorData(eventData);
    }
  }

  parseSensorData(logMessage) {
    try {
      console.log('🔍 Analizando mensaje del sensor...');
      
      // Múltiples patrones para capturar el mensaje en diferentes formatos
      const patterns = [
        /"message":\s*"([^"]+)"/,           // Formato JSON: "message": "count=1&led=0..."
        /message:\s*([^\s]+)/,              // Formato log simple
        /Publicado en[^:]+:\s*([^\n]+)/,    // Formato de publicación
        /count=\d+&led=\d+&foto=\d+&rssi=-?\d+/ // Formato directo
      ];
      
      let sensorMessage = '';
      
      // Intentar con cada patrón hasta encontrar uno que funcione
      for (const pattern of patterns) {
        const match = logMessage.match(pattern);
        if (match) {
          sensorMessage = match[1] || match[0];
          console.log('✅ Patrón coincidente:', pattern.toString());
          break;
        }
      }
      
      if (sensorMessage) {
        console.log('📊 Mensaje extraído:', sensorMessage);
        
        // Limpiar el mensaje de comillas y espacios
        sensorMessage = sensorMessage.replace(/^"+|"+$/g, '').trim();
        
        // Parsear los parámetros
        const params = new URLSearchParams(sensorMessage);
        
        this.sensorData = {
          fotoValue: params.get('foto') || '--',
          ledState: params.get('led') === '1' ? 'Encendido' : 'Apagado',
          wifiRssi: `${params.get('rssi') || '--'} dBm`,
          lastUpdate: new Date().toLocaleTimeString()
        };

        console.log('✅ Datos del sensor actualizados:', this.sensorData);
        this.updateDisplay();
        this.updateLEDSwitch();
        
      } else {
        console.log('⚠️ No se pudo extraer mensaje del sensor de:', logMessage);
      }
    } catch (error) {
      console.error('❌ Error procesando datos del sensor:', error);
    }
  }

  async sendCommand(command) {
    try {
      console.log(`📤 Enviando comando: ${command}`);
      
      const response = await fetch('/api/send-message', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          topic: 'esp32/control',
          message: command
        })
      });

      const data = await response.json();
      
      if (data.success) {
        console.log('✅ Comando enviado exitosamente:', command);
        
        // Actualizar estado local inmediatamente
        if (command === 'led_on') {
          this.sensorData.ledState = 'Encendido';
        } else if (command === 'led_off') {
          this.sensorData.ledState = 'Apagado';
        }
        this.updateDisplay();
        this.updateLEDSwitch();
        
      } else {
        console.error('❌ Error enviando comando:', data.message);
      }

    } catch (error) {
      console.error('❌ Error de conexión:', error.message);
      this.updateConnectionStatus(false);
    }
  }

  updateLEDSwitch() {
    const ledSwitch = document.getElementById('ledSwitch');
    const ledStatus = document.getElementById('ledStatus');
    
    if (!ledSwitch || !ledStatus) return;
    
    if (this.sensorData.ledState === 'Encendido') {
      ledSwitch.checked = true;
      ledStatus.textContent = 'Encendido';
      ledStatus.style.color = '#27ae60';
      ledStatus.className = 'status-online';
    } else {
      ledSwitch.checked = false;
      ledStatus.textContent = 'Apagado';
      ledStatus.style.color = '#e74c3c';
      ledStatus.className = 'status-offline';
    }
  }

  updateDisplay() {
    // Actualizar datos de sensores
    const elements = {
      fotoValue: document.getElementById('fotoValue'),
      ledState: document.getElementById('ledState'),
      wifiRssi: document.getElementById('wifiRssi'),
      lastUpdate: document.getElementById('lastUpdate')
    };
    
    if (elements.fotoValue) elements.fotoValue.textContent = this.sensorData.fotoValue;
    if (elements.ledState) elements.ledState.textContent = this.sensorData.ledState;
    if (elements.wifiRssi) elements.wifiRssi.textContent = this.sensorData.wifiRssi;
    if (elements.lastUpdate) elements.lastUpdate.textContent = this.sensorData.lastUpdate;
  }

  updateConnectionStatus(connected) {
    const statusElement = document.getElementById('connectionStatus');
    if (!statusElement) return;
    
    this.isConnected = connected;
    
    if (connected) {
      statusElement.textContent = '🟢 Conectado';
      statusElement.className = 'status-online';
    } else {
      statusElement.textContent = '🔴 Desconectado';
      statusElement.className = 'status-offline';
    }
  }

  startAutoRefresh() {
    // Actualizar visualización cada 3 segundos para mantener consistencia
    setInterval(() => {
      this.updateDisplay();
    }, 3000);
  }
}

// Funciones globales para interactuar con los botones
function toggleLED() {
  if (window.esp32Control) {
    const ledSwitch = document.getElementById('ledSwitch');
    if (!ledSwitch) return;
    
    const command = ledSwitch.checked ? 'led_on' : 'led_off';
    window.esp32Control.sendCommand(command);
  }
}

function sendCommand(command) {
  if (window.esp32Control) {
    window.esp32Control.sendCommand(command);
    
    // Actualizar switch visualmente para feedback inmediato
    const ledSwitch = document.getElementById('ledSwitch');
    if (ledSwitch) {
      if (command === 'led_on') ledSwitch.checked = true;
      if (command === 'led_off') ledSwitch.checked = false;
      window.esp32Control.updateLEDSwitch();
    }
  }
}

// Inicializar la aplicación cuando el DOM esté listo
document.addEventListener('DOMContentLoaded', () => {
  console.log('🚀 Inicializando controlador ESP32...');
  window.esp32Control = new ESP32Control();
  
  // Agregar event listeners para mejor UX
  const ledSwitch = document.getElementById('ledSwitch');
  if (ledSwitch) {
    ledSwitch.addEventListener('change', toggleLED);
  }
  
  // Botones de control rápido
  const quickButtons = document.querySelectorAll('.btn-control');
  quickButtons.forEach(button => {
    button.addEventListener('click', (e) => {
      const command = e.target.dataset.command;
      if (command) {
        sendCommand(command);
      }
    });
  });
});

// Manejar recarga de página
window.addEventListener('beforeunload', () => {
  if (window.esp32Control && window.esp32Control.eventSource) {
    window.esp32Control.eventSource.close();
  }
});