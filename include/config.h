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
// Standardwerte: Berlin. Fuer den eigenen Ort z.B. auf
// https://open-meteo.com/en/docs nachschlagen.
#ifndef LOCATION_NAME
#define LOCATION_NAME "Berlin"
#endif
#ifndef LOCATION_LATITUDE
#define LOCATION_LATITUDE 52.52
#endif
#ifndef LOCATION_LONGITUDE
#define LOCATION_LONGITUDE 13.405
#endif

// POSIX-Zeitzone (Europa/Berlin inkl. Sommerzeitregel).
#ifndef TIMEZONE_POSIX
#define TIMEZONE_POSIX "CET-1CEST,M3.5.0,M10.5.0/3"
#endif
#ifndef NTP_SERVER
#define NTP_SERVER "de.pool.ntp.org"
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
