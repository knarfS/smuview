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

# Connect the demo device (it's just one!)
demo_dev = Session.connect_device("demo")[0]

# Add a user device
user_dev = Session.add_user_device()

# Show tab for demo device
UiProxy.add_device_tab(demo_dev)

# Show tab for user device
UiProxy.add_device_tab(user_dev)

# Sleep 1 second to give the newly connected demo device the chance to create data and the corresponding signals. TODO
time.sleep(1)

# Get device id of user device
user_dev_id = user_dev.id()

# Add a data view to the device tab
data_view = UiProxy.add_data_view(user_dev_id, smuview.DockArea.TopDockArea, demo_dev.channels()["A1"].actual_signal())

# Add a control view to the device tab
UiProxy.add_control_view(user_dev_id, smuview.DockArea.TopDockArea, demo_dev.configurables()["A1"])

# Add plot view (1) with a channel to the device tab
plot_view_1 = UiProxy.add_plot_view(user_dev_id, smuview.DockArea.BottomDockArea, demo_dev.channels()["A1"])

# Add a plot view (2) with a signal to the device tab
plot_view_2 = UiProxy.add_plot_view(user_dev_id, smuview.DockArea.BottomDockArea, demo_dev.channels()["A1"].actual_signal());

# Add a x/y plot view (3) with two signals to the device tab
plot_view_3 = UiProxy.add_plot_view(user_dev_id, smuview.DockArea.BottomDockArea, demo_dev.channels()["A1"].actual_signal(), demo_dev.channels()["A2"].actual_signal())

# Add a power panel view to the device tab
UiProxy.add_power_panel_view(user_dev_id, smuview.DockArea.BottomDockArea, demo_dev.channels()["A1"].actual_signal(), demo_dev.channels()["A2"].actual_signal())

# Add a value panel view to the device tab
UiProxy.add_value_panel_view(user_dev_id, smuview.DockArea.TopDockArea, demo_dev.channels()["A1"])

# Add a value panel view to the device tab
UiProxy.add_value_panel_view(user_dev_id, smuview.DockArea.TopDockArea, demo_dev.channels()["A1"].actual_signal())

# Add a signal to the existing data view
UiProxy.add_signal_to_data_view(user_dev_id, data_view, demo_dev.channels()["A3"].actual_signal())

# Add a signal to the existing plot view (1)
UiProxy.add_signal_to_plot_view(user_dev_id, plot_view_1, demo_dev.channels()["A3"].actual_signal())

# Add a signal to the existing xy plot view (3)
UiProxy.add_signal_to_plot_view(user_dev_id, plot_view_3, demo_dev.channels()["A3"].actual_signal())

# Add a x/y signal to the existing xy plot view (3)
#UiProxy.add_signals_to_xy_plot_view(user_dev_id, plot_view_3, demo_dev.channels()["A0"].actual_signal(), demo_dev.channels()["A3"].actual_signal())

