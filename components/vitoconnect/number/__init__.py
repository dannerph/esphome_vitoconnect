import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_ID, CONF_NAME, CONF_ADDRESS, CONF_LENGTH, CONF_DIV_RATIO, CONF_MAX_VALUE, CONF_MIN_VALUE, CONF_STEP #, CONF_TYPE 
from .. import vitoconnect_ns, VitoConnect, CONF_VITOCONNECT_ID

DEPENDENCIES = ["vitoconnect"]
OPTOLINKNumber = vitoconnect_ns.class_("OPTOLINKNumber", number.Number)

CONFIG_SCHEMA = number.number_schema(OPTOLINKNumber).extend({
    cv.GenerateID(): cv.declare_id(OPTOLINKNumber),
    cv.GenerateID(CONF_VITOCONNECT_ID): cv.use_id(VitoConnect),
    cv.Required(CONF_ADDRESS): cv.uint16_t,
    cv.Required(CONF_LENGTH): cv.uint8_t,
    cv.Required(CONF_MAX_VALUE): cv.float_,
    cv.Required(CONF_MIN_VALUE): cv.float_range(),
    cv.Required(CONF_STEP): cv.float_,
    cv.Optional(CONF_DIV_RATIO, default=1): cv.one_of(
            1, 2, 10, 3600, int=True
        ),
})

async def to_code(config):
    var = await number.new_number(
        config,
        min_value=config["min_value"],
        max_value=config["max_value"],
        step=config["step"]
    )


    # Add configuration to datapoint
    cg.add(var.setAddress(config[CONF_ADDRESS]))
    cg.add(var.setLength(config[CONF_LENGTH]))
    cg.add(var.setDivRatio(config[CONF_DIV_RATIO]))

    # Add sensor to component hub (VitoConnect)
    hub = await cg.get_variable(config[CONF_VITOCONNECT_ID])
    cg.add(hub.register_datapoint(var))
