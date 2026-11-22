# Vollbild-Benachrichtigungen

> Benachrichtigungen mit Kamerabild (Türklingel) behalten die normale Karte von Tessera, die Platz für das Bild macht.

Die Alert-Karte von Tessera ist auf jedem Bildschirm gleich gross in Millimetern: Auf einem 2,8- oder 4-Zoll-Display
füllt sie den Bildschirm, auf einem 7- oder 10-Zoll-Display ist sie eine kleine Karte in der Mitte. Mit diesem Plugin
füllt jede Benachrichtigung den ganzen Bildschirm, egal wie gross er ist: das Icon in einer runden Scheibe, ein grosser
Titel und der Text als Gruppe in der Mitte, der Knopf (oder die zwei Knöpfe von `show_alert_choice`) gross am unteren
Rand. Auf Bildschirmen ab 6,5 Zoll nehmen Icon, Titel und Text die grösseren Schriften des Bildschirms, damit man die
Benachrichtigung auch von weitem liest; die Textzeilen bleiben auf breiten Displays angenehm schmal.

An deinen Automationen ändert sich nichts. `esphome.<screen>_show_alert`, `esphome.<screen>_show_alert_choice`, das
Event `esp_screens_show_alert` und die Benachrichtigungen der Events-Seite von Tessera funktionieren wie bisher; das
Plugin legt die Karte in dem Moment neu aus, in dem sie erscheint. Farbe, Timeout, Blinken und Knöpfe bleiben die von
Tessera.

## Einstellungen

In den Details des Plugins im Plugins-Tab des Bildschirms, in Home Assistant (Konfigurations-Entitäten des
Bildschirms) und auf der Einstellungsseite des Bildschirms selbst:

| Einstellung | Standard | Was |
|---|---|---|
| Fullscreen alerts | an | Aus: wieder die normale Karte von Tessera. |
| Fullscreen alert margin | 0 mm | Abstand zum Bildschirmrand. 0 füllt den Bildschirm bis zum Rand, mit eckigen Ecken. |
| Fullscreen alert from size | 0 Zoll | Nur ab dieser Bildschirmdiagonale: 0 auf allen, 7 nur auf den 7- und 10-Zoll-Displays. |

## Einrichten

In Tessera unter Plugins **Mit Link hinzufügen** und den GitHub-Link dieses Ordners einfügen; oder diesen Ordner nach
`/config/tessera-plugins/fullscreen_alerts/` kopieren und von dort hinzufügen. Dann zu einem Bildschirm hinzufügen:
die App baut ihn neu.
