// Copyright Kutin Nikolay 2021
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt)

#include "../zigbee/zigbee.h"

using namespace zigbee;

#include "led1623g12.h"
#include "wxkg01lm.h"
#include "basiczbr3.h"
#include "gzcq01lm.h"
#include "wxkg03lm.h"
#include "mccgq01lm.h"
#include "wsdcgq01lm.h"
#include "qbkg11lm.h"

void addZigbeeDevicesSupport() {
    ADD_ZIGBEE_DEVICE_CLASS(WXKG01LM, "lumi.sensor_switch", "WXKG01LM");
    ADD_ZIGBEE_DEVICE_CLASS(LED1623G12, "TRADFRI bulb E27 W opal 1000lm", "LED1623G12");
    ADD_ZIGBEE_DEVICE_CLASS(BASICZBR3, "BASICZBR3", "BASICZBR3");
    ADD_ZIGBEE_DEVICE_CLASS(GZCGQ01LM, "lumi.sen_ill.mgl01", "GZCGQ01LM");
    ADD_ZIGBEE_DEVICE_CLASS(WXKG03LM, "lumi.remote.b186acn01", "WXKG03LM");
    ADD_ZIGBEE_DEVICE_CLASS(MCCGQ01LM, "lumi.sensor_magnet", "MCCGQ01LM");
    ADD_ZIGBEE_DEVICE_CLASS(WSDCGQ01LM, "lumi.sensor_ht", "WSDCGQ01LM");
    ADD_ZIGBEE_DEVICE_CLASS(QBKG11LM, "lumi.ctrl_ln1.aq1", "QBKG11LM");
}