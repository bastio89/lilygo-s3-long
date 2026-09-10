#include "ui/printer_screen.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "services/bambu.h"
#include "services/govee.h"
#include "ui/dashboard.h"
#include "ui/theme.h"

namespace ui {
namespace printer_screen {
namespace {

lv_obj_t *g_state = nullptr;
lv_obj_t *g_percent = nullptr;
lv_obj_t *g_remaining = nullptr;
lv_obj_t *g_job = nullptr;
lv_obj_t *g_layer = nullptr;
lv_obj_t *g_nozzle = nullptr;
lv_obj_t *g_bed = nullptr;
lv_obj_t *g_amsTemperature = nullptr;
lv_obj_t *g_amsHumidity = nullptr;
lv_obj_t *g_amsBattery = nullptr;
lv_obj_t *g_status = nullptr;
lv_obj_t *g_updated = nullptr;

void setLabelTextIfChanged(lv_obj_t *label, const char *text) {
    if (strcmp(lv_label_get_text(label), text) != 0) {
        lv_label_set_text(label, text);
    }
}

const char *stateText(const services::BambuData &data) {
    const char *state = data.state;
    if (strcmp(state, "IDLE") == 0) {
        return "Bereit";
    }
    if (strcmp(state, "PREPARE") == 0) {
        return "Vorbereiten";
    }
    if (strcmp(state, "RUNNING") == 0) {
        return "Druckt";
    }
    if (strcmp(state, "PAUSE") == 0) {
        return "Pausiert";
    }
    if (strcmp(state, "FINISH") == 0) {
        return "Fertig";
    }
    if (strcmp(state, "FAILED") == 0) {
        return "Fehler";
    }
    if (strcmp(state, "SLICING") == 0) {
        return "Slicen";
    }

    // Wenn der Zustand unbekannt oder leer ist, aber aktive Druckdaten vorliegen
    if (data.remainingMinutes > 0 || data.percent > 0 || data.currentLayer > 0) {
        return "Druckt";
    }

    return (state[0] == '\0' || strcmp(state, "UNKNOWN") == 0) ? "Bereit" : state;
}

void refreshCb(lv_event_t *) {
    services::bambu().refresh();
    services::govee().refresh();
    dashboard::toast("Drucker- und AMS-Status werden aktualisiert");
}

lv_obj_t *makeCard(lv_obj_t *parent, lv_coord_t x, lv_coord_t width) {
    lv_obj_t *card = lv_obj_create(parent);
    theme::styleCard(card);
    lv_obj_set_pos(card, x, 0);
    lv_obj_set_size(card, width, kPageHeight - 12);
    return card;
}

} // namespace

void create(lv_obj_t *parent) {
    lv_obj_set_style_pad_all(parent, 6, 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *summary = makeCard(parent, 0, 196);
    lv_obj_t *summaryTitle = theme::label(summary, &lv_font_montserrat_14, theme::muted(), "P1S");
    lv_obj_set_pos(summaryTitle, 10, 8);
    g_state = theme::label(summary, &lv_font_montserrat_20, theme::text(), "Nicht verbunden");
    lv_obj_set_pos(g_state, 10, 28);
    lv_obj_set_size(g_state, 176, 26);
    lv_label_set_long_mode(g_state, LV_LABEL_LONG_CLIP);

    g_percent = theme::label(summary, &lv_font_montserrat_48, theme::text(), "-- %");
    lv_obj_set_pos(g_percent, 10, 52);
    lv_obj_set_size(g_percent, 176, 58);
    lv_label_set_long_mode(g_percent, LV_LABEL_LONG_CLIP);
    g_remaining = theme::label(summary, &lv_font_montserrat_16, theme::muted(), "Restzeit --");
    lv_obj_set_pos(g_remaining, 10, 112);
    lv_obj_set_size(g_remaining, 176, 20);
    lv_label_set_long_mode(g_remaining, LV_LABEL_LONG_CLIP);

    lv_obj_t *jobCard = makeCard(parent, 204, 244);
    lv_obj_t *jobTitle = theme::label(jobCard, &lv_font_montserrat_14, theme::muted(), "Auftrag");
    lv_obj_set_pos(jobTitle, 10, 8);
    g_job = theme::label(jobCard, &lv_font_montserrat_20, theme::text(), "Kein Auftrag");
    lv_obj_set_pos(g_job, 10, 30);
    lv_obj_set_size(g_job, 224, 30);
    lv_label_set_long_mode(g_job, LV_LABEL_LONG_DOT);
    g_layer = theme::label(jobCard, &lv_font_montserrat_16, theme::muted(), "Schicht -- / --");
    lv_obj_set_pos(g_layer, 10, 70);
    lv_obj_set_size(g_layer, 224, 22);
    lv_label_set_long_mode(g_layer, LV_LABEL_LONG_CLIP);
    g_updated = theme::label(jobCard, &lv_font_montserrat_14, theme::muted(), "Warte auf Daten");
    lv_obj_set_pos(g_updated, 10, 108);
    lv_obj_set_size(g_updated, 224, 20);
    lv_label_set_long_mode(g_updated, LV_LABEL_LONG_CLIP);

    lv_obj_t *details = makeCard(parent, 456, kScreenWidth - 462);
    lv_obj_t *detailsTitle =
        theme::label(details, &lv_font_montserrat_14, theme::muted(), "P1S / AMS");
    lv_obj_set_pos(detailsTitle, 10, 8);
    lv_obj_set_size(detailsTitle, 112, 18);
    lv_label_set_long_mode(detailsTitle, LV_LABEL_LONG_CLIP);
    g_nozzle = theme::label(details, &lv_font_montserrat_14, theme::text(), "Düse -- C");
    lv_obj_set_pos(g_nozzle, 10, 26);
    lv_obj_set_size(g_nozzle, 126, 18);
    lv_label_set_long_mode(g_nozzle, LV_LABEL_LONG_CLIP);
    g_bed = theme::label(details, &lv_font_montserrat_14, theme::text(), "Bett -- C");
    lv_obj_set_pos(g_bed, 10, 45);
    lv_obj_set_size(g_bed, 126, 18);
    lv_label_set_long_mode(g_bed, LV_LABEL_LONG_CLIP);
    g_amsTemperature = theme::label(details, &lv_font_montserrat_14, theme::muted(), "AMS -- C");
    lv_obj_set_pos(g_amsTemperature, 10, 64);
    lv_obj_set_size(g_amsTemperature, 140, 18);
    lv_label_set_long_mode(g_amsTemperature, LV_LABEL_LONG_CLIP);
    g_amsHumidity = theme::label(details, &lv_font_montserrat_14, theme::muted(), "Feuchte -- %");
    lv_obj_set_pos(g_amsHumidity, 10, 83);
    lv_obj_set_size(g_amsHumidity, 140, 18);
    lv_label_set_long_mode(g_amsHumidity, LV_LABEL_LONG_CLIP);
    g_amsBattery = theme::label(details, &lv_font_montserrat_12, theme::muted(), "Akku -- %");
    lv_obj_set_pos(g_amsBattery, 10, 104);
    lv_obj_set_size(g_amsBattery, 76, 16);
    lv_label_set_long_mode(g_amsBattery, LV_LABEL_LONG_CLIP);
    g_status = theme::label(details, &lv_font_montserrat_12, theme::muted(), "P1S --");
    lv_obj_set_pos(g_status, 90, 104);
    lv_obj_set_size(g_status, 78, 16);
    lv_label_set_long_mode(g_status, LV_LABEL_LONG_CLIP);

    lv_obj_t *refresh = theme::button(details, LV_SYMBOL_REFRESH, &lv_font_montserrat_20, 136, 4, 34, 34);
    lv_obj_add_event_cb(refresh, refreshCb, LV_EVENT_CLICKED, nullptr);
}

void tick(uint32_t nowMs) {
    const services::BambuData data = services::bambu().snapshot();
    const char *status = services::bambu().statusText();
    setLabelTextIfChanged(g_status, status);
    lv_obj_set_style_text_color(g_status, strcmp(status, "verbunden") == 0
                                           ? theme::good()
                                           : theme::muted(),
                                0);

    const services::GoveeData govee = services::govee().snapshot();
    const bool goveeReady = services::govee().ready();
    const lv_color_t goveeColor = goveeReady ? theme::good() : theme::muted();
    lv_obj_set_style_text_color(g_amsTemperature, goveeColor, 0);
    lv_obj_set_style_text_color(g_amsHumidity, goveeColor, 0);
    lv_obj_set_style_text_color(g_amsBattery, goveeColor, 0);
    if (!govee.valid) {
        setLabelTextIfChanged(g_amsTemperature, "AMS -- C");
        setLabelTextIfChanged(g_amsHumidity, "Feuchte -- %");
        setLabelTextIfChanged(g_amsBattery, "Akku -- %");
    } else {
        char amsTemperature[24];
        snprintf(amsTemperature, sizeof(amsTemperature), "AMS %.1f C",
                 static_cast<double>(govee.temperatureC));
        setLabelTextIfChanged(g_amsTemperature, amsTemperature);

        char amsHumidity[28];
        snprintf(amsHumidity, sizeof(amsHumidity), "Feuchte %.1f %%",
                 static_cast<double>(govee.humidityPct));
        setLabelTextIfChanged(g_amsHumidity, amsHumidity);

        char amsBattery[20];
        snprintf(amsBattery, sizeof(amsBattery), "Akku %u %%",
                 static_cast<unsigned>(govee.batteryPct));
        setLabelTextIfChanged(g_amsBattery, amsBattery);
    }

    if (!data.valid) {
        setLabelTextIfChanged(g_state, "Nicht verbunden");
        setLabelTextIfChanged(g_percent, "-- %");
        setLabelTextIfChanged(g_remaining, "Restzeit --");
        setLabelTextIfChanged(g_job, "Kein Auftrag");
        setLabelTextIfChanged(g_layer, "Schicht -- / --");
        setLabelTextIfChanged(g_nozzle, "Düse -- C");
        setLabelTextIfChanged(g_bed, "Bett -- C");
        setLabelTextIfChanged(g_updated, "Warte auf Daten");
        return;
    }

    char state[32];
    snprintf(state, sizeof(state), "%s", stateText(data));
    setLabelTextIfChanged(g_state, state);

    char percent[16];
    snprintf(percent, sizeof(percent), "%u %%", static_cast<unsigned>(data.percent));
    setLabelTextIfChanged(g_percent, percent);

    char remaining[32];
    if (data.remainingMinutes >= 60) {
        snprintf(remaining, sizeof(remaining), "Restzeit %lu h %lu min",
                 static_cast<unsigned long>(data.remainingMinutes / 60),
                 static_cast<unsigned long>(data.remainingMinutes % 60));
    } else {
        snprintf(remaining, sizeof(remaining), "Restzeit %lu min",
                 static_cast<unsigned long>(data.remainingMinutes));
    }
    setLabelTextIfChanged(g_remaining, remaining);

    setLabelTextIfChanged(g_job, data.fileName[0] == '\0' ? "Kein Auftrag" : data.fileName);

    char layer[32];
    snprintf(layer, sizeof(layer), "Schicht %lu / %lu",
             static_cast<unsigned long>(data.currentLayer),
             static_cast<unsigned long>(data.totalLayers));
    setLabelTextIfChanged(g_layer, layer);

    char nozzle[24];
    snprintf(nozzle, sizeof(nozzle), "Düse %.0f C", static_cast<double>(data.nozzleTemperature));
    setLabelTextIfChanged(g_nozzle, nozzle);
    char bed[24];
    snprintf(bed, sizeof(bed), "Bett %.0f C", static_cast<double>(data.bedTemperature));
    setLabelTextIfChanged(g_bed, bed);

    char updated[32];
    const uint32_t ageSeconds = static_cast<uint32_t>(nowMs - data.updatedMs) / 1000UL;
    if (data.errorCode != 0) {
        snprintf(updated, sizeof(updated), "Fehler %lu",
                 static_cast<unsigned long>(data.errorCode));
    } else {
        snprintf(updated, sizeof(updated), "vor %lu s", static_cast<unsigned long>(ageSeconds));
    }
    setLabelTextIfChanged(g_updated, updated);
}

} // namespace printer_screen
} // namespace ui