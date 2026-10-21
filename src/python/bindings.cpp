/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <limits>
#include <memory>
#include <set>
#include <string>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "bindings.hpp"
#include "src/config.h"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/hardwarechannel.hpp"
#include "src/channels/userchannel.hpp"
#include "src/data/analogbasesignal.hpp"
#include "src/data/analogsamplesignal.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/devices/userdevice.hpp"
#include "src/python/bindingsenums.hpp"
#include "src/python/pystreambuf.hpp"
#include "src/python/uiproxy.hpp"

using std::set;

using namespace pybind11::literals; // for the ""_a
namespace py = pybind11;

/*
 * NOTE: The documentation for the smuview module is optimized for pdoc3!
 */

PYBIND11_EMBEDDED_MODULE(smuview, module) {
	// Set options for proper docstring.
	py::options options;
	options.enable_function_signatures();
	options.disable_enum_members_docstring();

	module.doc() = "The SmuView " SV_VERSION_STRING " Python bindings.\n\n"
		"The Python bindings are a scripting extension for SmuView to automate, "
		"setup and control complex or repetitive measurements, to process the "
		"incoming data and to create a standardized user interface for those "
		"measurements.\n\n"
		"The smuview module offers two default object instances: `Session` and "
		"`UiProxy`.\n"
		"The `Session` object gives access to already connected devices or connects "
		"new devices. The returned device object can then be used to read data "
		"from the device or control the device.\n"
		"The `UiProxy` object instance is used to modify the user interface, for "
		"example adding tabs or views.\n\n"
		"Here is a short example that connects the HP 3478A DMM via GPIB, reads "
		"a sample and creates the default tab for the device:\n"
		"```python\n"
		"import smuview\n"
		"import time\n\n"
		"# Connect device.\n"
		"dmm_dev = Session.connect_device(\"hp-3478a:conn=libgpib/hp3478a\")[0]\n"
		"# Sleep 1s to give the devices the chance to create signals.\n"
		"time.sleep(1)\n"
		"# Get last sample from channel P1.\n"
		"sample = dmm_dev.channels()[\"P1\"].actual_signal().get_last_sample(True)\n"
		"print(sample)\n\n"
		"# Add default tab for the DMM device.\n"
		"UiProxy.add_device_tab(dmm_dev)\n"
		"```\n\n"
		"For more example scripts, please have a look into the `smuscript` folder.";

	// pdoc attribute to render numpy docstrings.
	module.add_object("__docformat__", py::str("numpy"), false);
	// pdoc3 dictionary for enum documentation.
	module.add_object("__pdoc__", py::dict(), false);

	// NOTE: The order of initialization is very important! Otherwise types
	//       could be unknown when pybind11 is generating the function
	//       signatures.
	init_Enums(module);
	init_Signal(module);
	init_Channel(module);
	init_Configurable(module);
	init_Device(module);
	init_Session(module);
	init_UI(module);
	init_StreamBuf(module);
}

void init_Session(py::module &module)
{
	py::class_<sv::Session> py_session(module, "Session");
	py_session.doc() = "The SmuView `Session` class for accessing the actual state of the application.";
	py_session.def("devices", &sv::Session::device_map,
		"Return all connected devices.\n"
		"The device id is an (in almost all cases) unique string containing the "
		"manufacturer, the device model name and a more or less unique idenifier "
		"like the devices' serial number or the connection port (e.g. `COM5`, "
		"`/dev/ttyUSB2`).\n\n"
		"Returns\n"
		"-------\n"
		"Dict[str, BaseDevice]\n"
		"    A Dict where the key is the device id and the value is the device object.");
	py_session.def("connect_device", &sv::Session::connect_device,
		py::arg("conn_str"),
		"Connect a new device. For some devices (like DMMs) you may want to "
		"wait a fixed time, until the first sample has arrived and an `AnalogSignal` "
		"object has been created. Example:\n"
		"```python\n"
		"import smuview\n"
		"import time\n\n"
		"# Connect device.\n"
		"dmm_dev = Session.connect_device(\"hp-3478a:conn=libgpib/hp3478a\")[0]\n"
		"# Sleep 1s to give the devices the chance to create signals.\n"
		"time.sleep(1)\n"
		"```\n\n"
		"Parameters\n"
		"----------\n"
		"conn_str : str\n"
		"    The connection string. See https://sigrok.org/wiki/Connection_parameters\n\n"
		"Returns\n"
		"-------\n"
		"List[HardwareDevice]\n"
		"    A List with the newly connected device objects.");
	py_session.def("add_user_device", &sv::Session::add_user_device,
		"Create a new user device.\n\n"
		"Returns\n"
		"-------\n"
		"UserDevice\n"
		"    The created user device object.");
	py_session.def("remove_device", &sv::Session::remove_device,
		py::arg("device"),
		"Close a device and remove it from the session. This will also delete all aquired data!\n\n"
		"Parameters\n"
		"-------\n"
		"device : BaseDevice\n"
		"    The device to remove.");

}

