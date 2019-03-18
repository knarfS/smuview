import smuview

devices = Session.devices()

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
            print("    actual_signal = " + actual_signal.name())

        signals = channel.signals()
        #for signalId in signals:
        #    signal = signals[signalId]
        #    print("")
        #    print("    signal   = " + signal.name())
        #    print("    signalId = " + signalId)
