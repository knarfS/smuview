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

# Connect a device with a fixed channel
device = Session.connect_device("arachnid-labs-re-load-pro:conn=/dev/ttyUSB2")[0]
channel_name = "V"
# Connect a device with no fixed channels
#device = Session.connect_device("hp-3478a:conn=libgpib/hp3478a")[0]
#channel_name = "P1"

# Don't give the devices a chance to create a (non fixed) signal by receiving samples
signal = device.channels()[channel_name].actual_signal()

# Read signals from a fixed channel. This will only work for fixed channels!
print("signal.sample_count() = {0}".format(signal.sample_count()))
print("signal.get_last_sample() = {0}".format(signal.get_last_sample(True)))

# Sleep 1s to give the devices a chance to receive some samples
time.sleep(1)

# Read signals from channel
print("signal.sample_count() = {0}".format(signal.sample_count()))
print("signal.get_last_sample() = {0}".format(signal.get_last_sample(True)))

Session.remove_device(device)
