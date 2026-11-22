#pragma once
#include <algorithm>
#include <cmath>

// Where the parts of Tessera's alert card stand when the card fills the glass. Pure numbers, so a test on a computer
// checks them (tests/test_fill_layout.cpp); fullscreen_alerts.cpp does the LVGL work.
//
// The card is the glass less `margin` on each side. The buttons (one, or the two of show_alert_choice) stand along the
// bottom, in the middle. Above them, in a column in the middle of the card that is never wider than a line is easy to
// read, stand one under the other: the icon in a round disc, the title in a large font and the text. The three are one
// group in the middle of the room above the buttons: a short alert stands in the middle of the glass, a long one takes
// every whole line down to the buttons and ends in dots there.
namespace fullscreen_alerts {

// The sizes of the look, in this board's pixels (tessera::ui::px of the reference look), and the line heights of the
// card's fonts as the screen has them.
struct Metrics {
  int pad = 28;            // the room inside the card's edge
  int column_max = 760;    // the widest the column of words gets
  int button_h = 72;       // a button's height
  int button_min = 240;    // the narrowest a button gets, if the card is wide enough
  int button_max = 420;    // the widest
  int button_gap = 16;     // between two buttons
  int gap_disc = 22;       // between the disc and the title
  int gap_title = 14;      // between the title and the text
  int gap_buttons = 28;    // between the words and the buttons
  int icon_line = 48, title_line = 46, text_line = 32;
};

struct Fill {
  int card_w = 0, card_h = 0;
  int text_x = 0, text_w = 0;  // the column of the title and the text
  int disc_x = 0, disc_y = 0, disc = 0, disc_pad = 0;  // the icon's disc, and the room above the glyph in it
  int title_y = 0, title_h = 0;
  int text_y = 0, text_max = 0;  // where the text starts, and the most it may take (whole lines)
  int button_y = 0, button_h = 0;
  int button_x = 0, button_w = 0;    // the first button, the one every alert has (on the right of a pair)
  int button2_x = 0, button2_w = 0;  // the second: button2_w is 0 without one
  int room_top = 0, room_bottom = 0;  // the room above the buttons the group stands in the middle of
};

inline int whole_lines(int room, int line) { return line > 0 ? std::max(0, room) / line * line : std::max(0, room); }

// The group of disc, title and text in the middle of the room, for a text that takes `text_h` pixels.
inline void centre(Fill &f, const Metrics &m, int text_h) {
  text_h = std::max(0, std::min(text_h, f.text_max));
  const int group = f.disc + m.gap_disc + f.title_h + (text_h > 0 ? m.gap_title + text_h : 0);
  const int top = f.room_top + std::max(0, (f.room_bottom - f.room_top - group) / 2);
  f.disc_y = top;
  f.title_y = f.disc_y + f.disc + m.gap_disc;
  f.text_y = f.title_y + f.title_h + m.gap_title;
}

inline Fill fill(int canvas_w, int canvas_h, int margin, bool two, const Metrics &m) {
  Fill f;
  margin = std::max(0, std::min(margin, std::min(canvas_w, canvas_h) / 4));
  f.card_w = std::max(0, canvas_w - 2 * margin);
  f.card_h = std::max(0, canvas_h - 2 * margin);
  const int pad = std::min(m.pad, std::min(f.card_w, f.card_h) / 8);
  f.text_w = std::max(0, std::min(f.card_w - 2 * pad, m.column_max));
  f.text_x = (f.card_w - f.text_w) / 2;

  // The buttons: a third of the card's width each (a quarter for a pair), between button_min and button_max while the
  // card has room for that, never wider than the card less its edge.
  const int row = std::max(0, f.card_w - 2 * pad);
  f.button_h = std::min(m.button_h, std::max(0, f.card_h / 5));
  f.button_y = std::max(0, f.card_h - pad - f.button_h);
  if (two) {
    const int most = std::max(0, (row - m.button_gap) / 2);
    f.button_w = f.button2_w = std::min(most, std::min(m.button_max, std::max(m.button_min, f.card_w / 4)));
    const int both = 2 * f.button_w + m.button_gap;
    f.button2_x = (f.card_w - both) / 2;
    f.button_x = f.button2_x + f.button_w + m.button_gap;
  } else {
    f.button_w = std::min(row, std::min(m.button_max, std::max(m.button_min, f.card_w / 3)));
    f.button_x = (f.card_w - f.button_w) / 2;
  }

  // The disc around the icon: twice its glyph, smaller on low glass so the title and two lines still fit.
  f.room_top = pad;
  f.room_bottom = std::max(f.room_top, f.button_y - m.gap_buttons);
  f.title_h = m.title_line;
  const int room = f.room_bottom - f.room_top;
  const int beside = m.gap_disc + f.title_h + m.gap_title + 2 * m.text_line;  // what the disc must leave room for
  f.disc = std::max(m.icon_line, std::min(2 * m.icon_line, room - beside));
  f.disc_pad = std::max(0, (f.disc - m.icon_line) / 2);
  f.disc_x = (f.card_w - f.disc) / 2;
  f.text_max = whole_lines(room - f.disc - m.gap_disc - f.title_h - m.gap_title, m.text_line);
  centre(f, m, f.text_max);
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
