import smuview
import time

devices = Session.devices()

for deviceId in devices:
    print("deviceId  = "+deviceId)
    #print("device    = "+devices[deviceId])

    device = devices[deviceId]
    print("device    = "+device.id())
    configurables = device.configurables()

    for configruableId in configurables:
        print("  configurableId = "+configruableId)
        #print("  configurable   = "+configurables[configruableId])
