#include <Wire.h>

// ===== CONFIGURE YOUR I2C PINS =====
#define SDA_PIN 6
#define SCL_PIN 7
// ===================================

#define EEPROM_BASE 0x50
uint8_t eepromAddr = EEPROM_BASE;

// ---------- Utilities ----------
void eepromWaitReady(uint32_t timeout = 25) {
    uint32_t t0 = millis();
    while (millis() - t0 < timeout) {
        Wire.beginTransmission(eepromAddr);
        if (Wire.endTransmission() == 0) return;
        delay(1);
    }
}

bool eepromWritePaged(uint16_t mem, const uint8_t *data, size_t len, size_t page = 32) {
    size_t off = 0;
    while (off < len) {
        size_t pageSpace = page - ((mem + off) % page);
        size_t chunk = min(len - off, pageSpace);
        Wire.beginTransmission(eepromAddr);
        Wire.write((mem + off) >> 8);
        Wire.write((mem + off) & 0xFF);
        Wire.write(data + off, chunk);
        if (Wire.endTransmission() != 0) return false;
        eepromWaitReady();
        off += chunk;
    }
    return true;
}

bool eepromReadSeq(uint16_t mem, uint8_t *buf, size_t len) {
    Wire.beginTransmission(eepromAddr);
    Wire.write(mem >> 8);
    Wire.write(mem & 0xFF);
    if (Wire.endTransmission(false) != 0) return false;
    size_t got = 0;
    while (got < len) {
        size_t req = min((size_t)32, len - got);
        if (Wire.requestFrom((int)eepromAddr, (int)req) != (int)req) return false;
        for (size_t i = 0; i < req; i++) buf[got++] = Wire.read();
    }
    return true;
}

void dumpHex(const uint8_t* buf, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (i && i % 16 == 0) Serial.println();
        Serial.printf("%02X ", buf[i]);
    }
    Serial.println();
}

// ---------- Commands ----------
void cmdScan() {
    Serial.println("\n[I2C] Scanning...");
    bool any = false;
    bool picked = false;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            any = true;
            Serial.printf("  - Device at 0x%02X", addr);
            if (addr >= 0x50 && addr <= 0x57) {
                Serial.print("  (EEPROM candidate)");
                if (!picked) { eepromAddr = addr; picked = true; }
            }
            Serial.println();
        }
        delay(2);
    }
    if (!any) Serial.println("  (Nothing found)");
    if (picked) Serial.printf("Using EEPROM at 0x%02X\n", eepromAddr);
    else Serial.println("No EEPROM detected (0x50–0x57).");
}

// Checks write protection by trying to change and restore a byte
void cmdWPCheck(uint16_t memAddr) {
    uint8_t orig = 0xFF, test;
    if (!eepromReadSeq(memAddr, &orig, 1)) { Serial.println("Initial read failed."); return; }

    test = (orig == 0xFF) ? 0x00 : 0xFF;         // safe different value
    if (!eepromWritePaged(memAddr, &test, 1)) {  // some chips with WP high NACK the write
        Serial.println("Write NACK → possible WP ACTIVE or bus error.");
        return;
    }
    uint8_t after = 0xFF;
    if (!eepromReadSeq(memAddr, &after, 1)) { Serial.println("Post-write read failed."); return; }

    if (after != test) {
        Serial.printf("WP ACTIVE: Byte did not change (0x%02X -> 0x%02X read 0x%02X)\n", orig, test, after);
        return;
    }

    // If it did change, WP is disabled; restore the original value.
    if (!eepromWritePaged(memAddr, &orig, 1)) {
        Serial.println("Warning! Could not restore original value.");
    } else {
        eepromReadSeq(memAddr, &after, 1);
    }
    Serial.printf("WP DISABLED: was able to write and restore (final=0x%02X)\n", after);
}

