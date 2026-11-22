#pragma once
#include "esphome/core/component.h"
#include "esphome/components/font/font.h"
#include "esphome/components/number/number.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/smart_display/plugin_api.h"
#include "fill_layout.h"

namespace esphome::fullscreen_alerts {

// The plugin: when Tessera's alert card shows (Plugin::on_alert, right after the core laid it out and before it is on
// the glass), it lays the card out again over the whole glass: the icon in a round disc, a large title and the text in
// the middle, large buttons along the bottom. It keeps an eye on the card every 250 ms (on_interval): a card the core
// lays out again (a turn of the glass) is fitted again; an alert with a camera picture, a setting turned off or a
// screen smaller than the chosen size gets Tessera's own card back, and so does every card once it closes.
class FullscreenAlerts : public Component, public tessera::Plugin {
 public:
  float get_setup_priority() const override { return setup_priority::LATE; }
  void on_ready() override { find(); }
  void on_alert() override { fit(); }
  void on_interval(uint32_t now_ms) override;
  bool settings(tessera::SettingsPage &page) override;

  void set_enabled(switch_::Switch *s) { enabled_ = s; }
  void set_margin(number::Number *n) { margin_ = n; }
  void set_from_size(number::Number *n) { from_size_ = n; }
  void set_icon_font(font::Font *f) { icon_font_ = f; }

 protected:
  // The parts of the alert card, as packages/core.yaml makes them on LVGL's top layer: the overlay, the card in it, and
  // in the card the icon, the title, the subtitle, the first and the second button (and a camera's frame after them).
  struct Parts {
    lv_obj_t *overlay = nullptr, *card = nullptr, *icon = nullptr, *title = nullptr, *subtitle = nullptr;
    lv_obj_t *button = nullptr, *label = nullptr, *button2 = nullptr, *label2 = nullptr;
  };
  // What the core gave the parts this plugin changes, put back by restore().
  struct Own {
    bool kept = false;
    int radius = 0;
    const lv_font_t *icon = nullptr, *title = nullptr, *subtitle = nullptr;
  };
  bool find();
  bool wanted() const;
  bool picture() const;
  bool roomy() const;
  void fit();
  void apply();
  void centre();
  void restore();
  int margin_px() const;
  static void text_resized(lv_event_t *event);

  switch_::Switch *enabled_{};
  number::Number *margin_{}, *from_size_{};
  font::Font *icon_font_{};  // the core's large icon font (plugin.yaml), for the disc on large glass
  Parts parts_;
  Own own_;
  fullscreen_layout::Fill fill_;
  fullscreen_layout::Metrics metrics_;
  bool applied_ = false;
};

}  // namespace esphome::fullscreen_alerts
