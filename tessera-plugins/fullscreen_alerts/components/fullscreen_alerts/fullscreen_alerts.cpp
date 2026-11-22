#include "fullscreen_alerts.h"
#include "esphome/core/log.h"

namespace esphome::fullscreen_alerts {

static const char *const TAG = "fullscreen_alerts";

using fullscreen_layout::diagonal_tenths;
using fullscreen_layout::Fill;
using fullscreen_layout::Metrics;

namespace {
bool is_label(lv_obj_t *obj) { return obj != nullptr && lv_obj_check_type(obj, &lv_label_class); }
bool hidden(lv_obj_t *obj) { return obj == nullptr || lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN); }
int canvas_w() { return lv_display_get_horizontal_resolution(lv_display_get_default()); }
int canvas_h() { return lv_display_get_vertical_resolution(lv_display_get_default()); }
int line_of(const lv_font_t *font) { return font ? lv_font_get_line_height(font) : 0; }
int line_of(lv_obj_t *label) { return label ? line_of(lv_obj_get_style_text_font(label, LV_PART_MAIN)) : 0; }
// The screen's own font, or `otherwise` on a core that has none for it.
const lv_font_t *face(tessera::Font font, const lv_font_t *otherwise) {
  const lv_font_t *f = tessera::ui::font(font);
  return f ? f : otherwise;
}
// Glass of 6.5 inches and more takes the large fonts: a 7 or 10 inch screen is read from across the room.
constexpr int ROOMY_TENTHS = 65;
}  // namespace

