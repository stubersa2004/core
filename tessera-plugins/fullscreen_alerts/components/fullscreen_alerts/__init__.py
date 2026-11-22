"""Fullscreen alerts: Tessera's alert card over the whole glass (Plugin::on_alert), with three settings of plugin.yaml.

register_plugin() takes the id, version and the screen's texts from the manifest and translations/ beside this folder.
"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import font, number, smart_display, switch
from esphome.const import CONF_ID

DEPENDENCIES = ["smart_display", "font"]

CONF_ENABLED = "enabled"
CONF_MARGIN = "margin"
CONF_FROM_SIZE = "from_size"
CONF_ICON_FONT = "icon_font"

fullscreen_alerts_ns = cg.esphome_ns.namespace("fullscreen_alerts")
FullscreenAlerts = fullscreen_alerts_ns.class_("FullscreenAlerts", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(FullscreenAlerts),
        cv.Required(CONF_ENABLED): cv.use_id(switch.Switch),
        cv.Required(CONF_MARGIN): cv.use_id(number.Number),
        cv.Required(CONF_FROM_SIZE): cv.use_id(number.Number),
        cv.Optional(CONF_ICON_FONT): cv.use_id(font.Font),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await smart_display.register_plugin(var, __file__)
    cg.add(var.set_enabled(await cg.get_variable(config[CONF_ENABLED])))
    cg.add(var.set_margin(await cg.get_variable(config[CONF_MARGIN])))
    cg.add(var.set_from_size(await cg.get_variable(config[CONF_FROM_SIZE])))
    if CONF_ICON_FONT in config:
        cg.add(var.set_icon_font(await cg.get_variable(config[CONF_ICON_FONT])))
