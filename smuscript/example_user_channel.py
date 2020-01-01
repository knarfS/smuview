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
user_device.add_user_channel("Results", "User")
result_ch = user_device.channels()["Results"]

# Show tab for the user device
UiProxy.add_device_tab(user_device)
# Add a plot view. We don't have to wait for the signal to be created, because we are using the channel here
UiProxy.add_plot_view(user_device.id(), smuview.DockArea.TopDockArea, user_device.channels()["Results"])

# Fill the user channel with some data
i = 0
while i<10000:
    ts = time.time()
    result_ch.push_sample(sin(i), ts, smuview.Quantity.Power, set(), smuview.Unit.Watt, 6, 3)
    time.sleep(0.25)
    i = i + 0.1
