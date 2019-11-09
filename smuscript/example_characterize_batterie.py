import smuview
import time

# Connect devices
load_device = Session.connect_device("arachnid-labs-re-load-pro:conn=/dev/ttyUSB1")[0]
load_conf = load_device.configurables()["1"]
dmm_device = Session.connect_device("hp-3478a:conn=libgpib/hp3478a")[0]
dmm_conf = dmm_device.configurables()[""]

# Init device settings
load_conf.set_config(smuview.ConfigKey.CurrentLimit, .0)
dmm_conf.set_config(smuview.ConfigKey.MeasuredQuantity, smuview.Quantity.Voltage)

# Give the devices the chance to create signals
time.sleep(1)

# Add user device
user_device = Session.add_user_device()
# Add channel for measurement values
user_device.add_user_channel("Results", "User")
result_ch = user_device.channels()["Results"]

# Show device tabs and add plot to user device
UiProxy.add_device_tab(load_device)
UiProxy.add_device_tab(dmm_device)
UiProxy.add_device_tab(user_device)
UiProxy.add_plot_view(user_device.id(), smuview.DockArea.BottomDockArea, result_ch)

# Start test
load_conf.set_config(smuview.ConfigKey.CurrentLimit, .150)

# Load the batterie until it is below 0.5 Volt
value = 100
while value > 0.2:
    # Take a reading every 2s and write it to the user channel
    time_stamp = time.time()
    value = dmm_device.channels()["P1"].actual_signal().get_last_sample(True)[1]
    result_ch.push_sample(value, time_stamp, smuview.Quantity.Voltage, set(), smuview.Unit.Volt, 6, 5)
    time.sleep(2)

# Set device settings to a save state
load_conf.set_config(smuview.ConfigKey.CurrentLimit, .0)