void init_Device(py::module &module)
{
	py::class_<sv::devices::BaseDevice, std::shared_ptr<sv::devices::BaseDevice>> py_base_device(module, "BaseDevice");
	py_base_device.doc() = "The base class for all device types.";
	py_base_device.def("name", &sv::devices::BaseDevice::name,
		"Return the name of the device.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The name of the device.");
	py_base_device.def("id", &sv::devices::BaseDevice::id,
		"Return the unique id of the device.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The id of the device.");
	py_base_device.def("channels", &sv::devices::BaseDevice::channel_map,
		"Return all channels of the device.\n\n"
		"Returns\n"
		"-------\n"
		"Dict[str, BaseChannel]\n"
		"    A Dict where the key is the id of the channel and the value is the channel object.");
	py_base_device.def("configurables", &sv::devices::BaseDevice::configurable_map,
		"Return all configurables of the device.\n\n"
		"Returns\n"
		"-------\n"
		"Dict[str, Configurable]\n"
		"    A Dict where the key is the id of the `Configurable` and the value is the `Configurable` object.");
	py_base_device.def("add_user_channel", &sv::devices::BaseDevice::add_user_channel,
		py::arg("channel_name"), py::arg("channel_group_name"),
		"Add a new user channel to the device.\n\n"
		"Parameters\n"
		"----------\n"
		"channel_name : str\n"
		"    The name of the new user channel.\n"
		"channel_group_name : str\n"
		"    The name of the channel group where to create the user channel. Can be empty.\n\n"
		"Returns\n"
		"-------\n"
		"UserChannel\n"
		"    The new user channel object.");

	py::class_<sv::devices::HardwareDevice, std::shared_ptr<sv::devices::HardwareDevice>> py_hardware_device(module, "HardwareDevice", py_base_device);
	py_hardware_device.doc() = "An actual hardware device.";

	py::class_<sv::devices::UserDevice, std::shared_ptr<sv::devices::UserDevice>> py_user_device(module, "UserDevice", py_base_device);
	py_user_device.doc() = "An user generated (virtual) device for storing custom data and showing a custom tab.";
}

void init_Channel(py::module &module)
{
	py::class_<sv::channels::BaseChannel, std::shared_ptr<sv::channels::BaseChannel>> py_base_channel(module, "BaseChannel");
	py_base_channel.doc() = "The base class for all channel types.";
	py_base_channel.def("name", &sv::channels::BaseChannel::name,
		"Return the name of the channel.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The name of the channel.");
	py_base_channel.def("add_signal",
		(shared_ptr<sv::data::BaseSignal> (sv::channels::BaseChannel::*)
		(sv::data::Quantity, set<sv::data::QuantityFlag>, sv::data::Unit, std::string))
			&sv::channels::BaseChannel::add_signal,
		py::arg("quantity"), py::arg("quantity_flags"), py::arg("unit"),
		py::arg("custom_name") = "",
		"Add a new signal to the channel.\n\n"
		"Parameters\n"
		"----------\n"
		"quantity : Quantity\n"
		"    The `Quantity` of the new signal.\n"
		"quantity_flags : Set[QuantityFlag]\n"
		"    The `QuantityFlag`s of the new signal.\n"
		"unit : Unit\n"
		"    The `Unit` of the new signal.\n"
		"custom_name: str\n"
		"    A custom name for the new signal. If empty (default), the signal name will be automatically generated.\n\n"
		"Returns\n"
		"-------\n"
		"BaseSignal\n"
		"    The new signal object.");
	py_base_channel.def("actual_signal", &sv::channels::BaseChannel::actual_signal,
		"Return the actual signal of the channel.\n\n"
		"Returns\n"
		"-------\n"
		"BaseSignal\n"
		"    The actual signal object.");
	py_base_channel.def("signals", &sv::channels::BaseChannel::signals,
		"Return all signals of the channel.\n\n"
		"Returns\n"
		"-------\n"
		"List[BaseSignal]\n"
		"    All signals of the channel.");

	py::class_<sv::channels::HardwareChannel, std::shared_ptr<sv::channels::HardwareChannel>> py_hardware_channel(module, "HardwareChannel", py_base_channel);
	py_hardware_channel.doc() = "An actual hardware channel";

	py::class_<sv::channels::UserChannel, std::shared_ptr<sv::channels::UserChannel>> py_user_channel(module, "UserChannel", py_base_channel);
	py_user_channel.doc() = "An user generated channel for storing custom data.";
	py_user_channel.def("push_sample", &sv::channels::UserChannel::push_sample,
		py::arg("sample"), py::arg("timestamp"), py::arg("quantity"),
		py::arg("quantity_flags"), py::arg("unit"), py::arg("digits"),
		py::arg("decimal_places"),
		"Push a single sample to the channel.\n\n"
		"Parameters\n"
		"----------\n"
		"sample : double\n"
		"    The sample value.\n"
		"timestamp : double\n"
		"    The absolute timestamp in milliseconds.\n"
		"quantity : Quantity\n"
		"    The `Quantity` of the new signal.\n"
		"quantity_flags : Set[QuantityFlag]\n"
		"    The `QuantityFlag`s of the new signal.\n"
		"unit : Unit\n"
		"    The `Unit` of the new signal.\n"
		"digits : int\n"
		"    The total number of digits.\n"
		"decimal_places : int\n"
		"    The number of decimal places.");
}

