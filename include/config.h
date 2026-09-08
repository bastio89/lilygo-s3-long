// Zentrale Konfiguration. Persoenliche Zugangsdaten stehen in secrets.h
// (siehe secrets.example.h).
#pragma once

#if __has_include("secrets.h")
#include "secrets.h"
#endif

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

// --- Ort / Zeit -------------------------------------------------------------
// Standardwerte: Menziken, Schweiz. Fuer den eigenen Ort z.B. auf
// https://open-meteo.com/en/docs nachschlagen.
#ifndef LOCATION_NAME
#define LOCATION_NAME "Menziken, Schweiz"
#endif
#ifndef LOCATION_LATITUDE
#define LOCATION_LATITUDE 47.23965
#endif
#ifndef LOCATION_LONGITUDE
#define LOCATION_LONGITUDE 8.18996
#endif

// POSIX-Zeitzone (Mitteleuropa inkl. Sommerzeitregel).
#ifndef TIMEZONE_POSIX
#define TIMEZONE_POSIX "CET-1CEST,M3.5.0,M10.5.0/3"
#endif
#ifndef NTP_SERVER
#define NTP_SERVER "de.pool.ntp.org"
#endif

// --- Pendelroute ------------------------------------------------------------
// Die privaten Koordinaten und der Mapbox-Public-Token koennen in
// include/secrets.h definiert werden. Bei 0.0 oder leerem Token ist die Route
// deaktiviert, bis die Einrichtung abgeschlossen ist.
#ifndef MAPBOX_ACCESS_TOKEN
#define MAPBOX_ACCESS_TOKEN ""
#endif
#ifndef COMMUTE_ORIGIN_NAME
#define COMMUTE_ORIGIN_NAME "Start"
#endif
#ifndef COMMUTE_ORIGIN_LATITUDE
#define COMMUTE_ORIGIN_LATITUDE 0.0f
#endif
#ifndef COMMUTE_ORIGIN_LONGITUDE
#define COMMUTE_ORIGIN_LONGITUDE 0.0f
#endif
#ifndef COMMUTE_DESTINATION_NAME
#define COMMUTE_DESTINATION_NAME "Buero"
#endif
#ifndef COMMUTE_DESTINATION_LATITUDE
#define COMMUTE_DESTINATION_LATITUDE 0.0f
#endif
#ifndef COMMUTE_DESTINATION_LONGITUDE
#define COMMUTE_DESTINATION_LONGITUDE 0.0f
#endif
#ifndef COMMUTE_REFRESH_MS
#define COMMUTE_REFRESH_MS (5UL * 60UL * 1000UL)
#endif

// --- Bambu Lab P1S ----------------------------------------------------------
// Der Drucker bleibt im normalen Cloud-Modus. Diese Werte aktivieren nur die
// lokale, lesende Statusabfrage per MQTT.
#ifndef BAMBU_PRINTER_IP
#define BAMBU_PRINTER_IP ""
#endif
#ifndef BAMBU_PRINTER_SERIAL
#define BAMBU_PRINTER_SERIAL ""
#endif
#ifndef BAMBU_ACCESS_CODE
#define BAMBU_ACCESS_CODE ""
#endif

// --- Schreibtisch -----------------------------------------------------------
// Fahrbereich der Tischplatte in cm. Vor dem ersten geregelten Fahren mit den
// Werten der eigenen Anlage abgleichen; zur Laufzeit auf der
// Einstellungsseite aenderbar.
#ifndef DESK_MIN_HEIGHT_CM
#define DESK_MIN_HEIGHT_CM 60.0f
#endif
#ifndef DESK_MAX_HEIGHT_CM
#define DESK_MAX_HEIGHT_CM 125.0f
#endif

// Werksvorgabe fuer die Speicherplaetze 1 und 2: geregelt angefahrene
// Zielhoehen. Die Plaetze 3 und 4 loesen ab Werk die Speicherplaetze 3 und 4
// der Steuerbox aus. Alles davon ist auf der Einstellungsseite aenderbar und
// wird dann im NVS abgelegt -- diese Werte gelten nur beim ersten Start bzw.
// nach "Werkseinstellungen".
#ifndef DESK_SIT_HEIGHT_CM
#define DESK_SIT_HEIGHT_CM 73.0f
#endif
#ifndef DESK_STAND_HEIGHT_CM
#define DESK_STAND_HEIGHT_CM 112.0f
#endif

// --- Anzeige ----------------------------------------------------------------
#ifndef DISPLAY_BRIGHTNESS
#define DISPLAY_BRIGHTNESS 180
#endif
// Nach dieser Zeit ohne Beruehrung geht das Display schlafen (0 = nie).
#ifndef DISPLAY_SLEEP_AFTER_MS
#define DISPLAY_SLEEP_AFTER_MS 120000UL
#endif

// --- Wetter -----------------------------------------------------------------
#ifndef WEATHER_REFRESH_MS
#define WEATHER_REFRESH_MS (15UL * 60UL * 1000UL)
#endif
