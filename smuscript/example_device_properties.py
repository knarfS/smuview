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

# Get all connected devices from the session
devices = Session.devices()

# Iterate over all devices and print all properties
for deviceId in devices:
    device = devices[deviceId]
    print("")
    print("device   = " + device.name())
    print("deviceId = " + deviceId)

    configurables = device.configurables()
    for configruableId in configurables:
        configurable = configurables[configruableId]
        print("")
        print("  configurable   = " + configurable.name())
        print("  configurableId = " + configruableId)

    channels = device.channels()
    for channelId in channels:
        channel = channels[channelId]
        print("")
        print("  channel   = " + channel.name())
        print("  channelId = " + channelId)

        actual_signal = channel.actual_signal()
        if actual_signal != None:
            print("")
            print("    actual_signal   = " + actual_signal.name())
            print("      sample_count  = {}".format(actual_signal.sample_count()))
            print("      get_sample(1) = {}".format(actual_signal.get_sample(1, True)))
            print("      last_sample   = {}".format(actual_signal.get_last_sample(True)))

        signals = channel.signals()
        for signal in signals:
            print("")
            print("    signal          = " + signal.name())
            print("      sample_count  = {}".format(signal.sample_count()))
            print("      get_sample(1) = {}".format(signal.get_sample(1, True)))
            print("      last_sample   = {}".format(signal.get_last_sample(True)))