void init_Signal(py::module &module)
{
	/*
	 * TODO:
	 *  - get_value_at_timestamp(): reference parameter &value
	 */

	py::class_<sv::data::BaseSignal, std::shared_ptr<sv::data::BaseSignal>> py_base_signal(module, "BaseSignal");
	py_base_signal.doc() = "The base class for all signal types.";
	py_base_signal.def("name", &sv::data::BaseSignal::name,
		"Return the name of the signal.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The name of the signal.");
	py_base_signal.def("set_name", &sv::data::BaseSignal::set_name,
		"Set a custom name for the signal.\n\n"
		"Parameters\n"
		"----------\n"
		"custom_name : str\n"
		"    A custom name for the signal. If empty, the signal name will be automatically generated.");
	py_base_signal.def("sample_count", &sv::data::BaseSignal::sample_count,
		"Return the number of samples of the signal.\n\n"
		"Returns\n"
		"-------\n"
		"int\n"
		"    The number of samples.");

	py::class_<sv::data::AnalogTimeSignal, std::shared_ptr<sv::data::AnalogTimeSignal>> py_analog_time_signal(module, "AnalogTimeSignal", py_base_signal);
	py_analog_time_signal.doc() = "A signal with time-value pairs.";
	py_analog_time_signal.def("get_sample", &sv::data::AnalogTimeSignal::get_sample,
		py::arg("pos"), py::arg("relative_time"),
		"Return the sample at the given position.\n\n"
		"Parameters\n"
		"----------\n"
		"pos : int\n"
		"    The position/number of the sample.\n"
		"relative_time : bool\n"
		"    When `True`, the returned timestamp is relative to the start of the SmuView session.\n\n"
		"Returns\n"
		"-------\n"
		"Tuple[double, double]\n"
		"    The sample with 1. timestamp in milliseconds and 2. the sample value.");
	py_analog_time_signal.def("get_last_sample", &sv::data::AnalogTimeSignal::get_last_sample,
		py::arg("relative_time"),
		"Return the last sample of the signal.\n\n"
		"Parameters\n"
		"----------\n"
		"relative_time : bool\n"
		"    When `True`, the returned timestamp is relative to the start of the SmuView session.\n\n"
		"Returns\n"
		"-------\n"
		"Tuple[double, double]\n"
		"    The sample with 1. timestamp in milliseconds and 2. the sample value.");
	py_analog_time_signal.def("push_sample", &sv::data::AnalogTimeSignal::push_sample,
		py::arg("sample"), py::arg("timestamp"), py::arg("digits"), py::arg("decimal_places"),
		"Push a new sample to the signal.\n\n"
		"Parameters\n"
		"----------\n"
		"sample : double\n"
		"    The sample value.\n"
		"timestamp : double\n"
		"    The absolute timestamp in milliseconds.\n"
		"digits : int\n"
		"    The total number of digits.\n"
		"decimal_places : int\n"
		"    The number of decimal places.");

	py::class_<sv::data::AnalogSampleSignal, std::shared_ptr<sv::data::AnalogSampleSignal>> py_analog_sample_signal(module, "AnalogSampleSignal", py_base_signal);
	py_analog_sample_signal.doc() = "A signal with key-value pairs.";
	py_analog_sample_signal.def("get_sample", &sv::data::AnalogSampleSignal::get_sample,
		py::arg("pos"),
		"Return the sample for the given position.\n\n"
		"Parameters\n"
		"----------\n"
		"pos : int\n"
		"    The position/number of the sample.\n\n"
		"Returns\n"
		"-------\n"
		"Tuple[int, double]\n"
		"    The sample with 1. the key and 2. the sample value.");
	py_analog_sample_signal.def("push_sample", &sv::data::AnalogSampleSignal::push_sample,
		py::arg("sample"), py::arg("pos"), py::arg("digits"), py::arg("decimal_places"),
		"Push a new sample to the signal.\n\n"
		"Parameters\n"
		"----------\n"
		"sample : double\n"
		"    The sample value.\n"
		"pos : int\n"
		"    The key (position) of the new sample.\n"
		"digits : int\n"
		"    The total number of digits.\n"
		"decimal_places : int\n"
		"    The number of decimal places.");
}

