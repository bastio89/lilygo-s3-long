#include "ui/weather_screen.h"

#include <stdio.h>
#include <string.h>

#include "core/clock.h"
#include "core/settings.h"
#include "services/weather.h"
#include "ui/dashboard.h"
#include "ui/theme.h"

namespace ui {
namespace weather_screen {
namespace {

constexpr lv_coord_t kCurrentCardHeight = 138;
constexpr lv_coord_t kForecastCardHeight = 106;
constexpr lv_coord_t kForecastGap = 8;
constexpr lv_coord_t kForecastCardWidth =
    (kScreenWidth - 12 - kForecastGap * (services::kWeatherForecastDays - 1)) /
    services::kWeatherForecastDays;

enum class WeatherIconKind : uint8_t {
    Clear,
    PartlyCloudy,
    Cloudy,
    Rain,
    Snow,
    Storm,
};

struct WeatherIcon {
    lv_obj_t *root = nullptr;
    lv_obj_t *sunGlow = nullptr;
    lv_obj_t *sun = nullptr;
    lv_obj_t *cloud[4] = {};
    lv_obj_t *rain[3] = {};
    lv_obj_t *snow[3] = {};
    lv_obj_t *bolt[2] = {};
    lv_coord_t width = 0;
    lv_coord_t unit = 0;
    int code = -1;
    bool visible = false;
};

struct ForecastUi {
    lv_obj_t *day = nullptr;
    lv_obj_t *range = nullptr;
    WeatherIcon icon;
};

lv_obj_t *g_temp = nullptr;
lv_obj_t *g_desc = nullptr;
lv_obj_t *g_feels = nullptr;
lv_obj_t *g_range = nullptr;
lv_obj_t *g_humidity = nullptr;
lv_obj_t *g_wind = nullptr;
lv_obj_t *g_precipitation = nullptr;
lv_obj_t *g_place = nullptr;
lv_obj_t *g_status = nullptr;
WeatherIcon g_currentIcon;
ForecastUi g_forecast[services::kWeatherForecastDays];

lv_obj_t *makeShape(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t width,
                    lv_coord_t height, uint32_t color, lv_coord_t radius) {
    lv_obj_t *shape = lv_obj_create(parent);
    lv_obj_set_pos(shape, x, y);
    lv_obj_set_size(shape, width, height);
    lv_obj_set_style_bg_color(shape, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(shape, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(shape, 0, 0);
    lv_obj_set_style_radius(shape, radius, 0);
    lv_obj_set_style_pad_all(shape, 0, 0);
    lv_obj_clear_flag(shape, LV_OBJ_FLAG_SCROLLABLE);
    return shape;
}

void setObjectHidden(lv_obj_t *obj, bool hidden) {
    if (hidden) {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}

WeatherIcon createWeatherIcon(lv_obj_t *parent, lv_coord_t width, lv_coord_t height) {
    WeatherIcon icon;
    icon.root = theme::plainBox(parent);
    icon.width = width;
    icon.unit = height / 13;
    if (icon.unit < 3) {
        icon.unit = 3;
    }
    lv_obj_set_size(icon.root, width, height);

    const lv_coord_t unit = icon.unit;
    icon.sunGlow =
        makeShape(icon.root, 0, 0, 8 * unit, 8 * unit, 0xFFB84Du, LV_RADIUS_CIRCLE);
    lv_obj_set_style_bg_opa(icon.sunGlow, LV_OPA_40, 0);
    icon.sun = makeShape(icon.root, 0, 0, 6 * unit, 6 * unit, 0xFFC64Bu, LV_RADIUS_CIRCLE);

    icon.cloud[0] = makeShape(icon.root, 3 * unit, 7 * unit, 13 * unit, 4 * unit, 0x78899Au,
                              2 * unit);
    icon.cloud[1] = makeShape(icon.root, 4 * unit, 5 * unit, 6 * unit, 6 * unit, 0xA9B6C2u,
                              LV_RADIUS_CIRCLE);
    icon.cloud[2] = makeShape(icon.root, 8 * unit, 3 * unit, 7 * unit, 8 * unit, 0xA9B6C2u,
                              LV_RADIUS_CIRCLE);
    icon.cloud[3] = makeShape(icon.root, 13 * unit, 6 * unit, 5 * unit, 5 * unit, 0xA9B6C2u,
                              LV_RADIUS_CIRCLE);

    for (uint8_t index = 0; index < 3; ++index) {
        icon.rain[index] = makeShape(icon.root, (6 + index * 4) * unit, 11 * unit, unit,
                                     2 * unit, 0x4FA3FFu, unit / 2);
        icon.snow[index] = makeShape(icon.root, (6 + index * 4) * unit, 11 * unit,
                                     2 * unit, 2 * unit, 0xDDEBFAu, LV_RADIUS_CIRCLE);
    }
    icon.bolt[0] = makeShape(icon.root, 10 * unit, 9 * unit, 2 * unit, 3 * unit, 0xFFC64Bu, 0);
    icon.bolt[1] = makeShape(icon.root, 9 * unit, 11 * unit, 2 * unit, 2 * unit, 0xFFC64Bu, 0);
    return icon;
}

WeatherIconKind weatherIconKind(int code) {
    if (code <= 1) {
        return WeatherIconKind::Clear;
    }
    if (code == 2) {
        return WeatherIconKind::PartlyCloudy;
    }
    if (code == 3 || code == 45 || code == 48) {
        return WeatherIconKind::Cloudy;
    }
    if ((code >= 71 && code <= 77) || code == 85 || code == 86) {
        return WeatherIconKind::Snow;
    }
    if (code >= 95) {
        return WeatherIconKind::Storm;
    }
    return WeatherIconKind::Rain;
}

void positionSun(WeatherIcon &icon, lv_coord_t x, lv_coord_t y) {
    const lv_coord_t unit = icon.unit;
    lv_obj_set_pos(icon.sunGlow, x - unit, y - unit);
    lv_obj_set_pos(icon.sun, x, y);
}

void updateWeatherIcon(WeatherIcon &icon, int code, bool valid) {
    if (!valid) {
        if (icon.visible) {
            lv_obj_add_flag(icon.root, LV_OBJ_FLAG_HIDDEN);
            icon.visible = false;
        }
        return;
    }
    if (icon.visible && icon.code == code) {
        return;
    }

    icon.visible = true;
    icon.code = code;
    lv_obj_clear_flag(icon.root, LV_OBJ_FLAG_HIDDEN);

    const WeatherIconKind kind = weatherIconKind(code);
    const bool showSun = kind == WeatherIconKind::Clear || kind == WeatherIconKind::PartlyCloudy;
    const bool showCloud = kind != WeatherIconKind::Clear;
    const bool showRain = kind == WeatherIconKind::Rain || kind == WeatherIconKind::Storm;
    const bool showSnow = kind == WeatherIconKind::Snow;
    const bool showStorm = kind == WeatherIconKind::Storm;

    const lv_coord_t sunX = kind == WeatherIconKind::Clear
                                 ? (icon.width - 6 * icon.unit) / 2
                                 : icon.unit;
    positionSun(icon, sunX, kind == WeatherIconKind::Clear ? 2 * icon.unit : icon.unit);
    setObjectHidden(icon.sunGlow, !showSun);
    setObjectHidden(icon.sun, !showSun);
    for (lv_obj_t *cloud : icon.cloud) {
        setObjectHidden(cloud, !showCloud);
    }
    for (lv_obj_t *drop : icon.rain) {
        setObjectHidden(drop, !showRain);
    }
    for (lv_obj_t *flake : icon.snow) {
        setObjectHidden(flake, !showSnow);
    }
    for (lv_obj_t *bolt : icon.bolt) {
        setObjectHidden(bolt, !showStorm);
    }
}

void setLabelTextIfChanged(lv_obj_t *label, const char *text) {
    if (strcmp(lv_label_get_text(label), text) != 0) {
        lv_label_set_text(label, text);
    }
}

lv_obj_t *makeMetric(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t width) {
    lv_obj_t *metric = theme::label(parent, &lv_font_montserrat_16, theme::muted(), "");
    lv_obj_set_pos(metric, x, y);
    lv_obj_set_size(metric, width, 20);
    lv_label_set_long_mode(metric, LV_LABEL_LONG_CLIP);
    return metric;
}

int weekdayIndex(int year, int month, int day) {
    static const int monthOffsets[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (month < 3) {
        --year;
    }
    return (year + year / 4 - year / 100 + year / 400 + monthOffsets[month - 1] + day) % 7;
}

void formatForecastDay(const char *date, char *text, size_t textSize) {
    int year = 0;
    int month = 0;
    int day = 0;
    if (sscanf(date, "%d-%d-%d", &year, &month, &day) != 3 || month < 1 || month > 12 ||
        day < 1 || day > 31) {
        snprintf(text, textSize, "Folgetag");
        return;
    }
    static const char *const weekdays[] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
    snprintf(text, textSize, "%s, %02d.%02d.", weekdays[weekdayIndex(year, month, day)], day,
             month);
}

void refreshCb(lv_event_t *) {
    services::weather().refresh();
    dashboard::toast("Wetter wird aktualisiert");
}

} // namespace

void create(lv_obj_t *parent) {
    lv_obj_set_style_pad_all(parent, 6, 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *list = lv_obj_create(parent);
    lv_obj_set_pos(list, 0, 0);
    lv_obj_set_size(list, kScreenWidth - 12, kPageHeight - 12);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_style_pad_row(list, kForecastGap, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scroll_dir(list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_ELASTIC);

    lv_obj_t *current = lv_obj_create(list);
    theme::styleCard(current);
    lv_obj_set_size(current, kScreenWidth - 12, kCurrentCardHeight);

    g_place = theme::label(current, &lv_font_montserrat_14, theme::muted(), "");
    lv_obj_set_pos(g_place, 12, 10);
    g_status = theme::label(current, &lv_font_montserrat_14, theme::muted(), "");
    lv_obj_set_pos(g_status, 470, 14);
    lv_obj_set_size(g_status, 104, 18);
    lv_obj_set_style_text_align(g_status, LV_TEXT_ALIGN_RIGHT, 0);

    lv_obj_t *refresh = theme::button(current, LV_SYMBOL_REFRESH, &lv_font_montserrat_20, 584, 6, 34, 34);
    lv_obj_add_event_cb(refresh, refreshCb, LV_EVENT_CLICKED, nullptr);

    g_currentIcon = createWeatherIcon(current, 166, 104);
    lv_obj_set_pos(g_currentIcon.root, 12, 27);
    updateWeatherIcon(g_currentIcon, 0, false);

    g_temp = theme::label(current, &lv_font_montserrat_48, theme::text(), "--");
    lv_obj_set_pos(g_temp, 194, 27);
    g_desc = theme::label(current, &lv_font_montserrat_24, theme::text(), "Keine Daten");
    lv_obj_set_pos(g_desc, 350, 38);
    lv_obj_set_size(g_desc, 226, 30);
    lv_label_set_long_mode(g_desc, LV_LABEL_LONG_CLIP);
    g_feels = makeMetric(current, 194, 80, 148);
    g_range = makeMetric(current, 350, 80, 258);
    g_humidity = makeMetric(current, 194, 106, 148);
    g_wind = makeMetric(current, 350, 106, 130);
    g_precipitation = makeMetric(current, 490, 106, 118);

    lv_obj_t *headline = theme::label(list, &lv_font_montserrat_20, theme::text(),
                                      "3-Tage-Prognose");
    lv_obj_set_size(headline, kScreenWidth - 12, 22);

    lv_obj_t *forecastRow = theme::plainBox(list);
    lv_obj_set_size(forecastRow, kScreenWidth - 12, kForecastCardHeight);
    for (uint8_t index = 0; index < services::kWeatherForecastDays; ++index) {
        lv_obj_t *card = lv_obj_create(forecastRow);
        theme::styleCard(card);
        lv_obj_set_pos(card, index * (kForecastCardWidth + kForecastGap), 0);
        lv_obj_set_size(card, kForecastCardWidth, kForecastCardHeight);

        g_forecast[index].day = theme::label(card, &lv_font_montserrat_14, theme::text(), "--");
        lv_obj_set_pos(g_forecast[index].day, 8, 8);
        g_forecast[index].icon = createWeatherIcon(card, 76, 52);
        lv_obj_set_pos(g_forecast[index].icon.root, 8, 34);
        updateWeatherIcon(g_forecast[index].icon, 0, false);
        g_forecast[index].range =
            theme::label(card, &lv_font_montserrat_16, theme::muted(), "Keine Daten");
        lv_obj_set_pos(g_forecast[index].range, 92, 43);
        lv_obj_set_size(g_forecast[index].range, 100, 44);
    }
}

void tick(uint32_t) {
    const services::WeatherData &wx = services::weather().data();

    setLabelTextIfChanged(g_place, core::settings().locationName);
    setLabelTextIfChanged(g_status, services::weather().statusText());

    if (!wx.valid) {
        setLabelTextIfChanged(g_temp, "--");
        setLabelTextIfChanged(g_desc, "Keine Daten");
        setLabelTextIfChanged(g_feels, "Wetterdaten werden abgerufen");
        setLabelTextIfChanged(g_range, "");
        setLabelTextIfChanged(g_humidity, "");
        setLabelTextIfChanged(g_wind, "");
        setLabelTextIfChanged(g_precipitation, "");
        updateWeatherIcon(g_currentIcon, 0, false);
        for (ForecastUi &forecast : g_forecast) {
            setLabelTextIfChanged(forecast.day, "--");
            setLabelTextIfChanged(forecast.range, "Keine Daten");
            updateWeatherIcon(forecast.icon, 0, false);
        }
        return;
    }
    char temperature[24];
    snprintf(temperature, sizeof(temperature), "%.0f C", static_cast<double>(wx.temperature));
    setLabelTextIfChanged(g_temp, temperature);
    setLabelTextIfChanged(g_desc, services::weatherDescription(wx.code));
    updateWeatherIcon(g_currentIcon, wx.code, true);
    char feels[32];
    snprintf(feels, sizeof(feels), "Gefühlt %.0f C", static_cast<double>(wx.apparent));
    setLabelTextIfChanged(g_feels, feels);
    char range[48];
    snprintf(range, sizeof(range), "Min %.0f C / Max %.0f C", static_cast<double>(wx.todayMin),
             static_cast<double>(wx.todayMax));
    setLabelTextIfChanged(g_range, range);
    char humidity[32];
    snprintf(humidity, sizeof(humidity), "Luftfeuchte %d %%", wx.humidity);
    setLabelTextIfChanged(g_humidity, humidity);
    char wind[32];
    snprintf(wind, sizeof(wind), "Wind %.0f km/h", static_cast<double>(wx.windKmh));
    setLabelTextIfChanged(g_wind, wind);
    char precipitation[32];
    snprintf(precipitation, sizeof(precipitation), "Regen %d %%", wx.precipitationProb);
    setLabelTextIfChanged(g_precipitation, precipitation);

    for (uint8_t index = 0; index < services::kWeatherForecastDays; ++index) {
        const services::WeatherForecastDay &day = wx.forecast[index];
        ForecastUi &forecast = g_forecast[index];
        if (!day.valid) {
            setLabelTextIfChanged(forecast.day, "Keine Daten");
            setLabelTextIfChanged(forecast.range, "--");
            updateWeatherIcon(forecast.icon, 0, false);
            continue;
        }

        char label[24];
        formatForecastDay(day.date, label, sizeof(label));
        setLabelTextIfChanged(forecast.day, label);
        char range[48];
        snprintf(range, sizeof(range), "min %.0f C\nmax %.0f C",
                 static_cast<double>(day.low), static_cast<double>(day.high));
        setLabelTextIfChanged(forecast.range, range);
        updateWeatherIcon(forecast.icon, day.code, true);
    }
}

} // namespace weather_screen
} // namespace ui
