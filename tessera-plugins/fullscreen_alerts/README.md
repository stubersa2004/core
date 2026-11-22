# Fullscreen alerts

> Alerts with a camera picture (doorbell) keep Tessera's own card, which makes room for the picture.

Tessera's alert card has the same size in millimetres on every screen: on a 2.8 or 4 inch screen it fills the glass,
on a 7 or 10 inch screen it is a small card in the middle. With this plugin every alert fills the whole screen,
whatever its size: the icon in a round disc, a large title and the text as one group in the middle, the button (or
the two buttons of `show_alert_choice`) large along the bottom. On screens of 6.5 inches and more the icon, the title
and the text take the screen's larger fonts, so the alert reads from across the room; the lines of text stay a
comfortable width on wide glass.

Nothing changes in your automations. `esphome.<screen>_show_alert`, `esphome.<screen>_show_alert_choice`, the
`esp_screens_show_alert` event and the alerts of Tessera's Events page all work as before; the plugin lays the card out
again the moment it shows. Its colour, timeout, flash and buttons stay Tessera's.

## Settings

In the plugin's details on the screen's Plugins tab, in Home Assistant (the screen's configuration entities) and on the
screen's own settings page:

| Setting | Default | What |
|---|---|---|
| Fullscreen alerts | on | Off gives Tessera's own card back. |
| Fullscreen alert margin | 0 mm | The room between the alert and the edge of the screen. 0 fills it edge to edge, with square corners. |
| Fullscreen alert from size | 0 in | Only on screens with at least this diagonal: 0 on every screen, 7 only on the 7 and 10 inch ones. |

## Set up

Tessera, Plugins, **Add with a link**, and paste the link of this folder on GitHub; or copy this folder to
`/config/tessera-plugins/fullscreen_alerts/` and add it from there. Then add it to a screen: the app builds it.