void init_Configurable(py::module &module)
{
	/*
	 * TODO:
	 *  - list
	 */

	py::class_<sv::devices::Configurable, std::shared_ptr<sv::devices::Configurable>> py_configurable(module, "Configurable");
	py_configurable.doc() = "A configurable for controlling a device with config keys.";
	py_configurable.def("name", &sv::devices::Configurable::name,
		"Return the name of the configurable.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The name of the configurable.");
	py_configurable.def("set_config", &sv::devices::Configurable::set_config<bool>,
		py::arg("config_key"), py::arg("value"),
		"Set a boolean value to the given config key.\n\n"
		"Parameters\n"
		"----------\n"
		"config_key : ConfigKey\n"
		"    The `ConfigKey` to set.\n"
		"value : bool\n"
		"    The bool value to set.");
	py_configurable.def("set_config", &sv::devices::Configurable::set_config<int32_t>,
		py::arg("config_key"), py::arg("value"),
		"Set an integer value to the given config key.\n\n"
		"Parameters\n"
		"----------\n"
		"config_key : ConfigKey\n"
		"    The `ConfigKey` to set.\n"
		"value : int\n"
		"    The int value to set.");
	py_configurable.def("set_config", &sv::devices::Configurable::set_config<uint64_t>,
		py::arg("config_key"), py::arg("value"),
		"Set an unsigned integer value to the given config key.\n\n"
		"Parameters\n"
		"----------\n"
		"config_key : ConfigKey\n"
		"    The `ConfigKey` to set.\n"
		"value : int\n"
		"    The (unsigned) int value to set.");
	py_configurable.def("set_config", &sv::devices::Configurable::set_config<double>,
		py::arg("config_key"), py::arg("value"),
		"Set a double value to the given config key.\n\n"
		"Parameters\n"
		"----------\n"
		"config_key : ConfigKey\n"
		"    The `ConfigKey` to set.\n"
		"value : float\n"
		"    The float value to set.");
	py_configurable.def("set_config", &sv::devices::Configurable::set_config<std::string>,
		py::arg("config_key"), py::arg("value"),
		"Set a string value to the given config key.\n\n"
		"Parameters\n"
		"----------\n"
		"config_key : ConfigKey\n"
		"    The `ConfigKey` to set.\n"
		"value : str\n"
		"    The string value to set.");
	py_configurable.def("set_config", &sv::devices::Configurable::set_measured_quantity_config,
		py::arg("config_key"), py::arg("value"),
		"Set a measured quantity value to the given config key.\n\n"
		"Parameters\n"
		"----------\n"
		"config_key : ConfigKey\n"
		"    The `ConfigKey` to set.\n"
		"value : Tuple[Quantity, Set[QuantityFlag]]\n"
		"    The measured quantity value to set.");
	py_configurable.def("get_bool_config",
		[](const sv::devices::Configurable &self, sv::devices::ConfigKey config_key) {
			bool value;
			self.get_config(config_key, value);
			return value;
		},
		py::arg("config_key"),
		"Return a boolean value from the given config key.\n\n"
		"Parameters\n"
		"----------\n"
		"config_key : ConfigKey\n"
		"    The `ConfigKey` to get.\n\n"
		"Returns\n"
		"-------\n"
		"bool\n"
		"    The bool value of the config key.");
	py_configurable.def("get_int_config",
		[](const sv::devices::Configurable &self, sv::devices::ConfigKey config_key) {
			int32_t value;
			self.get_config(config_key, value);
			return value;
		},
		py::arg("config_key"),
		"Return an integer value from the given config key.\n\n"
		"Parameters\n"
		"----------\n"
		"config_key : ConfigKey\n"
		"    The `ConfigKey` to get.\n\n"
		"Returns\n"
		"-------\n"
		"int\n"
		"    The int value of the config key.");
	py_configurable.def("get_uint_config",
		[](const sv::devices::Configurable &self, sv::devices::ConfigKey config_key) {
			uint64_t value;
			self.get_config(config_key, value);
			return value;
		},
		py::arg("config_key"),
		"Return an unsigned integer value from the given config key.\n\n"
		"Parameters\n"
		"----------\n"
		"config_key : ConfigKey\n"
		"    The `ConfigKey` to get.\n\n"
		"Returns\n"
		"-------\n"
		"int\n"
		"    The (unsigned) int value of the config key.");
	py_configurable.def("get_double_config",
		[](const sv::devices::Configurable &self, sv::devices::ConfigKey config_key) {
			double value;
			self.get_config(config_key, value);
			return value;
		},
		py::arg("config_key"),
		"Return a double value from the given config key.\n\n"
		"Parameters\n"
		"----------\n"
		"config_key : ConfigKey\n"
		"    The `ConfigKey` to get.\n\n"
		"Returns\n"
		"-------\n"
		"float\n"
		"    The float value of the config key.");
	py_configurable.def("get_string_config",
		[](const sv::devices::Configurable &self, sv::devices::ConfigKey config_key) {
			std::string value;
			self.get_config(config_key, value);
			return value;
		},
		py::arg("config_key"),
		"Return a string value from the given config key.\n\n"
		"Parameters\n"
		"----------\n"
		"config_key : ConfigKey\n"
		"    The `ConfigKey` to get.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The string value of the config key.");
	py_configurable.def("get_measured_quantity_config",
		[](const sv::devices::Configurable &self, sv::devices::ConfigKey config_key) {
			sv::data::measured_quantity_t value;
			self.get_measured_quantity_config(config_key, value);
			return value;
		},
		py::arg("config_key"),
		"Return a measured quantity value from the given config key.\n\n"
		"Parameters\n"
		"----------\n"
		"config_key : ConfigKey\n"
		"    The `ConfigKey` to get.\n\n"
		"Returns\n"
		"-------\n"
		"Tuple[Quantity, Set[QuantityFlag]]\n"
		"    The measured quantity value of the config key.");
	py_configurable.def("getable_configs", &sv::devices::Configurable::getable_configs,
		"Return all getable config keys.\n\n"
		"Returns\n"
		"-------\n"
		"List[ConfigKey]\n"
		"    All getable config keys.");
	py_configurable.def("setable_configs", &sv::devices::Configurable::setable_configs,
		"Return all setable config keys.\n\n"
		"Returns\n"
		"-------\n"
		"List[ConfigKey]\n"
		"    All setable config keys.");
	py_configurable.def("listable_configs", &sv::devices::Configurable::listable_configs,
		"Return all listable config keys.\n\n"
		"Returns\n"
		"-------\n"
		"List[ConfigKey]\n"
		"    All listable config keys.");
}

