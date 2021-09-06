// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#include "../functional_unit.h"

using namespace zigbee::GenericDevices;

#include "onoff.h"
#include "level_control.h"
#include "light.h"
#include "climate.h"
#include "illuminance_level.h"
#include "thermostat.h"
#include "alarm.h"

void addUnitsSupport() {
    ADD_UNIT_CLASS(OnOff, "onoff");
    ADD_UNIT_CLASS(Light, "light");
    ADD_UNIT_CLASS(Climate, "climate");
    ADD_UNIT_CLASS(IlluminanceLevel, "illuminance");
    ADD_UNIT_CLASS(Thermostat, "thermostat");
    ADD_UNIT_CLASS(Alarm, "alarm");
}