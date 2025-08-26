class MQTTMonitor {
  constructor() {
    this.logDiv = document.getElementById('logs');
    this.statusElement = document.getElementById('status');
    this.lastUpdateElement = document.getElementById('lastUpdate');
    this.messageCountElement = document.getElementById('messageCount');
    this.messageCount = 0;
    this.lastMessageTime = null;
    
    this.init();
  }

  init() {
    this.setupEventSource();
    this.setupClearButton();
    this.updateStats();
  }

  setupEventSource() {
    try {
      this.evtSource = new EventSource('/events');
      
      this.evtSource.onmessage = (event) => {
        this.addLog(event.data);
        this.messageCount++;
        this.lastMessageTime = new Date();
        this.updateStats();
      };

      this.evtSource.onerror = (error) => {
        this.addLog('❌ Error de conexión con el servidor', 'error');
        this.statusElement.textContent = 'Desconectado';
        this.statusElement.style.color = '#e74c3c';
      };

      this.addLog('✅ Conectado al servidor de logs', 'connected');
      this.statusElement.textContent = 'Conectado';
      this.statusElement.style.color = '#4ecca3';

    } catch (error) {
      this.addLog('❌ No se pudo conectar al servidor: ' + error.message, 'error');
    }
  }

  setupClearButton() {
    const clearBtn = document.getElementById('clearBtn');
    clearBtn.addEventListener('click', () => {
      this.clearLogs();
    });
  }

  addLog(message, type = 'message') {
    const timestamp = new Date().toLocaleTimeString();
    const logEntry = document.createElement('div');
    logEntry.className = `log-message ${type}`;
    logEntry.innerHTML = `
      <span class="timestamp" style="color: #888; margin-right: 10px;">[${timestamp}]</span>
      <span class="message">${this.formatMessage(message)}</span>
    `;
    
    this.logDiv.appendChild(logEntry);
    this.logDiv.scrollTop = this.logDiv.scrollHeight;
  }

  formatMessage(message) {
    // Añadir emojis y formato según el tipo de mensaje
    if (message.includes('conectado')) return `✅ ${message}`;
    if (message.includes('desconectado')) return `❌ ${message}`;
    if (message.includes('Mensaje')) return `📩 ${message}`;
    if (message.includes('Error')) return `⚠️ ${message}`;
    if (message.includes('Publicado')) return `📤 ${message}`;
    return message;
  }

  clearLogs() {
    this.logDiv.innerHTML = '';
    this.messageCount = 0;
    this.updateStats();
    this.addLog('🧹 Logs limpiados', 'message');
  }

  updateStats() {
    this.messageCountElement.textContent = this.messageCount;
    
    if (this.lastMessageTime) {
      this.lastUpdateElement.textContent = this.lastMessageTime.toLocaleTimeString();
    }
  }
}

// Inicializar la aplicación cuando el DOM esté listo
document.addEventListener('DOMContentLoaded', () => {
  new MQTTMonitor();
});

// Manejar recarga de página con confirmación
window.addEventListener('beforeunload', (e) => {
  e.preventDefault();
  e.returnValue = '';
});

