#include <Wire.h>

// ===== CONFIGURA TUS PINES I2C =====
#define SDA_PIN 6
#define SCL_PIN 7
// ===================================

#define EEPROM_BASE 0x50
uint8_t eepromAddr = EEPROM_BASE;

// ---------- Funciones básicas ----------
void eepromWaitReady(uint32_t timeout = 25) {
  uint32_t t0 = millis();
  while (millis() - t0 < timeout) {
    Wire.beginTransmission(eepromAddr);
    if (Wire.endTransmission() == 0) return;
    delay(1);
  }
}

bool eepromWrite(uint16_t addr, const uint8_t *data, size_t len) {
  Wire.beginTransmission(eepromAddr);
  Wire.write(addr >> 8);
  Wire.write(addr & 0xFF);
  Wire.write(data, len);
  if (Wire.endTransmission() != 0) return false;
  eepromWaitReady();
  return true;
}

bool eepromRead(uint16_t addr, uint8_t *buf, size_t len) {
  Wire.beginTransmission(eepromAddr);
  Wire.write(addr >> 8);
  Wire.write(addr & 0xFF);
  if (Wire.endTransmission(false) != 0) return false;
  
  if (Wire.requestFrom((int)eepromAddr, (int)len) != (int)len) return false;
  for (size_t i = 0; i < len; i++) {
    buf[i] = Wire.read();
  }
  return true;
}

void i2cScan() {
  Serial.println("\n=== Escaneando bus I2C ===");
  bool found = false;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Dispositivo encontrado: 0x%02X", addr);
      if (addr >= 0x50 && addr <= 0x57) {
        Serial.print(" <- EEPROM");
        eepromAddr = addr;
        found = true;
      }
      Serial.println();
    }
    delay(2);
  }
  if (!found) Serial.println("¡EEPROM no detectada!");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(200);
  
  Serial.println("\n*** DEMO EEPROM FT24C32A ***\n");
  
  // Inicializar I2C
#if defined(ESP8266) || defined(ESP32)
  Wire.begin(SDA_PIN, SCL_PIN);
#else
  Wire.begin();
#endif
  Wire.setClock(100000);
  
  // 1. SCAN del bus I2C
  i2cScan();
  
  // 2. ESCRIBIR datos
  Serial.println("=== Escribiendo en EEPROM ===");
  const char* testData = "Hola EEPROM!";
  uint16_t writeAddr = 0x0010;
  
  if (eepromWrite(writeAddr, (const uint8_t*)testData, strlen(testData))) {
    Serial.printf("✓ Escrito en 0x%04X: \"%s\"\n\n", writeAddr, testData);
  } else {
    Serial.println("✗ Error al escribir\n");
  }
  
  delay(10);
  
  // 3. LEER datos
  Serial.println("=== Leyendo de EEPROM ===");
  uint8_t readBuf[32] = {0};
  
  if (eepromRead(writeAddr, readBuf, strlen(testData))) {
    Serial.printf("✓ Leído de 0x%04X: \"", writeAddr);
    for (size_t i = 0; i < strlen(testData); i++) {
      Serial.print((char)readBuf[i]);
    }
    Serial.println("\"");
    
    // Mostrar en HEX
    Serial.print("  HEX: ");
    for (size_t i = 0; i < strlen(testData); i++) {
      Serial.printf("%02X ", readBuf[i]);
    }
    Serial.println("\n");
  } else {
    Serial.println("✗ Error al leer\n");
  }
  
  // Verificación
  if (memcmp(testData, readBuf, strlen(testData)) == 0) {
    Serial.println("✓ DEMO EXITOSO: Datos escritos y leídos correctamente!");
  } else {
    Serial.println("✗ ERROR: Los datos no coinciden");
  }
}

void loop() {
  // Demo completo, nada más que hacer
}
