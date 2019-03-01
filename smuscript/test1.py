import smuview
import time

devices = Session.devices()

for d in devices:
    print("device.name_str() = "+d.name_str())
    time.sleep(5)
