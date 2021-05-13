# This file is part of the SmuView project.
#
# Copyright (C) 2021 Frank Stettner <frank-stettner@gmx.net>
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

#
# Script to test AnalogTimeSignal::combine_signals()
# TODO: Implement as unit test!
#

import time
import smuview

# Add user device and tab
user_device = Session.add_user_device()
tab = UiProxy.add_device_tab(user_device)


# Test to reproduce the bug fixed in PR #30
def test_pr30():
    start_ts = time.time()

    #  Add 2 channels
    ch1 = user_device.add_user_channel("CH1", "Test_PR30")
    ch2 = user_device.add_user_channel("CH2", "Test_PR30")

    # Push initial sample data
    ch1.push_sample(1, start_ts+1, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    ch1.push_sample(2, start_ts+3, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    ch2.push_sample(10, start_ts+4, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)

    # Add a xy-plot to get AnalogTimeSignal::combine_signals() called and force the error.
    # Without the fix from PR #30 SmuView will get stuck in an infinite loop!
    plot = UiProxy.add_xy_plot_view(tab, smuview.DockArea.TopDockArea)
    curve = UiProxy.add_curve_to_xy_plot_view(tab, plot, ch1.actual_signal(), ch2.actual_signal())

    # Push more data
    ch1.push_sample(3, start_ts+5, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    ch2.push_sample(9, start_ts+6, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)


# Test with an empty signal
def test_empty():
    start_ts = time.time()

    #  Add and prime 2 channels
    ch1 = user_device.add_user_channel("CH1", "Test_Empty")
    ch1.add_signal(smuview.Quantity.Voltage, set(), smuview.Unit.Volt)
    ch2 = user_device.add_user_channel("CH2", "Test_Empty")
    ch2.add_signal(smuview.Quantity.Voltage, set(), smuview.Unit.Volt)

    # Push initial sample data
    ch1.push_sample(1, start_ts+1, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    ch1.push_sample(2, start_ts+3, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)

    # Add a xy-plot to call AnalogTimeSignal::combine_signals()
    plot = UiProxy.add_xy_plot_view(tab, smuview.DockArea.TopDockArea)
    curve = UiProxy.add_curve_to_xy_plot_view(tab, plot, ch1.actual_signal(), ch2.actual_signal())

    # Push more data
    ch1.push_sample(3, start_ts+5, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    ch2.push_sample(10, start_ts+6, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)


# Test for improved AnalogTimeSignal::combine_signals()
def test_improved1():
    start_ts = time.time()

    #  Add 2 channels
    ch1 = user_device.add_user_channel("CH1", "Test_Improve")
    ch1.add_signal(smuview.Quantity.Voltage, set(), smuview.Unit.Volt)
    ch2 = user_device.add_user_channel("CH2", "Test_Improve")
    ch2.add_signal(smuview.Quantity.Voltage, set(), smuview.Unit.Volt)

    # Push initial sample data
    ch1.push_sample(1, start_ts+1, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    ch1.push_sample(2, start_ts+3, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    ch1.push_sample(3, start_ts+5, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)

    # Add a xy-plot to call AnalogTimeSignal::combine_signals()
    plot = UiProxy.add_xy_plot_view(tab, smuview.DockArea.TopDockArea)
    curve = UiProxy.add_curve_to_xy_plot_view(tab, plot, ch1.actual_signal(), ch2.actual_signal())

    # Push more data
    ch1.push_sample(4, start_ts+7, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    time.sleep(1)
    ch2.push_sample(10, start_ts+8, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    time.sleep(1)
    ch1.push_sample(5, start_ts+9, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    time.sleep(1)
    ch2.push_sample(9, start_ts+10, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    time.sleep(1)
    ch1.push_sample(6, start_ts+11, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    time.sleep(1)
    ch2.push_sample(8, start_ts+12, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    time.sleep(1)
    ch1.push_sample(7, start_ts+13, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    time.sleep(1)
    ch2.push_sample(7, start_ts+14, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    time.sleep(1)
    ch1.push_sample(8, start_ts+15, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    time.sleep(1)
    ch2.push_sample(6, start_ts+16, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    time.sleep(1)
    ch2.push_sample(5, start_ts+18, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    time.sleep(1)
    ch2.push_sample(4, start_ts+20, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    time.sleep(1)
    ch1.push_sample(9, start_ts+17, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)


# Another test for improved AnalogTimeSignal::combine_signals()
#
# Time | S1 | S2 | combined S1 | combined S1 |
# --------------------------------------------
#    1 |  1 |    |             |             |
#    3 |  2 |    |             |             |
#    5 |  3 |    |             |             |
#    6 |    | 10 |         3.5 |          10 |
#    7 |  4 |    |           4 |         9.5 |
#    8 |    |  9 |         4.5 |           9 |
#    9 |  5 |    |           5 |         8.5 |
#   10 |    |  8 |             |             |
#   12 |    |  7 |             |             |
#
def test_improved2():
    start_ts = time.time()

    #  Add 2 channels
    ch1 = user_device.add_user_channel("CH1", "Test_Improve")
    ch1.add_signal(smuview.Quantity.Voltage, set(), smuview.Unit.Volt)
    ch2 = user_device.add_user_channel("CH2", "Test_Improve")
    ch2.add_signal(smuview.Quantity.Voltage, set(), smuview.Unit.Volt)

    # Push initial sample data
    ch1.push_sample(1, start_ts+1, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    ch1.push_sample(2, start_ts+3, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    ch1.push_sample(3, start_ts+5, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    ch2.push_sample(10, start_ts+6, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    ch1.push_sample(4, start_ts+7, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    ch2.push_sample(9, start_ts+8, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    ch1.push_sample(5, start_ts+9, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    ch2.push_sample(8, start_ts+10, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)
    ch2.push_sample(7, start_ts+12, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 4, 2)

    # Add a xy-plot to call AnalogTimeSignal::combine_signals()
    plot = UiProxy.add_xy_plot_view(tab, smuview.DockArea.TopDockArea)
    curve = UiProxy.add_curve_to_xy_plot_view(tab, plot, ch1.actual_signal(), ch2.actual_signal())

# Call tests
test_pr30()
test_empty()
test_improved1()
test_improved2()
