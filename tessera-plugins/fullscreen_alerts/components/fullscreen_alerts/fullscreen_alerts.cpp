#include "fullscreen_alerts.h"
#include "fill_layout.h"
#include "esphome/core/log.h"

namespace esphome::fullscreen_alerts {

static const char *const TAG = "fullscreen_alerts";

namespace {
bool is_label(lv_obj_t *obj) { return obj != nullptr && lv_obj_check_type(obj, &lv_label_class); }
bool hidden(lv_obj_t *obj) { return obj == nullptr || lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN); }
int canvas_w() { return lv_display_get_horizontal_resolution(lv_display_get_default()); }
int canvas_h() { return lv_display_get_vertical_resolution(lv_display_get_default()); }
int line_of(lv_obj_t *label) {
  const lv_font_t *font = label ? lv_obj_get_style_text_font(label, LV_PART_MAIN) : nullptr;
  return font ? lv_font_get_line_height(font) : 0;
}
}  // namespace

// The alert's overlay is a child of LVGL's top layer that holds one card, whose first three children are the icon, the
// title and the subtitle (labels) and whose next two are the buttons, each with its label (packages/core.yaml, "ALERT
// OVERLAY"). A plugin's popup (a card with a close key and a room) and the screensaver's glass look nothing like it.
bool FullscreenAlerts::find() {
  if (parts_.card != nullptr && lv_obj_is_valid(parts_.card)) return true;
  parts_ = {};
  lv_obj_t *top = lv_layer_top();
  if (top == nullptr) return false;
  const uint32_t count = lv_obj_get_child_count(top);
  for (uint32_t i = 0; i < count; ++i) {
    lv_obj_t *overlay = lv_obj_get_child(top, static_cast<int32_t>(i));
    if (lv_obj_get_child_count(overlay) < 1) continue;
    lv_obj_t *card = lv_obj_get_child(overlay, 0);
    if (lv_obj_get_child_count(card) < 5) continue;
    lv_obj_t *icon = lv_obj_get_child(card, 0), *title = lv_obj_get_child(card, 1), *subtitle = lv_obj_get_child(card, 2);
    lv_obj_t *button = lv_obj_get_child(card, 3), *button2 = lv_obj_get_child(card, 4);
    if (!is_label(icon) || !is_label(title) || !is_label(subtitle)) continue;
    if (is_label(button) || is_label(button2) || lv_obj_get_child_count(button) < 1 || lv_obj_get_child_count(button2) < 1)
      continue;
    lv_obj_t *label = lv_obj_get_child(button, 0), *label2 = lv_obj_get_child(button2, 0);
    if (!is_label(label) || !is_label(label2)) continue;
    parts_ = {overlay, card, icon, title, subtitle, button, label, button2, label2};
    ESP_LOGD(TAG, "Found the alert card");
    return true;
  }
  ESP_LOGW(TAG, "No alert card on the top layer: alerts keep their own size");
  return false;
}

// A camera's frame (features/camera.yaml) comes after the buttons; the core shows it only with a picture.
bool FullscreenAlerts::picture() const {
  const uint32_t count = lv_obj_get_child_count(parts_.card);
  for (uint32_t i = 5; i < count; ++i)
    if (!hidden(lv_obj_get_child(parts_.card, static_cast<int32_t>(i)))) return true;
  return false;
}

int FullscreenAlerts::margin_px() const {
  const int mm = margin_ != nullptr && margin_->has_state() ? static_cast<int>(margin_->state + 0.5f) : 0;
  return mm > 0 ? tessera::ui::mm(mm) : 0;
}

bool FullscreenAlerts::wanted() const {
  if (enabled_ != nullptr && !enabled_->state) return false;
  const int from = from_size_ != nullptr && from_size_->has_state() ? static_cast<int>(from_size_->state + 0.5f) : 0;
  if (from > 0 && diagonal_tenths(canvas_w(), canvas_h(), tessera::ui::mm(100)) < from * 10) return false;
  return !picture();
}

void FullscreenAlerts::fit() {
  if (!find()) return;
  if (wanted()) apply();
  else if (applied_) restore();
}

void FullscreenAlerts::on_interval(uint32_t) {
  if (parts_.card == nullptr || hidden(parts_.overlay)) return;
  if (!lv_obj_is_valid(parts_.card)) {
    parts_ = {};
    applied_ = false;
    return;
  }
  const bool want = wanted();
  if (!want) {
    if (applied_) restore();
    return;
  }
  // The core lays the card out again for a picture or a turn of the glass: fit it again when it is no longer ours.
  const int margin = margin_px();
  if (!applied_ || lv_obj_get_width(parts_.card) != canvas_w() - 2 * margin ||
      lv_obj_get_height(parts_.card) != canvas_h() - 2 * margin)
    apply();
}

