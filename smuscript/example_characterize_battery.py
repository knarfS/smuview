# This file is part of the SmuView project.
#
# Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
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

# Connect devices
load_device = Session.connect_device("arachnid-labs-re-load-pro:conn=/dev/ttyUSB1")[0]
load_conf = load_device.configurables()["1"]
dmm_device = Session.connect_device("hp-3478a:conn=libgpib/hp3478a")[0]
dmm_conf = dmm_device.configurables()[""]

# Init device settings
load_conf.set_config(smuview.ConfigKey.CurrentLimit, .0)
# TODO: Setting of MeasuredQuantities is not working yet!
#dmm_conf.set_config(smuview.ConfigKey.MeasuredQuantity, smuview.Quantity.Voltage)

# Give the devices the chance to create signals
time.sleep(1)

# Add user device
user_device = Session.add_user_device()
# Add channel for measurement values
user_device.add_user_channel("Results", "User")
result_ch = user_device.channels()["Results"]

# Show device tabs and add plot to user device
UiProxy.add_device_tab(load_device)
UiProxy.add_device_tab(dmm_device)
user_device_tab = UiProxy.add_device_tab(user_device)
plot = UiProxy.add_time_plot_view(user_device_tab, smuview.DockArea.BottomDockArea)
UiProxy.set_channel_to_time_plot_view(user_device_tab, plot, result_ch)

# Start test
load_conf.set_config(smuview.ConfigKey.CurrentLimit, .150)

# Drain the battery until it is below 0.5 Volt
value = 100
while value > 0.5:
    # Take a reading every 2s and write it to the user channel
    time_stamp = time.time()
    value = dmm_device.channels()["P1"].actual_signal().get_last_sample(True)[1]))
    result_ch.push_sample(value, time_stamp, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 6, 5)
    time.sleep(2)

# Set device settings to a save state
load_conf.set_config(smuview.ConfigKey.CurrentLimit, .0)
