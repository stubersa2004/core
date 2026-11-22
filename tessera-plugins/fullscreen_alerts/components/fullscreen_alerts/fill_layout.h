#pragma once
#include <algorithm>
#include <cmath>

// Where the parts of Tessera's alert card stand when the card fills the glass. Pure numbers, so a test on a computer
// checks them (tests/test_fill_layout.cpp); fullscreen_alerts.cpp does the LVGL work.
//
// The card is the glass less `margin` on each side. Its icon, title and text stand in the middle of the card, one under
// the other, and the button (or the two buttons of show_alert_choice) along the bottom, in the middle. The icon and the
// title start a little below the top on tall glass, so a short alert does not hang against the edge, and the text takes
// every whole line between the title and the buttons.
namespace fullscreen_alerts {

// The sizes of the look, in this board's pixels (tessera::ui::px of the reference look), and the line heights of the
// card's fonts as the screen has them.
struct Metrics {
  int pad = 28;          // the room inside the card's edge
  int button_h = 72;     // a button's height
  int button_min = 240;  // the narrowest a button gets, if the card is wide enough
  int button_gap = 16;   // between two buttons
  int gap_icon = 18;     // between the icon and the title
  int gap_title = 14;    // between the title and the text
  int gap_buttons = 24;  // between the text and the buttons
  int icon_line = 48, title_line = 40, text_line = 30;
};

struct Fill {
  int card_w = 0, card_h = 0;
  int text_x = 0, text_w = 0;  // the column of the icon, the title and the text
  int icon_y = 0, icon_h = 0;
  int title_y = 0, title_h = 0;
  int text_y = 0, text_h = 0;
  int button_y = 0, button_h = 0;
  int button_x = 0, button_w = 0;    // the first button, the one every alert has (on the right of a pair)
  int button2_x = 0, button2_w = 0;  // the second: button2_w is 0 without one
};

inline int whole_lines(int room, int line) { return line > 0 ? std::max(0, room) / line * line : std::max(0, room); }

inline Fill fill(int canvas_w, int canvas_h, int margin, bool two, const Metrics &m) {
  Fill f;
  margin = std::max(0, std::min(margin, std::min(canvas_w, canvas_h) / 4));
  f.card_w = std::max(0, canvas_w - 2 * margin);
  f.card_h = std::max(0, canvas_h - 2 * margin);
  const int pad = std::min(m.pad, std::min(f.card_w, f.card_h) / 8);
  f.text_x = pad;
  f.text_w = std::max(0, f.card_w - 2 * pad);

  // The buttons: a third of the card's width each (a quarter for a pair), never narrower than button_min while the
  // card has room for it, never wider than the column.
  f.button_h = std::min(m.button_h, std::max(0, f.card_h / 5));
  f.button_y = std::max(0, f.card_h - pad - f.button_h);
  if (two) {
    const int most = std::max(0, (f.text_w - m.button_gap) / 2);
    f.button_w = f.button2_w = std::min(most, std::max(m.button_min, f.card_w / 4));
    const int row = 2 * f.button_w + m.button_gap;
    f.button2_x = (f.card_w - row) / 2;
    f.button_x = f.button2_x + f.button_w + m.button_gap;
  } else {
    f.button_w = std::min(f.text_w, std::max(m.button_min, f.card_w / 3));
    f.button_x = (f.card_w - f.button_w) / 2;
  }

  // The icon and the title, then the text down to the buttons. What is left beyond the icon, the title and two lines of
  // text goes a quarter above the icon, so the words stand a little lower on tall glass.
  f.icon_h = m.icon_line;
  f.title_h = m.title_line;
  const int bottom = f.button_y - m.gap_buttons;  // where the text must end
  const int head = f.icon_h + m.gap_icon + f.title_h + m.gap_title;
  const int spare = std::max(0, bottom - pad - head - 2 * m.text_line);
  f.icon_y = pad + spare / 4;
  f.title_y = f.icon_y + f.icon_h + m.gap_icon;
  f.text_y = f.title_y + f.title_h + m.gap_title;
  f.text_h = whole_lines(bottom - f.text_y, m.text_line);
  return f;
}

// The glass's diagonal in tenths of an inch, from its pixels and how many pixels make 100 mm on it (tessera::ui::mm).
inline int diagonal_tenths(int canvas_w, int canvas_h, int px_per_100mm) {
  if (px_per_100mm <= 0) return 0;
  // sqrt(w^2 + h^2) pixels * 100 mm / px_per_100mm / 25.4 mm per inch, in tenths
  const double pixels = std::sqrt(double(canvas_w) * canvas_w + double(canvas_h) * canvas_h);
  return static_cast<int>(pixels * 1000.0 / (px_per_100mm * 25.4) + 0.5);
}

}  // namespace fullscreen_alerts
