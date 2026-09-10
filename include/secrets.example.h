// Kopiere diese Datei nach include/secrets.h und trage deine Werte ein.
// include/secrets.h ist per .gitignore ausgeschlossen.
#pragma once

#define WIFI_SSID "MeinWLAN"
#define WIFI_PASSWORD "geheim"

// Kostenlosen Public-Token unter https://account.mapbox.com/access-tokens/
// anlegen. Den echten Token nur in include/secrets.h eintragen.
// #define MAPBOX_ACCESS_TOKEN "pk. ..."

// Optional: read-only status from a P1S in the same local network.
// The printer can remain in normal cloud mode; this does not send print
// commands. Find the IP address and serial number in the printer network
// settings. The LAN access code is the printer's local MQTT password.
// #define BAMBU_PRINTER_IP "192.168.1.123"
// #define BAMBU_PRINTER_SERIAL "01P00A000000000"
// #define BAMBU_ACCESS_CODE "12345678"

// Optional: Govee H5074 im AMS. Die MAC-Adresse wird beim ersten Fund auch
// auf der seriellen Konsole ausgegeben. Leer lassen, um den ersten H5074 zu
// verwenden.
// #define GOVEE_H5074_ADDRESS "AA:BB:CC:DD:EE:FF"
