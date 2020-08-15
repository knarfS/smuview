# This file is part of the SmuView project.
#
# Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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
from math import sin

# Add user device
user_device = Session.add_user_device()
# Add a user channel for measurement values to user device
result_ch = user_device.add_user_channel("Results", "User")

# Set first dummy sample
result_ch.push_sample(0, time.time(), smuview.Quantity.Power, set(), smuview.Unit.Watt, 6, 3)
result_ch.actual_signal().set_name("My user signal")

# TEST
test_ch = user_device.add_user_channel("TestCh", "User")
eff_sig_1 = test_ch.add_signal(smuview.Quantity.PowerFactor, set(), smuview.Unit.Percentage)
eff_sig_2 = test_ch.add_signal(smuview.Quantity.PowerFactor, set(), smuview.Unit.Percentage, "Eff 10V")
eff_sig_3 = test_ch.add_signal(smuview.Quantity.PowerFactor, set(), smuview.Unit.Percentage, "Eff 18V")

# Show tab for the user device
user_device_tab = UiProxy.add_device_tab(user_device)
# Add a time plot view.
plot = UiProxy.add_time_plot_view(user_device_tab, smuview.DockArea.TopDockArea)
# We don't have to wait for the signal to be created, because we are using the channel here.
UiProxy.set_channel_to_time_plot_view(user_device_tab, plot, user_device.channels()["Results"])

# Add a time plot view.
plot_2 = UiProxy.add_time_plot_view(user_device_tab, smuview.DockArea.TopDockArea)
UiProxy.add_curve_to_time_plot_view(user_device_tab, plot_2, eff_sig_1)
UiProxy.add_curve_to_time_plot_view(user_device_tab, plot_2, eff_sig_2)
UiProxy.add_curve_to_time_plot_view(user_device_tab, plot_2, eff_sig_3)


# Fill the user channel with some data
print("Starting loop...")
i = 0
while i<10000:
    ts = time.time()
    result_ch.push_sample(sin(i), ts, smuview.Quantity.Power, set(), smuview.Unit.Watt, 6, 3)
    result_ch.actual_signal().set_name("My user signal " + str(ts))
    print("  new value for {} = {}".format(result_ch.actual_signal().name(), result_ch.actual_signal().get_last_sample(True)[1]))
    time.sleep(0.25)
    i = i + 0.1
