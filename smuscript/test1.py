import smuview
import time

devices = Session.devices()

for key in devices:
    print("key   = "+key)
    print("value = "+devices[key].id())
