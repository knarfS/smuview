# This file is part of the SmuView project.
#
# Copyright (C) 2022 François Revol <revol@free.fr>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import smuview
import time
import os
import signal
import threading

#
# This generates the mmdata.txt file as required by FlexBV to gather measurements
# for the OpenBoardData project.
#
# https://pldaniels.com/flexbv/
# https://openboarddata.org/
#

# Replace with the name and configuration of your multimeter
# eg. "hp-3478a:conn=libgpib/hp3478a"
device_name = "demo"

# Replace with the name of the channel to relay to FlexBV, or "" for the first one
# eg. "A1"
channel_name = ""

# Path to the mmdata.txt to generate or None for auto-detection
mmdata_path = "mmdata.txt"


## end of config variables

print("mmdata.txt generator for FlexBV")
print("")

tmp_path = mmdata_path.replace(".txt","") + ".tmp"

#if mmdata_path is None:
#    TODO: autodetect

# Connect the demo device (it's just one!)
print("Connecting to device '%s'" % device_name)
dmm_dev = Session.connect_device(device_name)[0]
channels = dmm_dev.channels()
if channel_name == "":
    channel_name = list(channels.keys())[0]
print("Using channel '%s'" % channel_name)

# XXX: set on channel or on "" ??
dmm_conf = dmm_dev.configurables()[channel_name]
dmm_conf.set_config(smuview.ConfigKey.MeasuredQuantity, (smuview.Quantity.Voltage, {smuview.QuantityFlag.Diode}))

# Add a user device
user_dev = Session.add_user_device()
user_dev_tab = UiProxy.add_device_tab(user_dev)

# Add a generic control view to the device tab
control_view_1 = UiProxy.add_control_view(user_dev_tab, smuview.DockArea.TopDockArea, dmm_dev.configurables()[""])

# Add a demo control view to the device tab
control_view_2 = UiProxy.add_control_view(user_dev_tab, smuview.DockArea.TopDockArea, dmm_dev.configurables()[channel_name])

# Add a value panel view to the device tab
value_panel_view_1 = UiProxy.add_value_panel_view(user_dev_tab, smuview.DockArea.TopDockArea, dmm_dev.channels()[channel_name])

time.sleep(1)

# Actually decimals but well
digits = 3
if smuview.ConfigKey.Digits in dmm_conf.getable_configs():
    digits = dmm_conf.get_int_config(smuview.ConfigKey.Digits)

quitting = threading.Event()

def handler(signum, frame):
    quitting.set()

signal.signal(signal.SIGINT, handler)

while not quitting.is_set():
    time.sleep(0.5)
    if os.path.exists(mmdata_path):
        continue
    # FlexBV digested the previous file
    s = dmm_dev.channels()[channel_name].actual_signal()
    # get the unit back from the signal name
    unit = s.name().split("[")[1].split(" ")[0]
    u_in = s.get_last_sample(True)[1]
    with open(tmp_path, 'w', encoding="utf-8") as f:
        f.write(("{:."+str(digits)+"f}").format(u_in) + unit)
    os.rename(tmp_path, mmdata_path)

Session.remove_device(user_dev)
Session.remove_device(dmm_dev)
print("Exited")
