/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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
#include "config.h"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/hardwarechannel.hpp"
#include "src/channels/userchannel.hpp"
#include "src/data/analogbasesignal.hpp"
#include "src/data/analogsamplesignal.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/devices/userdevice.hpp"
#include "src/python/pystreambuf.hpp"
#include "src/python/uiproxy.hpp"

using std::set;

using namespace pybind11::literals; // for the ""_a
namespace py = pybind11;

/*
 * NOTE: The documentation for the smuview module is optimized for pdoc3!
 */

PYBIND11_EMBEDDED_MODULE(smuview, m) {
	// Set options for proper docstring.
	py::options options;
	options.enable_function_signatures();
	options.disable_enum_members_docstring();
	options.enable_enum_pdoc();

	m.doc() = "The SmuView 0.0.4 Python bindings.\n\n" // TODO: Use SV_VERSION_STRING?
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
		"Here is a short example that connects the HP 3378A DMM via GPIB, reads "
		"a sample and creates the default tab for the device:\n"
		"```\n"
		"import smuview\n"
		"import time\n\n"
		"# Connect device.\n"
		"dmm_dev = Session.connect_device(\"hp-3478a:conn=libgpib/hp3478a\")[0]\n"
		"# Sleep 1s to give the devices the chance to create signals\n"
		"time.sleep(1)\n"
		"# Get last sample from channel P1\n"
		"sample = dmm_dev.channels()[\"P1\"].actual_signal().get_last_sample(True)\n"
		"print(sample)\n\n"
		"# Add default tab for the DMM device.\n"
		"UiProxy.add_device_tab(dmm_dev)\n"
		"```\n\n"
		"For more example scripts, please have a look into the `smuscript` folder.";

	// NOTE: The order of initialization is very important! Otherwise types
	//       could be unknown when pybind11 is generating the function
	//       signatures.
	init_Enums(m);
	init_Signal(m);
	init_Channel(m);
	init_Configurable(m);
	init_Device(m);
	init_Session(m);
	init_UI(m);
	init_StreamBuf(m);
}