void cmdRead(uint16_t addr, int len) {
    if (len <= 0 || len > 256) { Serial.println("len max 256"); return; }
    uint8_t buf[256];
    if (eepromReadSeq(addr, buf, len)) {
        Serial.println("HEX:"); dumpHex(buf, len);
        Serial.print("ASCII: ");
        for (int i = 0; i < len; i++) Serial.print((buf[i] >= 32 && buf[i] < 127) ? (char)buf[i] : '.');
        Serial.println();
    } else Serial.println("Read error");
}

void cmdWrite(uint16_t addr, const String& text) {
    if (eepromWritePaged(addr, (const uint8_t*)text.c_str(), text.length()))
        Serial.printf("Written at 0x%04X\n", addr);
    else
        Serial.println("Write error (WP high, address or bus).");
}

void cmdErase() {
    Serial.println("Erasing entire EEPROM (0xFF)...");
    const int total = 4096; // 32 Kbit = 4 KB
    uint8_t ff[32]; memset(ff, 0xFF, sizeof(ff));
    for (int a = 0; a < total; a += 32) {
        if (!eepromWritePaged(a, ff, 32)) { Serial.printf("Failed at 0x%04X\n", a); break; }
    }
    Serial.println("Erase complete.");
}

// Allows manually setting the found address
void cmdAddrSet(uint8_t addr) {
    eepromAddr = addr;
    Serial.printf("Selected EEPROM address: 0x%02X\n", eepromAddr);
}

// ---------- Parser ----------
String input;
void handleCommand(String line) {
    line.trim();
    if (line.length() == 0) return;

    if (line == "scan") { cmdScan(); return; }

    if (line.startsWith("addr ")) {
        String s = line.substring(5);
        uint8_t v = s.startsWith("0x") ? strtoul(s.c_str(), nullptr, 16) : (uint8_t)s.toInt();
        cmdAddrSet(v); return;
    }

    if (line.startsWith("read ")) {
        int sp1 = line.indexOf(' ');
        int sp2 = line.indexOf(' ', sp1 + 1);
        if (sp1 < 0 || sp2 < 0) { Serial.println("Usage: read <addr> <len>"); return; }
        String a = line.substring(sp1 + 1, sp2), b = line.substring(sp2 + 1);
        uint16_t addr = a.startsWith("0x") ? strtoul(a.c_str(), nullptr, 16) : (uint16_t)a.toInt();
        int len = b.toInt();
        cmdRead(addr, len); return;
    }

    if (line.startsWith("write ")) {
        int sp1 = line.indexOf(' ');
        int sp2 = line.indexOf(' ', sp1 + 1);
        if (sp1 < 0 || sp2 < 0) { Serial.println("Usage: write <addr> <text>"); return; }
        String a = line.substring(sp1 + 1, sp2), text = line.substring(sp2 + 1);
        uint16_t addr = a.startsWith("0x") ? strtoul(a.c_str(), nullptr, 16) : (uint16_t)a.toInt();
        cmdWrite(addr, text); return;
    }

    if (line == "erase") { cmdErase(); return; }

    if (line.startsWith("wpcheck")) {
        // wpcheck [addr]
        uint16_t m = 0x0000;
        int sp = line.indexOf(' ');
        if (sp > 0) {
            String a = line.substring(sp + 1);
            if (a.length()) m = a.startsWith("0x") ? strtoul(a.c_str(), nullptr, 16) : (uint16_t)a.toInt();
        }
        cmdWPCheck(m); return;
    }

    Serial.println("Commands:");
    Serial.println("  scan");
    Serial.println("  addr <0x50..0x57>");
    Serial.println("  read <addr> <len>     (addr accepts 0x..)");
    Serial.println("  write <addr> <text>");
    Serial.println("  erase");
    Serial.println("  wpcheck [addr]        (test WP; default 0x0000)");
}

void setup() {
    Serial.begin(115200);
    delay(200);
#if defined(ESP8266) || defined(ESP32)
    Wire.begin(SDA_PIN, SCL_PIN);
#else
    Wire.begin();
#endif
    Wire.setClock(100000);
    Serial.println("I2C-EEPROM monitor ready. Type 'scan' to detect the EEPROM.");
}

void loop() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') { if (input.length()) { handleCommand(input); input = ""; } }
        else input += c;
    }
}
