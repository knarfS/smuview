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

# Test settings:
input_voltages = [8.0, 14.0, 20.0]
input_colors = [(255, 0, 0), (255, 128, 0), (255, 255, 0)]
max_current = 2.0
start_current = 0.01
current_steps = 0.005


# Connect all devices
load_dev = Session.connect_device("arachnid-labs-re-load-pro:conn=/dev/ttyUSB2")[0]
load_conf = load_dev.configurables()["1"]
psu_dev = Session.connect_device("scpi-pps:conn=libgpib/hp6632b")[0]
psu_conf = psu_dev.configurables()["1"]
dmm_dev = Session.connect_device("hp-3478a:conn=libgpib/hp3478a")[0]
dmm_conf = dmm_dev.configurables()[""]

# Init device settings
load_conf.set_config(smuview.ConfigKey.CurrentLimit, .0)
psu_conf.set_config(smuview.ConfigKey.VoltageTarget, .0)
psu_conf.set_config(smuview.ConfigKey.CurrentLimit, max_current)
psu_conf.set_config(smuview.ConfigKey.Enabled, True)
#dmm_conf.set_config(smuview.ConfigKey.MeasuredQuantity, smuview.Quantity.Current)

# Sleep 1s to give the devices the chance to create signals
time.sleep(1)

# Add user device and result channels/signals
user_dev = Session.add_user_device()
p_in_ch = user_dev.add_user_channel("P_in", "User")
p_in_sig = p_in_ch.add_signal(smuview.Quantity.Power, set(), smuview.Unit.Watt)
p_out_ch = user_dev.add_user_channel("P_out", "User")
p_out_sig = p_out_ch.add_signal(smuview.Quantity.Power, set(), smuview.Unit.Watt)
eff_ch = user_dev.add_user_channel("eff", "User")

# Show device tabs and plots
UiProxy.add_device_tab(load_dev)
UiProxy.add_device_tab(psu_dev)
UiProxy.add_device_tab(dmm_dev)
user_dev_tab = UiProxy.add_device_tab(user_dev)
p_plot = UiProxy.add_time_plot_view(user_dev_tab, smuview.DockArea.TopDockArea)
UiProxy.add_curve_to_time_plot_view(user_dev_tab, p_plot, p_in_sig)
UiProxy.add_curve_to_time_plot_view(user_dev_tab, p_plot, p_out_sig)
xy_plot = UiProxy.add_xy_plot_view(user_dev_tab, smuview.DockArea.TopDockArea)

for n in range(len(input_voltages)):
    u_in_set = input_voltages[n]
    eff_sig = eff_ch.add_signal(smuview.Quantity.PowerFactor, set(), smuview.Unit.Percentage, "eff@{}V".format(str(u_in_set)))
    curve = UiProxy.add_curve_to_xy_plot_view(user_dev_tab, xy_plot, p_out_sig, eff_sig)
    UiProxy.set_curve_name(user_dev_tab, xy_plot, curve, "V_in = {}V".format(str(u_in_set)))
    UiProxy.set_curve_color(user_dev_tab, xy_plot, curve, input_colors[n])
    load_conf.set_config(smuview.ConfigKey.CurrentLimit, .0)
    psu_conf.set_config(smuview.ConfigKey.VoltageTarget, u_in_set)
    # Wait for 10 s to let the DUT cool down
    time.sleep(10)
    current = start_current
    while current <= max_current:
        load_conf.set_config(smuview.ConfigKey.CurrentLimit, current)
        time.sleep(0.5)
        u_in = psu_dev.channels()["V1"].actual_signal().get_last_sample(True)[1]
        i_in = dmm_dev.channels()["P1"].actual_signal().get_last_sample(True)[1]
        power_in = u_in * i_in
        u_out = load_dev.channels()["V"].actual_signal().get_last_sample(True)[1]
        i_out = load_dev.channels()["I"].actual_signal().get_last_sample(True)[1]
        # MathChannels are not in the python bindings yet, so we have to calculate by our own.
        power_out = u_out * i_out
        eff = (power_out / power_in) * 100
        ts = time.time()
        p_in_ch.push_sample(power_in, ts, smuview.Quantity.Power, set(), smuview.Unit.Watt, 6, 3)
        p_out_ch.push_sample(power_out, ts, smuview.Quantity.Power, set(), smuview.Unit.Watt, 6, 3)
        eff_ch.push_sample(eff, ts, smuview.Quantity.PowerFactor, set(), smuview.Unit.Percentage, 6, 3)
        current += current_steps

# Set values to a save state
load_conf.set_config(smuview.ConfigKey.CurrentLimit, .0)
psu_conf.set_config(smuview.ConfigKey.VoltageTarget, .0)
psu_conf.set_config(smuview.ConfigKey.Enabled, False)
