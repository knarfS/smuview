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

print("TEST UI")
print("=======")
print("")

# Connect the demo device (it's just one!)
demo_dev = Session.connect_device("demo")[0]
print("New demo device = " + demo_dev.id())

# Add a user device
user_dev = Session.add_user_device()

# Show tab for demo device
demo_dev_tab = UiProxy.add_device_tab(demo_dev)
print("New device tab (1) = " + demo_dev_tab)

# Show 2nd tab for demo device
demo_dev_tab_2 = UiProxy.add_device_tab(demo_dev)
print("New device tab (2) = " + demo_dev_tab_2)

# Show tab for user device
user_dev_tab = UiProxy.add_device_tab(user_dev)
print("New user tab = " + user_dev_tab)

# Sleep 1 second to give the newly connected demo device the chance to create data and the corresponding signals. TODO
time.sleep(1)

# Add a data view to the device tab
data_view = UiProxy.add_data_view(user_dev_tab, smuview.DockArea.TopDockArea, demo_dev.channels()["A1"].actual_signal())
print("New data view = " + data_view)
# Add a signal to the existing data view
UiProxy.add_signal_to_data_view(user_dev_tab, data_view, demo_dev.channels()["A3"].actual_signal())

# Add a generic control view to the device tab
control_view_1 = UiProxy.add_control_view(user_dev_tab, smuview.DockArea.TopDockArea, demo_dev.configurables()[""])
print("New control view (1) = " + control_view_1)

# Add a demo control view to the device tab
control_view_2 = UiProxy.add_control_view(user_dev_tab, smuview.DockArea.TopDockArea, demo_dev.configurables()["A1"])
print("New control view (2) = " + control_view_2)

# Add a time plot view (1) to the device tab
time_plot_view_1 = UiProxy.add_time_plot_view(user_dev_tab, smuview.DockArea.BottomDockArea)
print("New time plot view (1) = " + time_plot_view_1)
# Set a channel to time plot view (1)
UiProxy.set_channel_to_time_plot_view(user_dev_tab, time_plot_view_1, demo_dev.channels()["A1"])

# Add a time plot view (2) to the device tab
time_plot_view_2 = UiProxy.add_time_plot_view(user_dev_tab, smuview.DockArea.BottomDockArea)
print("New time plot view (2) = " + time_plot_view_2)
# Add a curve (1) to the time plot view (2)
curve_1 = UiProxy.add_curve_to_time_plot_view(user_dev_tab, time_plot_view_2, demo_dev.channels()["A2"].actual_signal())
print("New curve (1) = " + curve_1)
# Add a curve (2) to the time plot view (2)
curve_2 = UiProxy.add_curve_to_time_plot_view(user_dev_tab, time_plot_view_2, demo_dev.channels()["A3"].actual_signal())
print("New curve (2) = " + curve_2)

# Add a x/y plot view to the device tab
xy_plot_view = UiProxy.add_xy_plot_view(user_dev_tab, smuview.DockArea.BottomDockArea)
print("New x/y plot view = " + xy_plot_view)
# Add a curve (3) to the existing x/y plot view
curve_3 = UiProxy.add_curve_to_xy_plot_view(user_dev_tab, xy_plot_view, demo_dev.channels()["A1"].actual_signal(), demo_dev.channels()["A2"].actual_signal())
print("New curve (3) = " + curve_3)
# Add a curve (4) to the existing x/y plot view
curve_4 = UiProxy.add_curve_to_xy_plot_view(user_dev_tab, xy_plot_view, demo_dev.channels()["A0"].actual_signal(), demo_dev.channels()["A3"].actual_signal())
print("New curve (4) = " + curve_4)

# Add a power panel view to the device tab
power_panel_view = UiProxy.add_power_panel_view(user_dev_tab, smuview.DockArea.BottomDockArea, demo_dev.channels()["A1"].actual_signal(), demo_dev.channels()["A2"].actual_signal())
print("New powerpanel view = " + power_panel_view)

# Add a value panel view to the device tab
value_panel_view_1 = UiProxy.add_value_panel_view(user_dev_tab, smuview.DockArea.TopDockArea, demo_dev.channels()["A1"])
print("New valuepanel view (1) = " + value_panel_view_1)

# Add a value panel view to the device tab
value_panel_view_2 = UiProxy.add_value_panel_view(user_dev_tab, smuview.DockArea.TopDockArea, demo_dev.channels()["A1"].actual_signal())
print("New valuepanel view (2) = " + value_panel_view_2)

# Message box
msg_ret = UiProxy.show_message_box("Message Box", "This is a message box. The script will wait, until you press OK.")
if msg_ret:
    print("MessageBox: Was closed!")
else:
    print("MessageBox: Was canceled!")

# String input
s_input = UiProxy.show_string_input_dialog("Text Input", "Please enter a text:")
if s_input != None:
    print("StringInputDlg: " + s_input)
else:
    print("StringInputDlg: Was canceled!")

# Double input
d_input = UiProxy.show_double_input_dialog("Double Input", "Please enter a double number:")
if d_input != None:
    print("DoubleInputDlg: {}".format(d_input))
else:
    print("DoubleInputDlg: Was canceled!")

# Int input
i_input = UiProxy.show_int_input_dialog("Integer Input", "Please enter an integer number:")
if i_input != None:
    print("IntInputDlg: {}".format(i_input))
else:
    print("IntInputDlg: Was canceled!")

print("")
