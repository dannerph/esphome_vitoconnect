import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import (
    CONF_ADDRESS,
    CONF_LENGTH,
    CONF_MAX_VALUE,
    CONF_MIN_VALUE,
    CONF_STEP,
)

from .. import CONF_VITOCONNECT_ID, VitoConnect, vitoconnect_ns

DEPENDENCIES = ["vitoconnect"]

CONF_FACTOR = "factor"

OPTOLINKNumber = vitoconnect_ns.class_("OPTOLINKNumber", number.Number)


def validate_min_max(config):
    if config[CONF_MIN_VALUE] >= config[CONF_MAX_VALUE]:
        raise cv.Invalid(f"{CONF_MIN_VALUE} must be smaller than {CONF_MAX_VALUE}")
    if config[CONF_STEP] <= 0:
        raise cv.Invalid(f"{CONF_STEP} must be greater than 0")
    if config[CONF_FACTOR] == 0:
        raise cv.Invalid(f"{CONF_FACTOR} must not be 0")
    return config


CONFIG_SCHEMA = cv.All(
    number.number_schema(OPTOLINKNumber).extend(
        {
            cv.GenerateID(CONF_VITOCONNECT_ID): cv.use_id(VitoConnect),
            cv.Required(CONF_ADDRESS): cv.uint16_t,
            cv.Required(CONF_LENGTH): cv.one_of(1, 2, 4, int=True),
            cv.Required(CONF_MIN_VALUE): cv.float_,
            cv.Required(CONF_MAX_VALUE): cv.float_,
            cv.Required(CONF_STEP): cv.positive_float,
            # value = raw * factor  (e.g. 0.1 for temperatures in 1/10 °C)
            cv.Optional(CONF_FACTOR, default=1.0): cv.float_,
        }
    ),
    validate_min_max,
)


async def to_code(config):
    var = await number.new_number(
        config,
        min_value=config[CONF_MIN_VALUE],
        max_value=config[CONF_MAX_VALUE],
        step=config[CONF_STEP],
    )

    # Add configuration to datapoint
    cg.add(var.setAddress(config[CONF_ADDRESS]))
    cg.add(var.setLength(config[CONF_LENGTH]))
    cg.add(var.set_factor(config[CONF_FACTOR]))

    # Add number to component hub (VitoConnect)
    hub = await cg.get_variable(config[CONF_VITOCONNECT_ID])
    cg.add(var.set_parent(hub))
    cg.add(hub.register_datapoint(var))
