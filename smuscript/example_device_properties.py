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
