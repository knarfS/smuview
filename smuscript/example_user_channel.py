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
