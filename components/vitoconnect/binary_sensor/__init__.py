import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, CONF_NAME, CONF_ADDRESS, CONF_LENGTH #, CONF_TYPE 
from .. import vitoconnect_ns, VitoConnect, CONF_VITOCONNECT_ID

DEPENDENCIES = ["vitoconnect"]
OPTOLINKBinarySensor = vitoconnect_ns.class_("OPTOLINKBinarySensor", binary_sensor.BinarySensor)

CONFIG_SCHEMA =  binary_sensor.binary_sensor_schema(OPTOLINKBinarySensor).extend({
    cv.GenerateID(): cv.declare_id(OPTOLINKBinarySensor),
    cv.GenerateID(CONF_VITOCONNECT_ID): cv.use_id(VitoConnect),
    cv.Required(CONF_ADDRESS): cv.uint16_t
})

async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)

    # Add configuration to datapoint
    cg.add(var.setAddress(config[CONF_ADDRESS]))
    cg.add(var.setLength(1))

    # Add sensor to component hub (VitoConnect)
    hub = await cg.get_variable(config[CONF_VITOCONNECT_ID])
    cg.add(hub.register_datapoint(var))
