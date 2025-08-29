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
    
    // Verificar estado periódicamente
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
        this.updateConnectionStatus(false);
        setTimeout(() => this.connectToEventStream(), 5000);
      };

      this.updateConnectionStatus(true);

    } catch (error) {
      console.error('Error connecting to SSE:', error);
    }
  }

  processEvent(eventData) {
    // Buscar datos del ESP32 en los eventos
    if (eventData.includes('esp32/sensors')) {
      this.parseSensorData(eventData);
    }
  }

  parseSensorData(logMessage) {
    try {
      const messageMatch = logMessage.match(/"message": "([^"]+)"/);
      if (messageMatch && messageMatch[1]) {
        const message = messageMatch[1];
        
        // Parsear datos en formato: count=1&led=0&foto=2045&rssi=-65
        const params = new URLSearchParams(message);
        
        this.sensorData = {
          fotoValue: params.get('foto') || '--',
          ledState: params.get('led') === '1' ? 'Encendido' : 'Apagado',
          wifiRssi: `${params.get('rssi') || '--'} dBm`,
          lastUpdate: new Date().toLocaleTimeString()
        };

        this.updateDisplay();
        this.updateLEDSwitch();
      }
    } catch (error) {
      console.error('Error parsing sensor data:', error);
    }
  }

  async sendCommand(command) {
    try {
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
        console.log('✅ Comando enviado:', command);
      } else {
        console.error('❌ Error enviando comando:', data.message);
      }

    } catch (error) {
      console.error('❌ Error de conexión:', error.message);
    }
  }

  updateLEDSwitch() {
    const ledSwitch = document.getElementById('ledSwitch');
    const ledStatus = document.getElementById('ledStatus');
    
    if (this.sensorData.ledState === 'Encendido') {
      ledSwitch.checked = true;
      ledStatus.textContent = 'Encendido';
      ledStatus.style.color = '#27ae60';
    } else {
      ledSwitch.checked = false;
      ledStatus.textContent = 'Apagado';
      ledStatus.style.color = '#e74c3c';
    }
  }

  updateDisplay() {
    // Actualizar datos de sensores
    document.getElementById('fotoValue').textContent = this.sensorData.fotoValue;
    document.getElementById('ledState').textContent = this.sensorData.ledState;
    document.getElementById('wifiRssi').textContent = this.sensorData.wifiRssi;
    document.getElementById('lastUpdate').textContent = this.sensorData.lastUpdate;
  }

  updateConnectionStatus(connected) {
    const statusElement = document.getElementById('connectionStatus');
    this.isConnected = connected;
    
    if (connected) {
      statusElement.textContent = '🟢 Conectado';
      statusElement.className = 'status-online';
    } else {
      statusElement.textContent = '🔴 Desconectado';
      statusElement.className = 'status-offline';
    }
  }
}

// Funciones globales
function toggleLED() {
  if (window.esp32Control) {
    const ledSwitch = document.getElementById('ledSwitch');
    const command = ledSwitch.checked ? 'led_on' : 'led_off';
    window.esp32Control.sendCommand(command);
  }
}

function sendCommand(command) {
  if (window.esp32Control) {
    window.esp32Control.sendCommand(command);
    
    // Actualizar switch visualmente
    const ledSwitch = document.getElementById('ledSwitch');
    if (command === 'led_on') ledSwitch.checked = true;
    if (command === 'led_off') ledSwitch.checked = false;
    window.esp32Control.updateLEDSwitch();
  }
}

// Inicializar
document.addEventListener('DOMContentLoaded', () => {
  window.esp32Control = new ESP32Control();
});