void init_UI(py::module &module)
{
	py::class_<sv::python::UiProxy> py_ui_proxy(module, "UiProxy");
	py_ui_proxy.doc() = "Helper class for accessing the UI.";
	py_ui_proxy.def("add_device_tab", &sv::python::UiProxy::ui_add_device_tab,
		py::arg("device"),
		"Add a device tab with standard view for a device to the UI.\n\n"
		"Parameters\n"
		"----------\n"
		"device : BaseDevice\n"
		"    The device object.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The id of the new tab or empty if the tab couldn't be added.");
	py_ui_proxy.def("add_data_view", &sv::python::UiProxy::ui_add_data_view,
		py::arg("tab_id"), py::arg("area"), py::arg("signal"),
		"Add a data view for a signal to the given tab.\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"area : DockArea\n"
		"    Where to put the new view.\n"
		"signal : AnalogTimeSignal\n"
		"    The signal object.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The id of the new view or empty if the view couldn't be added.");
	py_ui_proxy.def("add_control_views", &sv::python::UiProxy::ui_add_control_views,
		py::arg("tab_id"), py::arg("area"), py::arg("configurable"),
		"Add one or more control views for a configurable to the given tab.\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"area : DockArea\n"
		"    Where to put the new view.\n"
		"configurable : Configurable\n"
		"    The `Configurable` object.\n\n"
		"Returns\n"
		"-------\n"
		"List[str]\n"
		"    A List with the ids of the new views or an empty list if no control views were added.");
	py_ui_proxy.def("add_time_plot_view",  &sv::python::UiProxy::ui_add_time_plot_view,
		py::arg("tab_id"), py::arg("area"),
		"Add a time plot view to the given tab. Use "
		"[`UiProxy.set_channel_to_time_plot_view()`](UiProxy.set_channel_to_time_plot_view) "
		"to set a channel to the plot view or use "
		"[`UiProxy.add_curve_to_time_plot_view()`](UiProxy.add_curve_to_time_plot_view) "
		"to set a signal to the plot view.\n"
		"When you have set a channel to the plot, new curves will be automatically "
		"created, when the channel changes (e.g. for multimeters when switching "
		"functions).\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"area : DockArea\n"
		"    Where to put the new view.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The id of the new view or empty if the view couldn't be added.");
	py_ui_proxy.def("add_xy_plot_view", &sv::python::UiProxy::ui_add_xy_plot_view,
		py::arg("tab_id"), py::arg("area"),
		"Add a x/y plot view for two signals to the given tab. Use "
		"[`UiProxy.add_curve_to_xy_plot_view()`](UiProxy.add_curve_to_xy_plot_view) "
		"to add a new curve (a set of two signals) to the plot view.\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"area : DockArea\n"
		"    Where to put the new view.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The id of the new view or empty if the view couldn't be added.");
	py_ui_proxy.def("add_power_panel_view", &sv::python::UiProxy::ui_add_power_panel_view,
		py::arg("tab_id"), py::arg("area"), py::arg("voltage_signal"),
		py::arg("current_signal"),
		"Add a power panel view for a voltage and a current signal to the given tab.\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"area : DockArea\n"
		"    Where to put the new view.\n"
		"voltage_signal : AnalogTimeSignal\n"
		"    The voltage signal object.\n"
		"current_signal : AnalogTimeSignal\n"
		"    The current signal object.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The id of the new view or empty if the view couldn't be added.");
	py_ui_proxy.def("add_value_panel_view",
		(std::string (sv::python::UiProxy::*) (const std::string &, Qt::DockWidgetArea, shared_ptr<sv::channels::BaseChannel>))
			&sv::python::UiProxy::ui_add_value_panel_view,
		py::arg("tab_id"), py::arg("area"), py::arg("channel"),
		"Add a value panel view for a channel to the given tab.\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"area : DockArea\n"
		"    Where to put the new view.\n"
		"channel : BaseChannel\n"
		"    The channel object.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The id of the new view or empty if the view couldn't be added.");
	py_ui_proxy.def("add_value_panel_view",
		(std::string (sv::python::UiProxy::*) (const std::string &, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>))
			&sv::python::UiProxy::ui_add_value_panel_view,
		py::arg("tab_id"), py::arg("area"), py::arg("signal"),
		"Add a value panel view for a signal to the given tab.\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"area : DockArea\n"
		"    Where to put the new view.\n"
		"signal : AnalogTimeSignal\n"
		"    The signal object.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The id of the new view or empty if the view couldn't be added.");

	py_ui_proxy.def("add_signal_to_data_view", &sv::python::UiProxy::ui_add_signal_to_data_view,
		py::arg("tab_id"), py::arg("view_id"), py::arg("signal"),
		"Add a signal to the given data view.\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"view_id : str\n"
		"    The id of the data view.\n"
		"signal : AnalogTimeSignal\n"
		"    The signal object.");
	py_ui_proxy.def("set_channel_to_time_plot_view", &sv::python::UiProxy::ui_set_channel_to_time_plot_view,
		py::arg("tab_id"), py::arg("view_id"), py::arg("channel"),
		"Set a channel to the given time plot view. New curves will be "
		"automatically created, when the channel changes (e.g. for multimeters "
		"when switching functions).\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"view_id : str\n"
		"    The id of the time plot view.\n"
		"channel : BaseChannel\n"
		"    The channel object.");
	py_ui_proxy.def("add_curve_to_time_plot_view", &sv::python::UiProxy::ui_add_curve_to_time_plot_view,
		py::arg("tab_id"), py::arg("view_id"), py::arg("signal"),
		"Add a signal to the given time plot view.\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"view_id : str\n"
		"    The id of the time plot view.\n"
		"signal : AnalogTimeSignal\n"
		"    The signal object.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The id of the new curve or empty if the curve couldn't be added.");
	py_ui_proxy.def("add_curve_to_xy_plot_view", &sv::python::UiProxy::ui_add_curve_to_xy_plot_view,
		py::arg("tab_id"), py::arg("view_id"), py::arg("x_signal"),
		py::arg("y_signal"),
		"Add x/y signals to the given x/y plot view.\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"view_id : str\n"
		"    The id of the x/y plot view.\n"
		"x_signal : AnalogTimeSignal\n"
		"    The x signal object.\n"
		"y_signal : AnalogTimeSignal\n"
		"    The y signal object.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The id of the new curve or empty if the curve couldn't be added.");
	py_ui_proxy.def("set_curve_name", &sv::python::UiProxy::ui_set_curve_name,
		py::arg("tab_id"), py::arg("view_id"), py::arg("curve_id"),
		py::arg("name"),
		"Set the name of the given curve.\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"view_id : str\n"
		"    The id of the plot view.\n"
		"curve_id : str\n"
		"    The id of the curve.\n"
		"name : str\n"
		"    The name for the curve.");
	py_ui_proxy.def("set_curve_color", &sv::python::UiProxy::ui_set_curve_color,
		py::arg("tab_id"), py::arg("view_id"), py::arg("curve_id"),
		py::arg("color"),
		"Set the color of the given curve.\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"view_id : str\n"
		"    The id of the plot view.\n"
		"curve_id : str\n"
		"    The id of the curve.\n"
		"color : Tuple[int, int, int]\n"
		"    The color for the curve as a Tuple with the RGB values.");
	py_ui_proxy.def("show_message_box", &sv::python::UiProxy::ui_show_message_box,
		py::arg("title"), py::arg("text"),
		"Show a (info) message box with the given window title and text. "
		"Returns `True` when the Ok button was pressed.\n\n"
		"Parameters\n"
		"----------\n"
		"title : str\n"
		"    The window title of the message box.\n"
		"text : str\n"
		"    The text to display in the message box.\n\n"
		"Returns\n"
		"-------\n"
		"bool\n"
		"    `True` when the Ok button was pressed, else `False`.");
	py_ui_proxy.def("show_string_input_dialog", &sv::python::UiProxy::ui_show_string_input_dialog,
		py::arg("title"), py::arg("label"), py::arg("value") = "",
		"Show a dialog window to get a string value from the user. It returns "
		"the entered string value or `None` if the Cancel button was pressed.\n\n"
		"Parameters\n"
		"----------\n"
		"title : str\n"
		"    The window title of the input dialog.\n"
		"label : str\n"
		"    The label to display in the input dialog.\n"
		"value : str\n"
		"    The default value of the string.\n\n"
		"Returns\n"
		"-------\n"
		"str or None\n"
		"    The user entered string value or `None` when the Cancel button was pressed.");
	py_ui_proxy.def("show_double_input_dialog", &sv::python::UiProxy::ui_show_double_input_dialog,
		py::arg("title"), py::arg("label"),
		py::arg("value") = 0.0, py::arg("decimals") = 1, py::arg("step") = 0.1,
		py::arg("min") = std::numeric_limits<double>::min(),
		py::arg("max") = std::numeric_limits<double>::max(),
		"Show a dialog window to get a double value from the user. It returns "
		"the entered double value or `None` if the Cancel button was pressed.\n\n"
		"Only has effect if the used Qt version is equal or greater than 5.10!\n\n"
		"Parameters\n"
		"----------\n"
		"title : str\n"
		"    The window title of the input dialog.\n"
		"label : str\n"
		"    The label to display in the input dialog.\n"
		"value : double\n"
		"    The default value of the double.\n"
		"decimals : int\n"
		"    The maximum number of decimal places the number may have. Default is 1.\n"
		"step : double\n"
		"    The amount by which the value can be incremented or decremented by the user."
		"    Default is 0.1.\n"
		"    Only has effect for Qt versions >= 5.10!\n"
		"min : double\n"
		"    The minimum value the user may choose.\n"
		"max : double\n"
		"    The maximum value the user may choose.\n\n"
		"Returns\n"
		"-------\n"
		"double or None\n"
		"    The user entered double value or `None` when the Cancel button was pressed.");
	py_ui_proxy.def("show_int_input_dialog", &sv::python::UiProxy::ui_show_int_input_dialog,
		py::arg("title"), py::arg("label"),
		py::arg("value") = 0, py::arg("step") = 1,
		py::arg("min") = std::numeric_limits<int>::min(),
		py::arg("max") = std::numeric_limits<int>::max(),
		"Show a dialog window to get an integer value from the user. It returns "
		"the entered integer value or `None` if the Cancel button was pressed.\n\n"
		"Parameters\n"
		"----------\n"
		"title : str\n"
		"    The window title of the input dialog.\n"
		"label : str\n"
		"    The label to display in the input dialog.\n"
		"value : int\n"
		"    The default value of the integer.\n"
		"step : int\n"
		"    The amount by which the value can be incremented or decremented by the user. Default is 1.\n"
		"min : int\n"
		"    The minimum value the user may choose.\n"
		"max : int\n"
		"    The maximum value the user may choose.\n\n"
		"Returns\n"
		"-------\n"
		"int or None\n"
		"    The user entered integer value or `None` when the Cancel button was pressed.");
}

