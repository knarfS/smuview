import smuview
import time

# Connect the demo device (it's just one!)
demo_dev = devices = Session.connect_device("demo")[0]

# Show tab for demo device
UiProxy.add_device_tab(demo_dev)

# Sleep 1 second to give the newly connected demo device the chance to create data and the corresponding signals. TODO
time.sleep(1)

# Get device id of the demo device
demo_dev_id = demo_dev.id()

# Get BaseTab for the demo device
#demo_dev_tab = UiProxy.get_base_tab_from_device_id(demo_dev_id);

# Add a data view to the device tab
UiProxy.add_data_view(demo_dev_id, smuview.DockArea.TopDockArea, demo_dev.channels()["A1"].actual_signal())

# Sleep 3 sec to finish the adding of the view in the MainThread, otherwise SmuView will crash... TODO
#time.sleep(3)