void init_Session(py::module &m)
{
	py::class_<sv::Session> py_session(m, "Session");
	py_session.doc() = "The SmuView `Session` class for accessing the actual state of the application.";
	py_session.def("devices", &sv::Session::device_map,
		"Return all connected devices.\n\n"
		"Returns\n"
		"-------\n"
		"Dict[str, BaseDevice]\n"
		"    A Dict where the key is the device id and the value is the device object.");
	py_session.def("connect_device", &sv::Session::connect_device,
		py::arg("conn_str"),
		"Connect a new device. For some devices (like DMMs) you may want to "
		"wait a fixed time, until the first sample has arrived and an `AnalogSignal` "
		"object has been created. Example:\n"
		"```\n"
		"import smuview\n"
		"import time\n\n"
		"# Connect device.\n"
		"dmm_dev = Session.connect_device(\"hp-3478a:conn=libgpib/hp3478a\")[0]\n"
		"# Sleep 1s to give the devices the chance to create signals\n"
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
}

void init_Device(py::module &m)
{
	py::class_<sv::devices::BaseDevice, std::shared_ptr<sv::devices::BaseDevice>> py_base_device(m, "BaseDevice");
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

	py::class_<sv::devices::HardwareDevice, std::shared_ptr<sv::devices::HardwareDevice>> py_hardware_device(m, "HardwareDevice", py_base_device);
	py_hardware_device.doc() = "An actual hardware device.";

	py::class_<sv::devices::UserDevice, std::shared_ptr<sv::devices::UserDevice>> py_user_device(m, "UserDevice", py_base_device);
	py_user_device.doc() = "An user generated (virtual) device for storing custom data and showing a custom tab.";
}

void init_Channel(py::module &m)
{
	py::class_<sv::channels::BaseChannel, std::shared_ptr<sv::channels::BaseChannel>> py_base_channel(m, "BaseChannel");
	py_base_channel.doc() = "The base class for all channel types.";
	py_base_channel.def("name", &sv::channels::BaseChannel::name,
		"Return the name of the channel.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The name of the channel.");
	py_base_channel.def("add_signal",
		(shared_ptr<sv::data::BaseSignal> (sv::channels::BaseChannel::*)
		(sv::data::Quantity, set<sv::data::QuantityFlag>, sv::data::Unit))
			&sv::channels::BaseChannel::add_signal,
		py::arg("quantity"), py::arg("quantity_flags"), py::arg("unit"),
		"Add a new signal to the channel.\n\n"
		"Parameters\n"
		"----------\n"
		"quantity : Quantity\n"
		"    The `Quantity` of the new signal.\n"
		"quantity_flags : Set[QuantityFlag]\n"
		"    The `QuantityFlag`s of the new signal.\n"
		"unit : Unit\n"
		"    The `Unit` of the new signal.\n\n"
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

	py::class_<sv::channels::HardwareChannel, std::shared_ptr<sv::channels::HardwareChannel>> py_hardware_channel(m, "HardwareChannel", py_base_channel);
	py_hardware_channel.doc() = "An actual hardware channel";

	py::class_<sv::channels::UserChannel, std::shared_ptr<sv::channels::UserChannel>> py_user_channel(m, "UserChannel", py_base_channel);
	py_user_channel.doc() = "An user generated channel for storing custom data.";
	py_user_channel.def("push_sample", &sv::channels::UserChannel::push_sample,
		py::arg("sample"), py::arg("timestamp"), py::arg("quantity"),
		py::arg("quantity_flags"), py::arg("unit"), py::arg("digits"),
		py::arg("decimal_places"),
		"Push a single sample to the channel.\n\n"
		"Parameters\n"
		"----------\n"
		"sample : float\n"
		"    The sample value.\n"
		"timestamp : float\n"
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

void init_Signal(py::module &m)
{
	/*
	 * TODO:
	 *  - get_value_at_timestamp(): reference parameter &value
	 */

	py::class_<sv::data::BaseSignal, std::shared_ptr<sv::data::BaseSignal>> py_base_signal(m, "BaseSignal");
	py_base_signal.doc() = "The base class for all signal types.";
	py_base_signal.def("name", &sv::data::BaseSignal::name,
		"Return the name of the signal.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The name of the signal.");
	py_base_signal.def("sample_count", &sv::data::BaseSignal::sample_count,
		"Return the number of samples of the signal.\n\n"
		"Returns\n"
		"-------\n"
		"int\n"
		"    The number of samples.");

	py::class_<sv::data::AnalogTimeSignal, std::shared_ptr<sv::data::AnalogTimeSignal>> py_analog_time_signal(m, "AnalogTimeSignal", py_base_signal);
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
		"Tuple[float, float]\n"
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
		"Tuple[float, float]\n"
		"    The sample with 1. timestamp in milliseconds and 2. the sample value.");
	py_analog_time_signal.def("push_sample", &sv::data::AnalogTimeSignal::push_sample,
		py::arg("sample"), py::arg("timestamp"), py::arg("unit_size"),
		py::arg("digits"), py::arg("decimal_places"),
		"Push a new sample to the signal.\n\n"
		"Parameters\n"
		"----------\n"
		"sample : float or double\n"
		"    The sample value.\n"
		"timestamp : float\n"
		"    The absolute timestamp in milliseconds.\n"
		"unit_size : int\n"
		"    The size of the floating point data type (float=4, double=8) for the `sample` argument.\n"
		"digits : int\n"
		"    The total number of digits.\n"
		"decimal_places : int\n"
		"    The number of decimal places.");

	py::class_<sv::data::AnalogSampleSignal, std::shared_ptr<sv::data::AnalogSampleSignal>> py_analog_sample_signal(m, "AnalogSampleSignal", py_base_signal);
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
		"Tuple[int, float]\n"
		"    The sample with 1. the key and 2. the sample value.");
	py_analog_sample_signal.def("push_sample", &sv::data::AnalogSampleSignal::push_sample,
		py::arg("sample"), py::arg("pos"), py::arg("unit_size"),
		py::arg("digits"), py::arg("decimal_places"),
		"Push a new sample to the signal.\n\n"
		"Parameters\n"
		"----------\n"
		"sample : float or double\n"
		"    The sample value.\n"
		"pos : int\n"
		"    The key (position) of the new sample.\n"
		"unit_size : int\n"
		"    The size of the floating point data type (float=4, double=8) for the `sample` argument.\n"
		"digits : int\n"
		"    The total number of digits.\n"
		"decimal_places : int\n"
		"    The number of decimal places.");
}

void init_Configurable(py::module &m)
{
	/*
	 * TODO:
	 *  - list
	 */

	py::class_<sv::devices::Configurable, std::shared_ptr<sv::devices::Configurable>> py_configurable(m, "Configurable");
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
	py_configurable.def("get_config", &sv::devices::Configurable::get_config<bool>,
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
	py_configurable.def("get_config", &sv::devices::Configurable::get_config<int32_t>,
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
	py_configurable.def("get_config", &sv::devices::Configurable::get_config<uint64_t>,
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
	py_configurable.def("get_config", &sv::devices::Configurable::get_config<double>,
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
	py_configurable.def("get_config", &sv::devices::Configurable::get_config<std::string>,
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
}

void init_UI(py::module &m)
{
	/*
	 * TODO:
	 *  - Change plot curve color
	 */

	py::class_<sv::python::UiProxy> py_ui_helper(m, "UiProxy");
	py_ui_helper.doc() = "Helper class for accessing the UI.";
	py_ui_helper.def("add_device_tab", &sv::python::UiProxy::ui_add_device_tab,
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
	py_ui_helper.def("add_data_view", &sv::python::UiProxy::ui_add_data_view,
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
	py_ui_helper.def("add_control_view", &sv::python::UiProxy::ui_add_control_view,
		py::arg("tab_id"), py::arg("area"), py::arg("configurable"),
		"Add a control view for a configurable to the given tab.\n\n"
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
		"str\n"
		"    The id of the new view or empty if the view couldn't be added.");
	py_ui_helper.def("add_plot_view",
		(std::string (sv::python::UiProxy::*) (std::string, Qt::DockWidgetArea, shared_ptr<sv::channels::BaseChannel>))
			&sv::python::UiProxy::ui_add_plot_view,
		py::arg("tab_id"), py::arg("area"), py::arg("channel"),
		"Add a time plot view for a channel to the given tab.\n\n"
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
	py_ui_helper.def("add_plot_view",
		(std::string (sv::python::UiProxy::*) (std::string, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>))
			&sv::python::UiProxy::ui_add_plot_view,
		py::arg("tab_id"), py::arg("area"), py::arg("signal"),
		"Add a time plot view for a signal to the given tab.\n\n"
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
	py_ui_helper.def("add_plot_view",
		(std::string (sv::python::UiProxy::*) (std::string, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>, shared_ptr<sv::data::AnalogTimeSignal>))
			&sv::python::UiProxy::ui_add_plot_view,
		py::arg("tab_id"), py::arg("area"), py::arg("x_signal"),
		py::arg("y_signal"),
		"Add a x/y plot view for two signals to the given tab.\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"area : DockArea\n"
		"    Where to put the new view.\n"
		"x_signal : AnalogTimeSignal\n"
		"    The x signal object.\n"
		"y_signal : AnalogTimeSignal\n"
		"    The y signal object.\n\n"
		"Returns\n"
		"-------\n"
		"str\n"
		"    The id of the new view or empty if the view couldn't be added.");
	py_ui_helper.def("add_power_panel_view", &sv::python::UiProxy::ui_add_power_panel_view,
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
	py_ui_helper.def("add_value_panel_view",
		(std::string (sv::python::UiProxy::*) (std::string, Qt::DockWidgetArea, shared_ptr<sv::channels::BaseChannel>))
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
	py_ui_helper.def("add_value_panel_view",
		(std::string (sv::python::UiProxy::*) (std::string, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>))
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

	py_ui_helper.def("add_signal_to_data_view", &sv::python::UiProxy::ui_add_signal_to_data_view,
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
	py_ui_helper.def("add_signal_to_plot_view", &sv::python::UiProxy::ui_add_signal_to_plot_view,
		py::arg("tab_id"), py::arg("view_id"), py::arg("signal"),
		"Add a signal to the given plot view. If the plot is a x/y plot, the signal will be added as a new y signal.\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"view_id : str\n"
		"    The id of the plot view.\n"
		"signal : AnalogTimeSignal\n"
		"    The signal object.");
	py_ui_helper.def("add_signals_to_xy_plot_view", &sv::python::UiProxy::ui_add_signals_to_xy_plot_view,
		py::arg("tab_id"), py::arg("view_id"), py::arg("x_signal"),
		py::arg("y_signal"),
		"Add x/y signals to the given x/y plot view.\n\n"
		"Parameters\n"
		"----------\n"
		"tab_id : str\n"
		"    The id of the tab.\n"
		"view_id : str\n"
		"    The id of the plot view.\n"
		"x_signal : AnalogTimeSignal\n"
		"    The x signal object.\n"
		"y_signal : AnalogTimeSignal\n"
		"    The y signal object.");

	py_ui_helper.def("show_message_box", &sv::python::UiProxy::ui_show_message_box,
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
	py_ui_helper.def("show_string_input_dialog", &sv::python::UiProxy::ui_show_string_input_dialog,
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
	py_ui_helper.def("show_double_input_dialog", &sv::python::UiProxy::ui_show_double_input_dialog,
		py::arg("title"), py::arg("label"),
		py::arg("value") = 0.0, py::arg("decimals") = 1, py::arg("step") = 0.1,
		py::arg("min") = std::numeric_limits<double>::min(),
		py::arg("max") = std::numeric_limits<double>::max(),
		"Show a dialog window to get a float value from the user. It returns "
		"the entered float value or `None` if the Cancel button was pressed.\n\n"
		"Parameters\n"
		"----------\n"
		"title : str\n"
		"    The window title of the input dialog.\n"
		"label : str\n"
		"    The label to display in the input dialog.\n"
		"value : float\n"
		"    The default value of the float.\n"
		"decimals : int\n"
		"    The maximum number of decimal places the number may have. Default is 1.\n"
		"step : float\n"
		"    The amount by which the value can be incremented or decremented by the user."
		"    Default is 0.1.\n"
		"min : float\n"
		"    The minimum value the user may choose.\n"
		"max : float\n"
		"    The maximum value the user may choose.\n\n"
		"Returns\n"
		"-------\n"
		"float or None\n"
		"    The user entered float value or `None` when the Cancel button was pressed.");
	py_ui_helper.def("show_int_input_dialog", &sv::python::UiProxy::ui_show_int_input_dialog,
		py::arg("title"), py::arg("label"),
		py::arg("value") = 0, py::arg("step") = 1,
		py::arg("min") = std::numeric_limits<int>::min(),
		py::arg("max") = std::numeric_limits<int>::max(),
		"Show a dialog window to get an integer value from the user. It returns "
		"the entered float value or `None` if the Cancel button was pressed.\n\n"
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

void init_StreamBuf(py::module &m)
{
	py::class_<sv::python::PyStreamBuf> py_stream_buf(m, "PyStreamBuf");
	py_stream_buf.doc() = "Redirect Python output to SmuView console. This class is for internal SmuView use only!";
	py_stream_buf.def(py::init<const std::string &, const std::string &>());
	py_stream_buf.def("close", &sv::python::PyStreamBuf::py_close,
		"Flush and close this stream.");
	py_stream_buf.def("fileno", &sv::python::PyStreamBuf::py_fileno,
		"Raises an OSError, because PyStreamBuf doesn't use a file descriptor.");
	py_stream_buf.def("flush", &sv::python::PyStreamBuf::py_flush,
		"Flush the write buffers of the stream.");
	py_stream_buf.def("isatty", &sv::python::PyStreamBuf::py_isatty,
		"Always return False.");
	py_stream_buf.def("readable", &sv::python::PyStreamBuf::py_readable,
		"Always return False.");
	py_stream_buf.def("readlines", &sv::python::PyStreamBuf::py_readlines,
		py::arg("hint"),
		"Raises an OSError, because PyStreamBuf is write only.");
	py_stream_buf.def("seekable", &sv::python::PyStreamBuf::py_seekable,
		"Always return False. PyStreamBuf is not seekable atm.");
	py_stream_buf.def("truncate", &sv::python::PyStreamBuf::py_truncate,
		py::arg("size"),
		"Raises an OSError, because PyStreamBuf is not seekable.");
	py_stream_buf.def("writable", &sv::python::PyStreamBuf::py_writable,
		"Always return `True`.");
	py_stream_buf.def("writelines", &sv::python::PyStreamBuf::py_writelines,
		py::arg("lines"),
		"Write a list of lines to the stream.");
	py_stream_buf.def("__del__", &sv::python::PyStreamBuf::py_del,
		"Prepare for object destruction.");
	py_stream_buf.def("read", &sv::python::PyStreamBuf::py_read,
		py::arg("size"),
		"Raises an OSError, because PyStreamBuf is write only.");
	py_stream_buf.def("readline", &sv::python::PyStreamBuf::py_readline,
		py::arg("size"),
		"Raises an OSError, because PyStreamBuf is write only.");
	py_stream_buf.def("seek", &sv::python::PyStreamBuf::py_seek,
		py::arg("offset"), py::arg("whence"),
		"Raises an OSError, because PyStreamBuf is not seekable.");
	py_stream_buf.def("tell", &sv::python::PyStreamBuf::py_tell,
		"Raises an OSError, because PyStreamBuf is not seekable.");
	py_stream_buf.def("write", &sv::python::PyStreamBuf::py_write,
		py::arg("s"),
		"Write the string s to the stream and return the number of characters written.");
	py_stream_buf.def_readonly("closed", &sv::python::PyStreamBuf::py_closed,
		"True if the stream is closed.");
	py_stream_buf.def_readonly("encoding", &sv::python::PyStreamBuf::py_encoding,
		"The name of the encoding that is used.");
	py_stream_buf.def_readonly("errors", &sv::python::PyStreamBuf::py_errors,
		"The error setting of the decoder or encoder.");
}

void init_Enums(py::module &m)
{
	py::enum_<sv::devices::ConfigKey> py_config_key(m, "ConfigKey",
		"Enum of all available config keys for controlling a device.");
	py_config_key.value("Samplerate", sv::devices::ConfigKey::Samplerate,
		"The samplerate, in Hz.");
	py_config_key.value("CaptureRatio", sv::devices::ConfigKey::CaptureRatio,
		"The pre/post-trigger capture ratio.");
	py_config_key.value("PatternMode", sv::devices::ConfigKey::PatternMode,
		"A pattern (pattern generator mode).");
	py_config_key.value("RLE", sv::devices::ConfigKey::RLE,
		"Run-length encoding (RLE).");
	py_config_key.value("TriggerSlope", sv::devices::ConfigKey::TriggerSlope,
		"The trigger slope.");
	py_config_key.value("Averaging", sv::devices::ConfigKey::Averaging,
		"Averaging.");
	py_config_key.value("AvgSamples", sv::devices::ConfigKey::AvgSamples,
		"The number of samples to be averaged over.");
	py_config_key.value("TriggerSource", sv::devices::ConfigKey::TriggerSource,
		"Trigger source.");
	py_config_key.value("HorizTriggerPos", sv::devices::ConfigKey::HorizTriggerPos,
		"Horizontal trigger position.");
	py_config_key.value("BufferSize", sv::devices::ConfigKey::BufferSize,
		"Buffer size.");
	py_config_key.value("TimeBase", sv::devices::ConfigKey::TimeBase,
		"Time base.");
	py_config_key.value("Filter", sv::devices::ConfigKey::Filter,
		"Filter.");
	py_config_key.value("VDiv", sv::devices::ConfigKey::VDiv,
		"Volts/div.");
	py_config_key.value("Coupling", sv::devices::ConfigKey::Coupling,
		"Coupling.");
	py_config_key.value("TriggerMatch", sv::devices::ConfigKey::TriggerMatch,
		"Trigger matches.");
	py_config_key.value("SampleInterval", sv::devices::ConfigKey::SampleInterval,
		"The sample interval, in ms.");
	py_config_key.value("NumHDiv", sv::devices::ConfigKey::NumHDiv,
		"Number of horizontal divisions, as related to `ConfigKey.TimeBase`.");
	py_config_key.value("NumVDiv", sv::devices::ConfigKey::NumVDiv,
		"Number of vertical divisions, as related to `ConfigKey.VDiv`.");
	py_config_key.value("SplWeightFreq", sv::devices::ConfigKey::SplWeightFreq,
		"Sound pressure level frequency weighting.");
	py_config_key.value("SplWeightTime", sv::devices::ConfigKey::SplWeightTime,
		"Sound pressure level time weighting.");
	py_config_key.value("SplMeasurementRange", sv::devices::ConfigKey::SplMeasurementRange,
		"Sound pressure level measurement range.");
	py_config_key.value("HoldMax", sv::devices::ConfigKey::HoldMax,
		"Max hold mode.");
	py_config_key.value("HoldMin", sv::devices::ConfigKey::HoldMin,
		"Min hold mode.");
	py_config_key.value("VoltageThreshold", sv::devices::ConfigKey::VoltageThreshold,
		"Logic low-high threshold range.");
	py_config_key.value("ExternalClock", sv::devices::ConfigKey::ExternalClock,
		"Using an external clock.");
	py_config_key.value("Swap", sv::devices::ConfigKey::Swap,
		"Swapping channels.");
	py_config_key.value("CenterFrequency", sv::devices::ConfigKey::CenterFrequency,
		"Center frequency.");
	py_config_key.value("NumLogicChannels", sv::devices::ConfigKey::NumLogicChannels,
		"The number of logic channels.");
	py_config_key.value("NumAnalogChannels", sv::devices::ConfigKey::NumAnalogChannels,
		"The number of analog channels.");
	py_config_key.value("Voltage", sv::devices::ConfigKey::Voltage,
		"Current voltage.");
	py_config_key.value("VoltageTarget", sv::devices::ConfigKey::VoltageTarget,
		"Maximum target voltage.");
	py_config_key.value("Current", sv::devices::ConfigKey::Current,
		"Current current.");
	py_config_key.value("CurrentLimit", sv::devices::ConfigKey::CurrentLimit,
		"Current limit.");
	py_config_key.value("Enabled", sv::devices::ConfigKey::Enabled,
		"Enabling/disabling a channel (group).");
	py_config_key.value("ChannelConfig", sv::devices::ConfigKey::ChannelConfig,
		"Channel configuration.");
	py_config_key.value("OverVoltageProtectionEnabled", sv::devices::ConfigKey::OverVoltageProtectionEnabled,
		"Enabling/disable over voltage protection (OVP) feature.");
	py_config_key.value("OverVoltageProtectionActive", sv::devices::ConfigKey::OverVoltageProtectionActive,
		"Status of over voltage protection (OVP).");
	py_config_key.value("OverVoltageProtectionThreshold", sv::devices::ConfigKey::OverVoltageProtectionThreshold,
		"Over voltage protection (OVP) threshold.");
	py_config_key.value("OverCurrentProtectionEnabled", sv::devices::ConfigKey::OverCurrentProtectionEnabled,
		"Enabling/disable  over current protection (OCP) feature.");
	py_config_key.value("OverCurrentProtectionActive", sv::devices::ConfigKey::OverCurrentProtectionActive,
		"Status of over current protection (OCP).");
	py_config_key.value("OverCurrentProtectionThreshold", sv::devices::ConfigKey::OverCurrentProtectionThreshold,
		"Over current protection (OCP) threshold.");
	py_config_key.value("OverTemperatureProtectionEnabled", sv::devices::ConfigKey::OverTemperatureProtectionEnabled,
		"Enabling/disable over temperature protection (OTP) feature.");
	py_config_key.value("OverTemperatureProtectionActive", sv::devices::ConfigKey::OverTemperatureProtectionActive,
		"Status of over temperature protection (OTP).");
	py_config_key.value("UnderVoltageConditionEnabled", sv::devices::ConfigKey::UnderVoltageConditionEnabled,
		"Enabling/disable under voltage condition (UVC) feature.");
	py_config_key.value("UnderVoltageConditionActive", sv::devices::ConfigKey::UnderVoltageConditionActive,
		"Status of under voltage condition (UVC).");
	py_config_key.value("UnderVoltageConditionThreshold", sv::devices::ConfigKey::UnderVoltageConditionThreshold,
		"Under voltage condition threshold (UVC).");
	py_config_key.value("ClockEdge", sv::devices::ConfigKey::ClockEdge,
		"Choice of clock edge for external clock (``r`` or ``f``).");
	py_config_key.value("Amplitude", sv::devices::ConfigKey::Amplitude,
		"Amplitude of a source without strictly-defined `ConfigKey.MeasuredQuantity`.");
	py_config_key.value("Regulation", sv::devices::ConfigKey::Regulation,
		"Channel regulation. ``CV``, ``CC`` or ``UR``, denoting constant voltage, constant "
		"current or unregulated. ``CC-`` denotes a power supply in current sink mode "
		"(e.g. HP 66xxB). An empty string is used when there is no regulation, e.g. "
		" the output is disabled.");
	py_config_key.value("OutputFrequency", sv::devices::ConfigKey::OutputFrequency,
		"Output frequency in Hz.");
	py_config_key.value("OutputFrequencyTarget", sv::devices::ConfigKey::OutputFrequencyTarget,
		"Output frequency target in Hz.");
	py_config_key.value("MeasuredQuantity", sv::devices::ConfigKey::MeasuredQuantity,
		"Measured quantity.");
	py_config_key.value("EquivCircuitModel", sv::devices::ConfigKey::EquivCircuitModel,
		"Equivalent circuit model.");
	py_config_key.value("TriggerLevel", sv::devices::ConfigKey::TriggerLevel,
		"Trigger level.");
	py_config_key.value("ExternalClockSource", sv::devices::ConfigKey::ExternalClockSource,
		"Which external clock source to use if the device supports multiple external clock channels.");
	py_config_key.value("Offset", sv::devices::ConfigKey::Offset,
		"Offset of a source without strictly-defined `ConfigKey.MeasuredQuantity`.");
	py_config_key.value("TriggerPattern", sv::devices::ConfigKey::TriggerPattern,
		"The pattern for the logic trigger.");
	py_config_key.value("HighResolution", sv::devices::ConfigKey::HighResolution,
		"High resolution mode.");
	py_config_key.value("PeakDetection", sv::devices::ConfigKey::PeakDetection,
		"Peak detection.");
	py_config_key.value("LogicThreshold", sv::devices::ConfigKey::LogicThreshold,
		"Logic threshold: predefined levels (``TTL``, ``ECL``, ``CMOS``, etc).");
	py_config_key.value("LogicThresholdCustom", sv::devices::ConfigKey::LogicThresholdCustom,
		"Logic threshold: custom numerical value.");
	py_config_key.value("Range", sv::devices::ConfigKey::Range,
		"The measurement range of a DMM or the output range of a power supply.");
	py_config_key.value("Digits", sv::devices::ConfigKey::Digits,
		"The number of digits (e.g. for a DMM).");
	py_config_key.value("SessionFile", sv::devices::ConfigKey::SessionFile,
		"Session filename.");
	py_config_key.value("CaptureFile", sv::devices::ConfigKey::CaptureFile,
		"The capturefile to inject.");
	py_config_key.value("CaptureUnitSize", sv::devices::ConfigKey::CaptureUnitSize,
		"The capturefile unit size.");
	py_config_key.value("PowerOff", sv::devices::ConfigKey::PowerOff,
		"Power off the device.");
	py_config_key.value("DataSource", sv::devices::ConfigKey::DataSource,
		"Data source for acquisition.");
	py_config_key.value("ProbeFactor", sv::devices::ConfigKey::ProbeFactor,
		"The probe factor.");
	py_config_key.value("ADCPowerlineCycles", sv::devices::ConfigKey::ADCPowerlineCycles,
		"Number of powerline cycles for ADC integration time.");
	py_config_key.value("DataLog", sv::devices::ConfigKey::DataLog,
		"The device has internal storage, into which data is logged.");
	py_config_key.value("DeviceMode", sv::devices::ConfigKey::DeviceMode,
		"Device mode for multi-function devices.");
	py_config_key.value("TestMode", sv::devices::ConfigKey::TestMode,
		"Self test mode.");
	py_config_key.value("Unknown", sv::devices::ConfigKey::Unknown,
		"Unknown config key.");

	py::enum_<sv::data::Quantity> py_quantity(m, "Quantity", "Enum of all available quantities.");
	py_quantity.value("Voltage", sv::data::Quantity::Voltage,
		"Voltage");
	py_quantity.value("Current", sv::data::Quantity::Current,
		"Current");
	py_quantity.value("Resistance", sv::data::Quantity::Resistance,
		"Resistance");
	py_quantity.value("Capacitance", sv::data::Quantity::Capacitance,
		"Capacitance");
	py_quantity.value("Temperature", sv::data::Quantity::Temperature,
		"Temperature");
	py_quantity.value("Frequency", sv::data::Quantity::Frequency,
		"Frequency");
	py_quantity.value("DutyCyle", sv::data::Quantity::DutyCyle,
		"DutyCyle");
	py_quantity.value("Continuity", sv::data::Quantity::Continuity,
		"Continuity");
	py_quantity.value("PulseWidth", sv::data::Quantity::PulseWidth,
		"PulseWidth");
	py_quantity.value("Conductance", sv::data::Quantity::Conductance,
		"Conductance");
	py_quantity.value("Power", sv::data::Quantity::Power,
		"Electrical power, usually in W, or dBm.");
	py_quantity.value("Work", sv::data::Quantity::Work,
		"Work, also Energy");
	py_quantity.value("ElectricCharge", sv::data::Quantity::ElectricCharge,
		"Electric charge");
	py_quantity.value("Gain", sv::data::Quantity::Gain,
		"Gain (a transistor's gain, or hFE, for example).");
	py_quantity.value("SoundPressureLevel", sv::data::Quantity::SoundPressureLevel,
		"Logarithmic representation of sound pressure relative to a reference value.");
	py_quantity.value("CarbonMonoxide", sv::data::Quantity::CarbonMonoxide,
		"Carbon monoxide");
	py_quantity.value("RelativeHumidity", sv::data::Quantity::RelativeHumidity,
		"Relative humidity");
	py_quantity.value("Time", sv::data::Quantity::Time,
		"Time");
	py_quantity.value("WindSpeed", sv::data::Quantity::WindSpeed,
		"Wind speed");
	py_quantity.value("Pressure", sv::data::Quantity::Pressure,
		"Pressure");
	py_quantity.value("ParallelInductance", sv::data::Quantity::ParallelInductance,
		"Parallel inductance");
	py_quantity.value("ParallelCapacitance", sv::data::Quantity::ParallelCapacitance,
		"Parallel capacitance");
	py_quantity.value("ParallelResistance", sv::data::Quantity::ParallelResistance,
		"Parallel resistance");
	py_quantity.value("SeriesInductance", sv::data::Quantity::SeriesInductance,
		"Series inductance");
	py_quantity.value("SeriesCapacitance", sv::data::Quantity::SeriesCapacitance,
		"Series capacitance");
	py_quantity.value("SeriesResistance", sv::data::Quantity::SeriesResistance,
		"Series resistance");
	py_quantity.value("DissipationFactor", sv::data::Quantity::DissipationFactor,
		"Dissipation factor");
	py_quantity.value("QualityFactor", sv::data::Quantity::QualityFactor,
		"Quality factor");
	py_quantity.value("PhaseAngle", sv::data::Quantity::PhaseAngle,
		"Phase angle");
	py_quantity.value("Difference", sv::data::Quantity::Difference,
		"Difference from reference value.");
	py_quantity.value("Count", sv::data::Quantity::Count,
		"Count");
	py_quantity.value("PowerFactor", sv::data::Quantity::PowerFactor,
		"Power factor");
	py_quantity.value("ApparentPower", sv::data::Quantity::ApparentPower,
		"Apparent power");
	py_quantity.value("Mass", sv::data::Quantity::Mass,
		"Mass");
	py_quantity.value("HarmonicRatio", sv::data::Quantity::HarmonicRatio,
		"Harmonic ratio");
	py_quantity.value("Unknown", sv::data::Quantity::Unknown,
		"Unknown");

	py::enum_<sv::data::QuantityFlag> py_quantity_flag(m, "QuantityFlag",
		"Enum of all available quantity flags.");
	py_quantity_flag.value("AC", sv::data::QuantityFlag::AC,
		"Alternating current.");
	py_quantity_flag.value("DC", sv::data::QuantityFlag::DC,
		"Direct current.");
	py_quantity_flag.value("RMS", sv::data::QuantityFlag::RMS,
		"Root mean square (RMS).");
	py_quantity_flag.value("Diode", sv::data::QuantityFlag::Diode,
		"Value is voltage drop across a diode, or NAN.");
	py_quantity_flag.value("Hold", sv::data::QuantityFlag::Hold,
		"Device is in hold mode (repeating the last measurement).");
	py_quantity_flag.value("Max", sv::data::QuantityFlag::Max,
		"Device is in max mode, only updating upon a new max value.");
	py_quantity_flag.value("Min", sv::data::QuantityFlag::Min,
		"Device is in min mode, only updating upon a new min value.");
	py_quantity_flag.value("Autorange", sv::data::QuantityFlag::Autorange,
		"Device is in autoranging mode.");
	py_quantity_flag.value("Relative", sv::data::QuantityFlag::Relative,
		"Device is in relative mode.");
	py_quantity_flag.value("SplFreqWeightA", sv::data::QuantityFlag::SplFreqWeightA,
		"Sound pressure level is A-weighted in the frequency domain, according "
		"to IEC 61672:2003.");
	py_quantity_flag.value("SplFreqWeightC", sv::data::QuantityFlag::SplFreqWeightC,
		"Sound pressure level is C-weighted in the frequency domain, according "
		"to IEC 61672:2003.");
	py_quantity_flag.value("SplFreqWeightZ", sv::data::QuantityFlag::SplFreqWeightZ,
		"Sound pressure level is Z-weighted.");
	py_quantity_flag.value("SplFreqWeightFlat", sv::data::QuantityFlag::SplFreqWeightFlat,
		"Sound pressure level is not weighted in the frequency domain, albeit "
		"without standards-defined low and high frequency limits.");
	py_quantity_flag.value("SplTimeWeightS", sv::data::QuantityFlag::SplTimeWeightS,
		"Sound pressure level measurement is S-weighted (1s) in the time domain.");
	py_quantity_flag.value("SplTimeWeightF", sv::data::QuantityFlag::SplTimeWeightF,
		"Sound pressure level measurement is F-weighted (125ms) in the time domain.");
	py_quantity_flag.value("SplLAT", sv::data::QuantityFlag::SplLAT,
		"Sound pressure level is time-averaged (LAT), also known as Equivalent "
		"Continuous A-weighted Sound Level (LEQ).");
	py_quantity_flag.value("SplPctOverAlarm", sv::data::QuantityFlag::SplPctOverAlarm,
		"Sound pressure level represented as a percentage of measurements that "
		"were over a preset alarm level.");
	py_quantity_flag.value("Duration", sv::data::QuantityFlag::Duration,
		"Time is duration (as opposed to epoch, ...).");
	py_quantity_flag.value("Avg", sv::data::QuantityFlag::Avg,
		"Device is in average mode, averaging upon each new value.");
	py_quantity_flag.value("Reference", sv::data::QuantityFlag::Reference,
		"Reference value shown.");
	py_quantity_flag.value("Unstable", sv::data::QuantityFlag::Unstable,
		"Unstable value (hasn't settled yet).");
	py_quantity_flag.value("FourWire", sv::data::QuantityFlag::FourWire,
		"Device is in 4-wire mode.");
	py_quantity_flag.value("Unknown", sv::data::QuantityFlag::Unknown,
		"Unknown quantity flag.");

	py::enum_<sv::data::Unit> py_unit(m, "Unit", "Enum of all available units.");
	py_unit.value("Volt", sv::data::Unit::Volt,
		"Volt");
	py_unit.value("Ampere", sv::data::Unit::Ampere,
		"Ampere");
	py_unit.value("Ohm", sv::data::Unit::Ohm,
		"Ohm");
	py_unit.value("Farad", sv::data::Unit::Farad,
		"Farad");
	py_unit.value("Kelvin", sv::data::Unit::Kelvin,
		"Kelvin");
	py_unit.value("Celsius", sv::data::Unit::Celsius,
		"Celsius");
	py_unit.value("Fahrenheit", sv::data::Unit::Fahrenheit,
		"Fahrenheit");
	py_unit.value("Hertz", sv::data::Unit::Hertz,
		"Hertz");
	py_unit.value("Percentage", sv::data::Unit::Percentage,
		"Percentage");
	py_unit.value("Boolean", sv::data::Unit::Boolean,
		"Boolean");
	py_unit.value("Second", sv::data::Unit::Second,
		"Second");
	py_unit.value("Siemens", sv::data::Unit::Siemens,
		"Siemens");
	py_unit.value("DecibelMW", sv::data::Unit::DecibelMW,
		"Decibel milliWatt (dBm)");
	py_unit.value("DecibelVolt", sv::data::Unit::DecibelVolt,
		"Decibel Volt (dBV)");
	py_unit.value("Decibel", sv::data::Unit::Decibel,
		"Decibel (dB)");
	py_unit.value("Unitless", sv::data::Unit::Unitless,
		"Unitless");
	py_unit.value("DecibelSpl", sv::data::Unit::DecibelSpl,
		"Decibel sound pressure level");
	py_unit.value("Concentration", sv::data::Unit::Concentration,
		"Concentration");
	py_unit.value("RevolutionsPerMinute", sv::data::Unit::RevolutionsPerMinute,
		"Revolutions per minute (RPM)");
	py_unit.value("VoltAmpere", sv::data::Unit::VoltAmpere,
		"VoltAmpere (VA)");
	py_unit.value("Watt", sv::data::Unit::Watt,
		"Watt");
	py_unit.value("WattHour", sv::data::Unit::WattHour,
		"WattHour (Wh)");
	py_unit.value("Joule", sv::data::Unit::Joule,
		"Joule");
	py_unit.value("AmpereHour", sv::data::Unit::AmpereHour,
		"AmpereHour (Ah)");
	py_unit.value("Coulomb", sv::data::Unit::Coulomb,
		"Coulomb");
	py_unit.value("MeterPerSecond", sv::data::Unit::MeterPerSecond,
		"Meter per second (m/s)");
	py_unit.value("HectoPascal", sv::data::Unit::HectoPascal,
		"HectoPascal (hPa)");
	py_unit.value("Humidity293K", sv::data::Unit::Humidity293K,
		"Humidity at 293K");
	py_unit.value("Degree", sv::data::Unit::Degree,
		"Degree");
	py_unit.value("Henry", sv::data::Unit::Henry,
		"Henry");
	py_unit.value("Gram", sv::data::Unit::Gram,
		"Weight in gram (g).");
	py_unit.value("Carat", sv::data::Unit::Carat,
		"Weight in carat.");
	py_unit.value("Ounce", sv::data::Unit::Ounce,
		"Weight in avoirdupois ounce (oz).");
	py_unit.value("TroyOunce", sv::data::Unit::TroyOunce,
		"Weight in troy ounce (oz t).");
	py_unit.value("Pound", sv::data::Unit::Pound,
		"Weight in avoirdupois pound (lb).");
	py_unit.value("Pennyweight", sv::data::Unit::Pennyweight,
		"Weight in pennyweight.");
	py_unit.value("Grain", sv::data::Unit::Grain,
		"Weight in grain.");
	py_unit.value("Tael", sv::data::Unit::Tael,
		"Weight in tael.");
	py_unit.value("Momme", sv::data::Unit::Momme,
		"Weight in momme.");
	py_unit.value("Tola", sv::data::Unit::Tola,
		"Weight in tola.");
	py_unit.value("Piece", sv::data::Unit::Piece,
		"Piece");
	py_unit.value("Unknown", sv::data::Unit::Unknown,
		"Unknown");

	// Qt enumerations
	py::enum_<Qt::DockWidgetArea> py_dock_area(m, "DockArea",
		"Enum of all possible docking locations for a view.");
	py_dock_area.value("LeftDocktArea", Qt::DockWidgetArea::LeftDockWidgetArea,
		"Dock to the left dock area.");
	py_dock_area.value("RightDockArea", Qt::DockWidgetArea::RightDockWidgetArea,
		"Dock to the right dock area.");
	py_dock_area.value("TopDockArea", Qt::DockWidgetArea::TopDockWidgetArea,
		"Dock to the top dock area.");
	py_dock_area.value("BottomDockArea", Qt::DockWidgetArea::BottomDockWidgetArea,
		"Dock to the bottom dock area.");
	//py_dock_area.value("AllDockAreas", Qt::DockWidgetArea::AllDockWidgetAreas,
	//	"Dock to all dock area.");
	//py_dock_area.value("NoDockArea", Qt::DockWidgetArea::NoDockWidgetArea,
	//	"Dock to no dock area.");
}