// The alert's overlay is a child of LVGL's top layer that holds one card, whose first three children are the icon, the
// title and the subtitle (labels) and whose next two are the buttons, each with its label (packages/core.yaml, "ALERT
// OVERLAY"). A plugin's popup (a card with a close key and a room) and the screensaver's glass look nothing like it.
bool FullscreenAlerts::find() {
  if (parts_.card != nullptr && lv_obj_is_valid(parts_.card)) return true;
  parts_ = {};
  own_ = {};
  applied_ = false;
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

bool FullscreenAlerts::roomy() const {
  return tessera::ui::large() && diagonal_tenths(canvas_w(), canvas_h(), tessera::ui::mm(100)) >= ROOMY_TENTHS;
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
  if (parts_.card == nullptr) return;
  if (!lv_obj_is_valid(parts_.card)) {
    parts_ = {};
    own_ = {};
    applied_ = false;
    return;
  }
  // A card that closed goes back to Tessera's own, so the core lays out the next one with its own fonts.
  if (hidden(parts_.overlay)) {
    if (applied_) restore();
    return;
  }
  if (!wanted()) {
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
  using tessera::Font;
  const bool big = ui::large(), wide = roomy();
  if (!own_.kept) {
    own_.radius = lv_obj_get_style_radius(parts_.card, LV_PART_MAIN);
    own_.icon = lv_obj_get_style_text_font(parts_.icon, LV_PART_MAIN);
    own_.title = lv_obj_get_style_text_font(parts_.title, LV_PART_MAIN);
    own_.subtitle = lv_obj_get_style_text_font(parts_.subtitle, LV_PART_MAIN);
    own_.kept = true;
  }
  // Large glass: the icon in the large icon font of full-page tiles, the title in the card's large value font (the same
  // letters as the headline, larger), the text in the headline's. Smaller glass keeps the card's own fonts.
  const lv_font_t *icon_font = wide && icon_font_ != nullptr ? icon_font_->get_lv_font() : own_.icon;
  lv_obj_set_style_text_font(parts_.icon, icon_font, LV_PART_MAIN);
  const lv_font_t *title_font = wide ? face(Font::VALUE, own_.title) : own_.title;
  const lv_font_t *text_font = wide ? face(Font::HEADLINE, own_.subtitle) : own_.subtitle;
  lv_obj_set_style_text_font(parts_.title, title_font, LV_PART_MAIN);
  lv_obj_set_style_text_font(parts_.subtitle, text_font, LV_PART_MAIN);

  Metrics &m = metrics_;
  m.pad = ui::px(big ? 32 : 14);
  m.column_max = ui::px(big ? 760 : 420);
  m.button_h = ui::px(wide ? 80 : big ? 64 : 44);
  m.button_min = ui::px(big ? 240 : 120);
  m.button_max = ui::px(big ? 420 : 240);
  m.button_gap = ui::px(big ? 20 : 10);
  m.gap_disc = ui::px(big ? 24 : 8);
  m.gap_title = ui::px(big ? 12 : 6);
  m.gap_buttons = ui::px(big ? 32 : 12);
  m.icon_line = line_of(icon_font);
  m.title_line = line_of(title_font);
  m.text_line = line_of(text_font);
  const int margin = margin_px();
  const bool two = !hidden(parts_.button2);
  fill_ = fullscreen_layout::fill(canvas_w(), canvas_h(), margin, two, m);
  const Fill &f = fill_;

  lv_obj_set_style_radius(parts_.card, margin == 0 ? 0 : own_.radius, LV_PART_MAIN);
  lv_obj_set_size(parts_.card, f.card_w, f.card_h);
  lv_obj_align(parts_.card, LV_ALIGN_CENTER, 0, 0);

  // The icon in a round disc a shade of ink deeper than the card, in the card's colour or the plain one.
  lv_obj_set_size(parts_.icon, f.disc, f.disc);
  lv_obj_set_style_radius(parts_.icon, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(parts_.icon, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(parts_.icon,
                            lv_color_mix(ui::color(theme::INK), lv_obj_get_style_bg_color(parts_.card, LV_PART_MAIN), 28),
                            LV_PART_MAIN);
  lv_obj_set_style_pad_top(parts_.icon, f.disc_pad, LV_PART_MAIN);
  lv_obj_set_style_text_align(parts_.icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

  lv_obj_set_size(parts_.title, f.text_w, f.title_h);
  lv_obj_set_style_text_align(parts_.title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  // The text as high as its lines, up to the whole lines there are room for: the core sets its words after this call,
  // and the group moves to the middle again when their height changes (text_resized).
  lv_obj_set_width(parts_.subtitle, f.text_w);
  lv_obj_set_height(parts_.subtitle, LV_SIZE_CONTENT);
  lv_obj_set_style_max_height(parts_.subtitle, f.text_max, LV_PART_MAIN);
  lv_obj_set_style_text_align(parts_.subtitle, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  if (!applied_) lv_obj_add_event_cb(parts_.subtitle, text_resized, LV_EVENT_SIZE_CHANGED, this);

  // From the card's bottom right, as the core places them (its content area is measured from there).
  const int label_pad = ui::px(big ? 12 : 8);
  const auto place = [&](lv_obj_t *button, lv_obj_t *label, int x, int w) {
    lv_obj_set_size(button, w, f.button_h);
    lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -(f.card_w - x - w), -(f.card_h - f.button_y - f.button_h));
    lv_obj_set_size(label, std::max(0, w - 2 * label_pad), line_of(label));
  };
  place(parts_.button, parts_.label, f.button_x, f.button_w);
  if (two) place(parts_.button2, parts_.label2, f.button2_x, f.button2_w);
  if (!applied_) ESP_LOGI(TAG, "Alert over the whole glass: %d x %d%s", f.card_w, f.card_h, wide ? ", large fonts" : "");
  applied_ = true;
  centre();
}

// The disc, the title and the text as one group in the middle of the room above the buttons, for the text as high as
// it is now.
void FullscreenAlerts::centre() {
  if (!applied_) return;
  lv_obj_update_layout(parts_.subtitle);
  const char *words = lv_label_get_text(parts_.subtitle);
  const int text_h = words != nullptr && *words ? lv_obj_get_height(parts_.subtitle) : 0;
  fullscreen_layout::centre(fill_, metrics_, text_h);
  lv_obj_set_pos(parts_.icon, fill_.disc_x, fill_.disc_y);
  lv_obj_set_pos(parts_.title, fill_.text_x, fill_.title_y);
  lv_obj_set_pos(parts_.subtitle, fill_.text_x, fill_.text_y);
  lv_obj_invalidate(parts_.overlay);
}

void FullscreenAlerts::text_resized(lv_event_t *event) {
  auto *self = static_cast<FullscreenAlerts *>(lv_event_get_user_data(event));
  if (self != nullptr) self->centre();
}

// Tessera's own card again: the core places and sizes every part for each alert (runtime_tiles::alert_place), so only
// what it never sets goes back: the fonts, the alignment of the words, the icon's disc and width, the text's height
// limit and the card's corners.
void FullscreenAlerts::restore() {
  lv_obj_remove_event_cb_with_user_data(parts_.subtitle, text_resized, this);
  for (lv_obj_t *part : {parts_.icon, parts_.title, parts_.subtitle})
    lv_obj_remove_local_style_prop(part, LV_STYLE_TEXT_ALIGN, LV_PART_MAIN);
  for (lv_style_prop_t prop : {LV_STYLE_RADIUS, LV_STYLE_BG_OPA, LV_STYLE_BG_COLOR, LV_STYLE_PAD_TOP})
    lv_obj_remove_local_style_prop(parts_.icon, prop, LV_PART_MAIN);
  lv_obj_remove_local_style_prop(parts_.subtitle, LV_STYLE_MAX_HEIGHT, LV_PART_MAIN);
  lv_obj_set_size(parts_.icon, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  if (own_.kept) {
    lv_obj_set_style_text_font(parts_.icon, own_.icon, LV_PART_MAIN);
    lv_obj_set_style_text_font(parts_.title, own_.title, LV_PART_MAIN);
    lv_obj_set_style_text_font(parts_.subtitle, own_.subtitle, LV_PART_MAIN);
    lv_obj_set_style_radius(parts_.card, own_.radius, LV_PART_MAIN);
  }
  lv_obj_invalidate(parts_.overlay);
  applied_ = false;
  ESP_LOGD(TAG, "Alert card back to Tessera's own");
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