void init_StreamBuf(py::module &module)
{
	py::class_<sv::python::PyStreamBuf> py_stream_buf(module, "PyStreamBuf");
	py_stream_buf.doc() = "Redirect all Python output to a SmuView console. This class is for internal SmuView use only!";
	py_stream_buf.def(py::init<const std::string &, const std::string &>());
	py_stream_buf.def("close", &sv::python::PyStreamBuf::py_close,
		"Flush and close this stream.");
	py_stream_buf.def("fileno", &sv::python::PyStreamBuf::py_fileno,
		"Raises an `OSError`, because `PyStreamBuf` doesn't use a file descriptor.");
	py_stream_buf.def("flush", &sv::python::PyStreamBuf::py_flush,
		"Flush the write buffers of the stream.");
	py_stream_buf.def("isatty", &sv::python::PyStreamBuf::py_isatty,
		"Always returns `False`.");
	py_stream_buf.def("readable", &sv::python::PyStreamBuf::py_readable,
		"Always returns `False`.");
	py_stream_buf.def("readlines", &sv::python::PyStreamBuf::py_readlines,
		py::arg("hint"),
		"Raises an `OSError`, because `PyStreamBuf` is write only.");
	py_stream_buf.def("seekable", &sv::python::PyStreamBuf::py_seekable,
		"Always returns `False`. `PyStreamBuf` is not seekable atm.");
	py_stream_buf.def("truncate", &sv::python::PyStreamBuf::py_truncate,
		py::arg("size"),
		"Raises an `OSError`, because `PyStreamBuf` is not seekable.");
	py_stream_buf.def("writable", &sv::python::PyStreamBuf::py_writable,
		"Always return `True`.");
	py_stream_buf.def("writelines", &sv::python::PyStreamBuf::py_writelines,
		py::arg("lines"),
		"Write a list of lines to the stream.");
	py_stream_buf.def("__del__", &sv::python::PyStreamBuf::py_del,
		"Prepare for object destruction.");
	py_stream_buf.def("read", &sv::python::PyStreamBuf::py_read,
		py::arg("size"),
		"Raises an `OSError`, because `PyStreamBuf` is write only.");
	py_stream_buf.def("readline", &sv::python::PyStreamBuf::py_readline,
		py::arg("size"),
		"Raises an `OSError`, because `PyStreamBuf` is write only.");
	py_stream_buf.def("seek", &sv::python::PyStreamBuf::py_seek,
		py::arg("offset"), py::arg("whence"),
		"Raises an `OSError`, because `PyStreamBuf` is not seekable.");
	py_stream_buf.def("tell", &sv::python::PyStreamBuf::py_tell,
		"Raises an `OSError`, because `PyStreamBuf` is not seekable.");
	py_stream_buf.def("write", &sv::python::PyStreamBuf::py_write,
		py::arg("s"),
		"Write the string `s` to the stream and return the number of characters written.");
	py_stream_buf.def_readonly("closed", &sv::python::PyStreamBuf::py_closed,
		"`True` if the stream is closed.");
	py_stream_buf.def_readonly("encoding", &sv::python::PyStreamBuf::py_encoding,
		"The name of the encoding that is used.");
	py_stream_buf.def_readonly("errors", &sv::python::PyStreamBuf::py_errors,
		"The error setting of the decoder or encoder.");
}