void FullscreenAlerts::apply() {
  namespace ui = tessera::ui;
  const bool big = ui::large();
  Metrics m;
  m.pad = ui::px(big ? 28 : 14);
  m.button_h = ui::px(big ? 72 : 44);
  m.button_min = ui::px(big ? 240 : 120);
  m.button_gap = ui::px(big ? 16 : 10);
  m.gap_icon = ui::px(big ? 18 : 8);
  m.gap_title = ui::px(big ? 14 : 6);
  m.gap_buttons = ui::px(big ? 24 : 12);
  m.icon_line = line_of(parts_.icon);
  m.title_line = line_of(parts_.title);
  m.text_line = line_of(parts_.subtitle);
  const int margin = margin_px();
  const bool two = !hidden(parts_.button2);
  const Fill f = fill(canvas_w(), canvas_h(), margin, two, m);

  if (radius_ < 0) radius_ = lv_obj_get_style_radius(parts_.card, LV_PART_MAIN);
  lv_obj_set_style_radius(parts_.card, margin == 0 ? 0 : radius_, LV_PART_MAIN);
  lv_obj_set_size(parts_.card, f.card_w, f.card_h);
  lv_obj_align(parts_.card, LV_ALIGN_CENTER, 0, 0);

  const auto column = [&](lv_obj_t *part, int y, int h) {
    lv_obj_set_pos(part, f.text_x, y);
    lv_obj_set_size(part, f.text_w, h);
    lv_obj_set_style_text_align(part, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  };
  column(parts_.icon, f.icon_y, f.icon_h);
  column(parts_.title, f.title_y, f.title_h);
  column(parts_.subtitle, f.text_y, f.text_h);

  // From the card's bottom right, as the core places them (its content area is measured from there).
  const int label_pad = ui::px(big ? 12 : 8);
  const auto place = [&](lv_obj_t *button, lv_obj_t *label, int x, int w) {
    lv_obj_set_size(button, w, f.button_h);
    lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -(f.card_w - x - w), -(f.card_h - f.button_y - f.button_h));
    lv_obj_set_size(label, std::max(0, w - 2 * label_pad), line_of(label));
  };
  place(parts_.button, parts_.label, f.button_x, f.button_w);
  if (two) place(parts_.button2, parts_.label2, f.button2_x, f.button2_w);
  lv_obj_invalidate(parts_.overlay);
  if (!applied_) ESP_LOGI(TAG, "Alert over the whole glass: %d x %d", f.card_w, f.card_h);
  applied_ = true;
}

// Tessera's own card again: the core places and sizes every part for each alert (runtime_tiles::alert_place), so only
// what it never sets goes back: the alignment of the words, the icon's own width and the card's corners.
void FullscreenAlerts::restore() {
  for (lv_obj_t *part : {parts_.icon, parts_.title, parts_.subtitle})
    lv_obj_remove_local_style_prop(part, LV_STYLE_TEXT_ALIGN, LV_PART_MAIN);
  lv_obj_set_size(parts_.icon, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  if (radius_ >= 0) lv_obj_set_style_radius(parts_.card, radius_, LV_PART_MAIN);
  lv_obj_invalidate(parts_.overlay);
  applied_ = false;
  ESP_LOGI(TAG, "Alert in Tessera's own size");
}

bool FullscreenAlerts::settings(tessera::SettingsPage &page) {
  page.icon = "\U000F009E";  // bell-ring
  page.toggle(text("fullscreen"), [this] { return enabled_ == nullptr || enabled_->state; },
              [this](bool on) { if (enabled_ != nullptr) on ? enabled_->turn_on() : enabled_->turn_off(); });
  const auto value = [](number::Number *n) { return n != nullptr && n->has_state() ? static_cast<int>(n->state + 0.5f) : 0; };
  const auto write = [](number::Number *n, int v) {
    if (n == nullptr) return;
    auto call = n->make_call();
    call.set_value(static_cast<float>(v));
    call.perform();
  };
  page.number(text("margin"), 0, 20, 1, text("unit_mm"), [this, value] { return value(margin_); },
              [this, write](int v) { write(margin_, v); });
  page.number(text("from_size"), 0, 13, 1, text("unit_inch"), [this, value] { return value(from_size_); },
              [this, write](int v) { write(from_size_, v); });
  return true;
}

}  // namespace esphome::fullscreen_alerts
