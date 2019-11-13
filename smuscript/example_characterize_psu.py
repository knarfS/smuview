import smuview
import time

# Connect all devices
load_dev = Session.connect_device("arachnid-labs-re-load-pro:conn=/dev/ttyUSB2")[0]
load_conf = load_dev.configurables()["1"]
psu_dev = Session.connect_device("scpi-pps:conn=libgpib/hp6632b")[0]
psu_conf = psu_dev.configurables()["1"]
dmm_dev = Session.connect_device("hp-3478a:conn=libgpib/hp3478a")[0]
dmm_conf = dmm_dev.configurables()[""]

# Init device settings
load_conf.set_config(smuview.ConfigKey.CurrentLimit, .0)
psu_conf.set_config(smuview.ConfigKey.VoltageTarget, 10.0)
psu_conf.set_config(smuview.ConfigKey.CurrentLimit, 2.000)
psu_conf.set_config(smuview.ConfigKey.Enabled, True)
#dmm_conf.set_config(smuview.ConfigKey.MeasuredQuantity, smuview.Quantity.Current)

# Sleep 1s to give the devices the chance to create signals
time.sleep(1)

# Add user device and result channels/signals
user_dev = Session.add_user_device()
p_in_ch = user_dev.add_user_channel("P_in", "User")
p_in_sig = p_in_ch.add_signal(smuview.Quantity.Power, set(), smuview.Unit.Watt)
p_out_ch = user_dev.add_user_channel("P_out", "User")
p_out_sig = p_out_ch.add_signal(smuview.Quantity.Power, set(), smuview.Unit.Watt)
eff_ch = user_dev.add_user_channel("eff", "User")
eff_sig = eff_ch.add_signal(smuview.Quantity.PowerFactor, set(), smuview.Unit.Percentage)

# Show device tabs and plots
UiProxy.add_device_tab(load_dev)
UiProxy.add_device_tab(psu_dev)
UiProxy.add_device_tab(dmm_dev)
UiProxy.add_device_tab(user_dev)
p_plot = UiProxy.add_plot_view(user_dev.id(), smuview.DockArea.TopDockArea, p_in_sig)
UiProxy.add_signal_to_plot(user_dev.id(), p_plot, p_out_sig)
UiProxy.add_plot_view(user_dev.id(), smuview.DockArea.TopDockArea, p_out_sig, eff_sig)

d = .0
while d <= 2.0:
    load_conf.set_config(smuview.ConfigKey.CurrentLimit, d)
    time.sleep(0.5)
    u_in = psu_dev.channels()["V1"].actual_signal().get_last_sample(True)[1]
    i_in = dmm_dev.channels()["P1"].actual_signal().get_last_sample(True)[1]
    power_in = u_in * i_in
    u_out = load_dev.channels()["V"].actual_signal().get_last_sample(True)[1]
    i_out = load_dev.channels()["I"].actual_signal().get_last_sample(True)[1]
    power_out = u_out * i_out
    eff = power_out / power_in
    ts = time.time()
    # MathChannels are not in the python bindings yet, so we have to calculate by our own.
    p_in_ch.push_sample(power_in, ts, smuview.Quantity.Power, set(), smuview.Unit.Watt, 6, 3)
    p_out_ch.push_sample(power_out, ts, smuview.Quantity.Power, set(), smuview.Unit.Watt, 6, 3)
    eff_ch.push_sample(eff, ts, smuview.Quantity.PowerFactor, set(), smuview.Unit.Percentage, 6, 3)
    d += 0.005

# Set values to a save state
load_conf.set_config(smuview.ConfigKey.CurrentLimit, .0)
psu_conf.set_config(smuview.ConfigKey.Enabled, False)
