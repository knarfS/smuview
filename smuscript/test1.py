import smuview
import time

# Get all already connected devices from the session
#devices = Session.devices()

# Connect the demo device
devices = Session.connect_device("demo")
print("{} devices connected.".format(len(devices)))
print("")
# Sleep 1 second to give the newly connected device the chance to create
# data and the corresponding signals
time.sleep(1)

# Iterate and print through all devices
#for deviceId in devices:
#    device = devices[deviceId]
for device in devices:
    print("")
    print("device   = " + device.name())
    #print("deviceId = " + deviceId)

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
        for signal in signals:
            print("")
            print("    signal        = " + signal.name())
