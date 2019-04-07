import smuview
import time
from math import sin

# Add user device
#user_device = Session.add_user_device(None)
# Add channel for measurement values to user device
#user_device.add_user_channel("Results", "User")

devices = Session.devices()

# Demo device
demo_dev = devices["demo:0"]
if "Results" in demo_dev.channels():
    result_ch = demo_dev.channels()["Results"]
else:
    result_ch = demo_dev.add_user_channel("Results", "User")

i = 0
while i<10:
    ts = time.time()
    result_ch.push_sample(sin(i), ts, smuview.Quantity.Power, set(), smuview.Unit.Watt, 6, 3)
    time.sleep(0.25)
    i = i + 0.1
