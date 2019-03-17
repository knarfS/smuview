
mport smuview
import time

devices = Session.devices()
print(devices)

load = devices["arachnid-labs-re-load-pro:/dev/ttyUSB2"]
load_conf = load.configurables()["1"]
psu = devices["conrad-digi-35-cpu:/dev/ttyUSB0"]
psu_conf = psu.configurables()[""]
dmm1 = devices["hp-3478a:"]
dmm1_conf = dmm1.configurables()[""]

# Add channel for measurement values
load.add_user_channel("Results", "User")

# Init
load_conf.set_config(smuview.ConfigKey.CurrentLimit, .0)
psu_conf.set_config(smuview.ConfigKey.VoltageTarget, 5.0)
psu_conf.set_config(smuview.ConfigKey.CurrentLimit, 0.15)
#dmm1_conf.set_config(smuview.ConfigKey.MeasurementQunatity, smuview.Qunatity.Voltage)

d = .0
while d <= 0.1:
    load_conf.set_config(smuview.ConfigKey.CurrentLimit, d)
    time.sleep(0.5)
    d += 0.005

# Set values to a save state
load_conf.set_config(smuview.ConfigKey.CurrentLimit, .0)
