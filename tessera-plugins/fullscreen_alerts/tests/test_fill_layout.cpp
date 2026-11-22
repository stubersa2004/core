// g++ -std=c++17 -I../components/fullscreen_alerts test_fill_layout.cpp && ./a.out
#include "fill_layout.h"
#include <cassert>
#include <cstdio>

using namespace fullscreen_alerts;

static void check(const char *name, int w, int h, int margin, bool two, const Metrics &m) {
  Fill f = fill(w, h, margin, two, m);
  std::printf("%-24s card %4d x %-4d column %3d disc %3d@%-3d title@%-3d text@%-3d max %-4d buttons@%-4d w%d/%d\n",
              name, f.card_w, f.card_h, f.text_w, f.disc, f.disc_y, f.title_y, f.text_y, f.text_max, f.button_y,
              f.button_w, f.button2_w);
  assert(f.card_w == w - 2 * margin && f.card_h == h - 2 * margin);
  assert(f.text_x >= 0 && f.text_x + f.text_w <= f.card_w && f.text_w <= m.column_max);
  assert(f.text_x == f.card_w - (f.text_x + f.text_w) || f.text_x + 1 == f.card_w - (f.text_x + f.text_w));
  assert(f.disc >= m.icon_line && f.disc <= 2 * m.icon_line && f.disc_x >= 0 && f.disc_x + f.disc <= f.card_w);
  assert(f.disc_y >= f.room_top && f.title_y >= f.disc_y + f.disc && f.text_y >= f.title_y + f.title_h);
  assert(f.text_max >= 2 * m.text_line);               // room for two lines at least
  assert(f.text_max % m.text_line == 0);               // whole lines only
  assert(f.text_y + f.text_max <= f.room_bottom);      // the longest text never runs into the buttons
  assert(f.button_y + f.button_h <= f.card_h);         // the buttons stay on the card
  assert(f.button_x >= 0 && f.button_x + f.button_w <= f.card_w && f.button_w <= m.button_max);
  if (two) {
    assert(f.button2_w == f.button_w && f.button2_x >= 0);
    assert(f.button2_x + f.button2_w < f.button_x);    // side by side, the first on the right
    assert(f.button2_x == f.card_w - (f.button_x + f.button_w));  // the pair in the middle
  } else {
    assert(f.button2_w == 0);
    assert(f.button_x == f.card_w - (f.button_x + f.button_w));   // in the middle
  }
  // A short alert: one line of text, or none, stands in the middle of the room above the buttons.
  for (int lines : {0, 1}) {
    centre(f, m, lines * m.text_line);
    const int group = f.disc + m.gap_disc + f.title_h + (lines ? m.gap_title + m.text_line : 0);
    const int above = f.disc_y - f.room_top, below = f.room_bottom - (f.disc_y + group);
    assert(above >= 0 && below >= 0 && (above - below) <= 1 && (below - above) <= 1);
  }
  // A text longer than the room takes the room and no more.
  centre(f, m, 100000);
  assert(f.text_y + f.text_max <= f.room_bottom);
}

int main() {
  Metrics large;  // the standard look's sizes at the reference density, the large fonts
  Metrics compact{14, 420, 44, 120, 240, 10, 8, 6, 12, 32, 26, 20};
  check("CYD 320x240", 320, 240, 0, false, compact);
  check("Guition 480x480", 480, 480, 0, true, large);
  check("7in 800x480", 800, 480, 0, false, large);
  check("7in 1024x600 margin", 1024, 600, 12, true, large);
  check("10in 1280x800", 1280, 800, 0, false, large);
  check("10in 800x1280 upright", 800, 1280, 0, true, large);
  // The diagonal: a 10.1 inch 1280 x 800 panel has about 149 px per inch, so 100 mm are about 587 px.
  const int d = diagonal_tenths(1280, 800, 587);
  std::printf("diagonal 1280x800 @ 587 px/100mm: %d.%d in\n", d / 10, d % 10);
  assert(d >= 98 && d <= 104);
  assert(diagonal_tenths(320, 240, 0) == 0);
  std::puts("ok");
}
