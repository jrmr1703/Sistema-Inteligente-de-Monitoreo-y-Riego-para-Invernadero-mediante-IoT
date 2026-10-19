# 🌿 Sistema Inteligente de Monitoreo y Riego para Invernadero mediante IoT

Sistema de control distribuido y monitoreo ambiental para cultivo automatizado, desarrollado con arquitectura dual **ESP32**, protocolo inalámbrico **ESP-NOW**, interfaz gráfica táctil **Nextion HMI** y plataformas IoT (**Ubidots** y **Node-RED**).

---

## 🛠️ Especificaciones Técnicas y Hardware Utilizado

| Componente | Cantidad | Función / Protocolo |
| :--- | :---: | :--- |
| **ESP32 DevKit V1** | 2 | Unidad de Control y Unidad de Supervisión [ESP-NOW] |
| **Pantalla Nextion HMI** | 1 | Interfaz táctil local de supervisión y configuración [UART] |
| **Sensor AHT20** | 1 | Medición de temperatura y humedad ambiental [I2C] |
| **Sensor YL69** | 1 | Medición de humedad del suelo [Entrada Analógica ADC] |
| **Módulo Relevadores (2 CH)** | 1 | Control para bomba de agua e iluminación |
| **Bomba de Agua 5V** | 1 | Riego automático del cultivo |
| **Iluminación LED (Azul/Rojo)** | 1 | Iluminación de crecimiento vegetal |
| **Regulador LM2596** | 1 | Fuente de alimentación de voltaje |
| **Gabinete Impreso 3D** | 1 | Chasis personalizado diseñado en **SolidWorks** |

---

## 📐 Arquitectura del Sistema

El sistema utiliza dos microcontroladores para separar las tareas de control e interfaz:

1. **Unidad de Control (`ESP32_1.ino`):**
   - Adquiere datos del sensor AHT20 (**I2C**) y humedad de suelo YL69 (**ADC**).
   - Controla el encendido y apagado de la bomba de agua y luz LED.
   - Envía telemetría inalámbricamente a la segunda tarjeta vía **ESP-NOW**.

2. **Unidad de Supervisión e IoT (`ESP32_2.ino`):**
   - Recibe datos mediante **ESP-NOW**.
   - Muestra la información y recibe comandos desde la pantalla **Nextion (UART)**.
   - Publica datos a la nube en **Ubidots** y dashboard en **Node-RED** vía **WiFi / MQTT**.
  
   - https://drive.google.com/file/d/1de_QhfY7tO_Ukbk7hHV6DqTn-4Afu19-/view
     
---

## 👨‍💻 Autor

**José Rodolfo Molina Rodríguez**  
*Ingeniero Electrónico*  
- **LinkedIn:** [jose-rodolfo-molina-rodriguez](https://linkedin.com/in/josé-rodolfo-molina-rodríguez-b48014445)  
- **Correo:** jrmr1703@gmail.com
