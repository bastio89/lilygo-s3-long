#include "ui/theme.h"

namespace ui {
namespace theme {

lv_color_t bg() { return lv_color_hex(0x10141A); }
lv_color_t panel() { return lv_color_hex(0x1C232D); }
lv_color_t panelLight() { return lv_color_hex(0x27303C); }
lv_color_t accent() { return lv_color_hex(0x4FA3FF); }
lv_color_t text() { return lv_color_hex(0xECF1F7); }
lv_color_t muted() { return lv_color_hex(0x8B97A6); }
lv_color_t warn() { return lv_color_hex(0xFF8A3D); }
lv_color_t good() { return lv_color_hex(0x5BD68A); }

void styleCard(lv_obj_t *obj) {
    lv_obj_set_style_bg_color(obj, panel(), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, 10, 0);
    lv_obj_set_style_pad_all(obj, 8, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

lv_obj_t *plainBox(lv_obj_t *parent) {
    lv_obj_t *box = lv_obj_create(parent);
    lv_obj_set_style_bg_opa(box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(box, 0, 0);
    lv_obj_set_style_pad_all(box, 0, 0);
    lv_obj_set_style_radius(box, 0, 0);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    return box;
}

lv_obj_t *label(lv_obj_t *parent, const lv_font_t *font, lv_color_t color, const char *text) {
    lv_obj_t *obj = lv_label_create(parent);
    lv_obj_set_style_text_font(obj, font, 0);
    lv_obj_set_style_text_color(obj, color, 0);
    lv_label_set_text(obj, text);
    return obj;
}

lv_obj_t *button(lv_obj_t *parent, const char *caption, const lv_font_t *font, lv_coord_t x,
                 lv_coord_t y, lv_coord_t w, lv_coord_t h) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_style_radius(btn, 10, 0);
    lv_obj_set_style_bg_color(btn, panel(), 0);
    lv_obj_set_style_bg_color(btn, accent(), LV_STATE_PRESSED);
    lv_obj_set_style_shadow_width(btn, 0, 0);

    lv_obj_t *text = lv_label_create(btn);
    lv_obj_set_style_text_font(text, font, 0);
    lv_obj_set_style_text_color(text, theme::text(), 0);
    lv_label_set_text(text, caption);
    lv_obj_center(text);
    return btn;
}

lv_obj_t *buttonLabel(lv_obj_t *btn) { return lv_obj_get_child(btn, 0); }

} // namespace theme
} // namespace ui
