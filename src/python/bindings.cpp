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

#include <memory>
#include <string>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "bindings.hpp"
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

using namespace pybind11::literals; // for the ""_a
namespace py = pybind11;

PYBIND11_EMBEDDED_MODULE(smuview, m) {
	m.doc() = "SmuView python bindings.";

	init_Session(m);
	init_Device(m);
	init_Channel(m);
	init_Signal(m);
	init_Configurable(m);
	init_UI(m);
	init_StreamBuf(m);
	init_Enums(m);
}

void init_Session(py::module &m)
{
	py::class_<sv::Session> py_session(m, "Session");
	py_session.doc() = "The SmuView session class for accessing the actual state of the application.";
	py_session.def("devices", &sv::Session::devices, "Return all connected devices.");
	py_session.def("connect_device", &sv::Session::connect_device, "Connect a new device.");
	py_session.def("add_user_device", &sv::Session::add_user_device, "Create and connect a new user device.");
}

void init_Device(py::module &m)
{
	py::class_<sv::devices::BaseDevice, std::shared_ptr<sv::devices::BaseDevice>> py_base_device(m, "BaseDevice");
	py_base_device.doc() = "The base class for all device types.";
	py_base_device.def("name", &sv::devices::BaseDevice::name, "Return the name of the device.");
	py_base_device.def("id", &sv::devices::BaseDevice::id, "Return the unique id of the device.");
	py_base_device.def("channels", &sv::devices::BaseDevice::channel_map, "Return all channels of the device.");
	py_base_device.def("configurables", &sv::devices::BaseDevice::configurable_map, "Return all configurables of the device.");
	py_base_device.def("add_user_channel", &sv::devices::BaseDevice::add_user_channel, "Add a new user channel to the device.");

	py::class_<sv::devices::HardwareDevice, std::shared_ptr<sv::devices::HardwareDevice>> py_hardware_device(m, "HardwareDevice", py_base_device);
	py_hardware_device.doc() = "An actual hardware device.";

	py::class_<sv::devices::UserDevice, std::shared_ptr<sv::devices::UserDevice>> py_user_device(m, "UserDevice", py_base_device);
	py_user_device.doc() = "An user generated (dummy) device for storing custom data and showing a custom tab.";
}

void init_Channel(py::module &m)
{
	py::class_<sv::channels::BaseChannel, std::shared_ptr<sv::channels::BaseChannel>> py_base_channel(m, "BaseChannel");
	py_base_channel.doc() = "The base class for all channel types.";
	py_base_channel.def("name", &sv::channels::BaseChannel::name, "Return the name of the channel.");
	py_base_channel.def("add_signal",
		(shared_ptr<sv::data::BaseSignal> (sv::channels::BaseChannel::*)
		(sv::data::Quantity, set<sv::data::QuantityFlag>, sv::data::Unit))
			&sv::channels::BaseChannel::add_signal, "Add a new signal to the channel.");
	py_base_channel.def("actual_signal", &sv::channels::BaseChannel::actual_signal, "Return the actual signal of the channel.");
	py_base_channel.def("signals", &sv::channels::BaseChannel::signals, "Return all signals of the channel.");

	py::class_<sv::channels::HardwareChannel, std::shared_ptr<sv::channels::HardwareChannel>> py_hardware_channel(m, "HardwareChannel", py_base_channel);
	py_hardware_channel.doc() = "An actual hardware channel";

	py::class_<sv::channels::UserChannel, std::shared_ptr<sv::channels::UserChannel>> py_user_channel(m, "UserChannel", py_base_channel);
	py_user_channel.doc() = "An user generated channel for storing custom data.";
	py_user_channel.def("push_sample", &sv::channels::UserChannel::push_sample);
}

void init_Signal(py::module &m)
{
	py::class_<sv::data::BaseSignal, std::shared_ptr<sv::data::BaseSignal>> py_base_signal(m, "BaseSignal");
	py_base_signal.doc() = "The base class for all signal types.";
	py_base_signal.def("name", &sv::data::BaseSignal::name, "Return the name of the signal.");
	py_base_signal.def("sample_count", &sv::data::BaseSignal::sample_count, "Return the number of samples for the signal.");

	py::class_<sv::data::AnalogTimeSignal, std::shared_ptr<sv::data::AnalogTimeSignal>> py_analog_time_signal(m, "AnalogTimeSignal", py_base_signal);
	py_analog_time_signal.doc() = "A signal with time-value pairs.";
	py_analog_time_signal.def("get_sample", &sv::data::AnalogTimeSignal::get_sample, "Return the sample for the given timestamp.");
	py_analog_time_signal.def("get_last_sample", &sv::data::AnalogTimeSignal::get_last_sample, "Return the last sample of the signal.");
	py_analog_time_signal.def("push_sample", &sv::data::AnalogTimeSignal::push_sample, "Push a new sample to the signal.");

	py::class_<sv::data::AnalogSampleSignal, std::shared_ptr<sv::data::AnalogSampleSignal>> py_analog_sample_signal(m, "AnalogSampleSignal", py_base_signal);
	py_analog_sample_signal.doc() = "A signal with key-value pairs.";
	py_analog_sample_signal.def("get_sample", &sv::data::AnalogSampleSignal::get_sample, "Return the sample for the given timestamp.");
	py_analog_sample_signal.def("push_sample", &sv::data::AnalogSampleSignal::push_sample, "Push a new sample to the signal.");
}

void init_Configurable(py::module &m)
{
	/*
	 * TODO:
	 *  - list
	 */

	py::class_<sv::devices::Configurable, std::shared_ptr<sv::devices::Configurable>> py_configurable(m, "Configurable");
	py_configurable.doc() = "A configurable for controlling a device via the config keys.";
	py_configurable.def("name", &sv::devices::Configurable::name, "Return the name of the configurable.");
	py_configurable.def("set_config", &sv::devices::Configurable::set_config<bool>, "Set a boolean value to the given config key.");
	py_configurable.def("set_config", &sv::devices::Configurable::set_config<int32_t>, "Set an integer value to the given config key.");
	py_configurable.def("set_config", &sv::devices::Configurable::set_config<uint64_t>, "Set an unsigned integer value to the given config key.");
	py_configurable.def("set_config", &sv::devices::Configurable::set_config<double>, "Set a double value to the given config key.");
	py_configurable.def("set_config", &sv::devices::Configurable::set_config<std::string>, "Set a string value to the given config key.");
	py_configurable.def("get_config", &sv::devices::Configurable::set_config<bool>, "Return a boolean value from the given config key.");
	py_configurable.def("get_config", &sv::devices::Configurable::set_config<int32_t>, "Return an integer value from the given config key.");
	py_configurable.def("get_config", &sv::devices::Configurable::set_config<uint64_t>, "Return an unsigned integer value from the given config key.");
	py_configurable.def("get_config", &sv::devices::Configurable::set_config<double>, "Return a double value from the given config key.");
	py_configurable.def("get_config", &sv::devices::Configurable::set_config<std::string>, "Return a string value from the given config key.");
}

void init_UI(py::module &m)
{
	/*
	 * TODO:
	 *  - Add x/y signal to plot
	 *  - Change plot curve color
	 *  - Add channel to plot (?)
	 *  - Display message boxes and simple input dialogs (howto return a value?)
	 */

	py::class_<sv::python::UiProxy> py_ui_helper(m, "UiProxy");
	py_ui_helper.doc() = "Helper class for accessing the UI.";
	py_ui_helper.def("add_device_tab", &sv::python::UiProxy::ui_add_device_tab, "Add a device tab to the UI.");
	py_ui_helper.def("add_data_view", &sv::python::UiProxy::ui_add_data_view, "Add a data view for a signal to the given tab.");
	py_ui_helper.def("add_control_view", &sv::python::UiProxy::ui_add_control_view, "Add a control view for a configurable to the given tab.");
	py_ui_helper.def("add_plot_view",
		(std::string (sv::python::UiProxy::*) (std::string, Qt::DockWidgetArea, shared_ptr<sv::channels::BaseChannel>))
		&sv::python::UiProxy::ui_add_plot_view,
		"Add a time plot view for a channel to the given tab.");
	py_ui_helper.def("add_plot_view",
		(std::string (sv::python::UiProxy::*) (std::string, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>))
		&sv::python::UiProxy::ui_add_plot_view,
		"Add a time plot view for a signal to the given tab.");
	py_ui_helper.def("add_plot_view",
		(std::string (sv::python::UiProxy::*) (std::string, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>, shared_ptr<sv::data::AnalogTimeSignal>))
		&sv::python::UiProxy::ui_add_plot_view,
		"Add a x/y plot view for two signals to the given tab.");
	py_ui_helper.def("add_power_panel_view", &sv::python::UiProxy::ui_add_power_panel_view, "Add a power panel control for two signals to the given tab.");
	py_ui_helper.def("add_value_panel_view",
		(std::string (sv::python::UiProxy::*) (std::string, Qt::DockWidgetArea, shared_ptr<sv::channels::BaseChannel>))
		&sv::python::UiProxy::ui_add_value_panel_view,
		"Add a value panel view for a channel to the given tab.");
	py_ui_helper.def("add_value_panel_view",
		(std::string (sv::python::UiProxy::*) (std::string, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>))
		&sv::python::UiProxy::ui_add_value_panel_view,
		"Add a value panel view for a signal to the given tab.");

	py_ui_helper.def("add_signal_to_data_view", &sv::python::UiProxy::ui_add_signal_to_data_view, "Add a signal to the given data view.");
	py_ui_helper.def("add_signal_to_plot_view", &sv::python::UiProxy::ui_add_signal_to_plot_view, "Add a signal to the given plot view. If the plot is a x/y plot, the signal will be added as a new y signal.");
	py_ui_helper.def("add_signals_to_xy_plot_view", &sv::python::UiProxy::ui_add_signals_to_xy_plot_view, "Add x/y signals to the given x/y plot view.");

	// Qt enumerations
	py::enum_<Qt::DockWidgetArea> py_dock_area(m, "DockArea", "Enum of all possible docking locations for a view.");
	py_dock_area.value("LeftDocktArea", Qt::DockWidgetArea::LeftDockWidgetArea);
	py_dock_area.value("RightDockArea", Qt::DockWidgetArea::RightDockWidgetArea);
	py_dock_area.value("TopDockArea", Qt::DockWidgetArea::TopDockWidgetArea);
	py_dock_area.value("BottomDockArea", Qt::DockWidgetArea::BottomDockWidgetArea);
	//py_dock_area.value("AllDockAreas", Qt::DockWidgetArea::AllDockWidgetAreas);
	//py_dock_area.value("NoDockArea", Qt::DockWidgetArea::NoDockWidgetArea);
}

void init_StreamBuf(py::module &m)
{
	py::class_<sv::python::PyStreamBuf> py_stream_buf(m, "PyStreamBuf");
	py_stream_buf.doc() = "Redirect Python output to SmuView console.";
	py_stream_buf.def(py::init<const std::string &, const std::string &>());
	py_stream_buf.def("close", &sv::python::PyStreamBuf::py_close, "Flush and close this stream.");
	py_stream_buf.def("fileno", &sv::python::PyStreamBuf::py_fileno, "Raises an OSError, because PyStreamBuf doesn't use a file descriptor.");
	py_stream_buf.def("flush", &sv::python::PyStreamBuf::py_flush, "Flush the write buffers of the stream.");
	py_stream_buf.def("isatty", &sv::python::PyStreamBuf::py_isatty, "Always return False.");
	py_stream_buf.def("readable", &sv::python::PyStreamBuf::py_readable, "Always return False.");
	py_stream_buf.def("readlines", &sv::python::PyStreamBuf::py_readlines, "Raises an OSError, because PyStreamBuf is write only.");
	py_stream_buf.def("seekable", &sv::python::PyStreamBuf::py_seekable, "Always return False. PyStreamBuf is not seekable atm.");
	py_stream_buf.def("truncate", &sv::python::PyStreamBuf::py_truncate, "Raises an OSError, because PyStreamBuf is not seekable.");
	py_stream_buf.def("writable", &sv::python::PyStreamBuf::py_writable, "Always return True.");
	py_stream_buf.def("writelines", &sv::python::PyStreamBuf::py_writelines, "Write a list of lines to the stream.");
	py_stream_buf.def("__del__", &sv::python::PyStreamBuf::py_del, "Prepare for object destruction.");
	py_stream_buf.def("read", &sv::python::PyStreamBuf::py_read, "Raises an OSError, because PyStreamBuf is write only.");
	py_stream_buf.def("readline", &sv::python::PyStreamBuf::py_readline, "Raises an OSError, because PyStreamBuf is write only.");
	py_stream_buf.def("seek", &sv::python::PyStreamBuf::py_seek, "Raises an OSError, because PyStreamBuf is not seekable.");
	py_stream_buf.def("tell", &sv::python::PyStreamBuf::py_tell, "Raises an OSError, because PyStreamBuf is not seekable.");
	py_stream_buf.def("write", &sv::python::PyStreamBuf::py_write, "Write the string s to the stream and return the number of characters written.");
	py_stream_buf.def_readonly("closed", &sv::python::PyStreamBuf::py_closed, "True if the stream is closed.");
	py_stream_buf.def_readonly("encoding", &sv::python::PyStreamBuf::py_encoding, "The name of the encoding that is used.");
	py_stream_buf.def_readonly("errors", &sv::python::PyStreamBuf::py_errors, "The error setting of the decoder or encoder.");
}

void init_Enums(py::module &m)
{
	py::enum_<sv::devices::ConfigKey> py_config_key(m, "ConfigKey", "Enum of all available config keys for controlling a device.");
	py_config_key.value("Samplerate", sv::devices::ConfigKey::Samplerate);
	py_config_key.value("CaptureRatio", sv::devices::ConfigKey::CaptureRatio);
	py_config_key.value("PatternMode", sv::devices::ConfigKey::PatternMode);
	py_config_key.value("RLE", sv::devices::ConfigKey::RLE);
	py_config_key.value("TriggerSlope", sv::devices::ConfigKey::TriggerSlope);
	py_config_key.value("Averaging", sv::devices::ConfigKey::Averaging);
	py_config_key.value("AvgSamples", sv::devices::ConfigKey::AvgSamples);
	py_config_key.value("TriggerSource", sv::devices::ConfigKey::TriggerSource);
	py_config_key.value("HorizTriggerPos", sv::devices::ConfigKey::HorizTriggerPos);
	py_config_key.value("BufferSize", sv::devices::ConfigKey::BufferSize);
	py_config_key.value("TimeBase", sv::devices::ConfigKey::TimeBase);
	py_config_key.value("Filter", sv::devices::ConfigKey::Filter);
	py_config_key.value("VDiv", sv::devices::ConfigKey::VDiv);
	py_config_key.value("Coupling", sv::devices::ConfigKey::Coupling);
	py_config_key.value("TriggerMatch", sv::devices::ConfigKey::TriggerMatch);
	py_config_key.value("SampleInterval", sv::devices::ConfigKey::SampleInterval);
	py_config_key.value("NumHDiv", sv::devices::ConfigKey::NumHDiv);
	py_config_key.value("NumVDiv", sv::devices::ConfigKey::NumVDiv);
	py_config_key.value("SplWeightFreq", sv::devices::ConfigKey::SplWeightFreq);
	py_config_key.value("SplWeightTime", sv::devices::ConfigKey::SplWeightTime);
	py_config_key.value("SplMeasurementRange", sv::devices::ConfigKey::SplMeasurementRange);
	py_config_key.value("HoldMax", sv::devices::ConfigKey::HoldMax);
	py_config_key.value("HoldMin", sv::devices::ConfigKey::HoldMin);
	py_config_key.value("VoltageThreshold", sv::devices::ConfigKey::VoltageThreshold);
	py_config_key.value("ExternalClock", sv::devices::ConfigKey::ExternalClock);
	py_config_key.value("Swap", sv::devices::ConfigKey::Swap);
	py_config_key.value("CenterFrequency", sv::devices::ConfigKey::CenterFrequency);
	py_config_key.value("NumLogicChannels", sv::devices::ConfigKey::NumLogicChannels);
	py_config_key.value("NumAnalogChannels", sv::devices::ConfigKey::NumAnalogChannels);
	py_config_key.value("Voltage", sv::devices::ConfigKey::Voltage);
	py_config_key.value("VoltageTarget", sv::devices::ConfigKey::VoltageTarget);
	py_config_key.value("Current", sv::devices::ConfigKey::Current);
	py_config_key.value("CurrentLimit", sv::devices::ConfigKey::CurrentLimit);
	py_config_key.value("Enabled", sv::devices::ConfigKey::Enabled);
	py_config_key.value("ChannelConfig", sv::devices::ConfigKey::ChannelConfig);
	py_config_key.value("OverVoltageProtectionEnabled", sv::devices::ConfigKey::OverVoltageProtectionEnabled);
	py_config_key.value("OverVoltageProtectionActive", sv::devices::ConfigKey::OverVoltageProtectionActive);
	py_config_key.value("OverVoltageProtectionThreshold", sv::devices::ConfigKey::OverVoltageProtectionThreshold);
	py_config_key.value("OverCurrentProtectionEnabled", sv::devices::ConfigKey::OverCurrentProtectionEnabled);
	py_config_key.value("OverCurrentProtectionActive", sv::devices::ConfigKey::OverCurrentProtectionActive);
	py_config_key.value("OverCurrentProtectionThreshold", sv::devices::ConfigKey::OverCurrentProtectionThreshold);
	py_config_key.value("OverTemperatureProtectionEnabled", sv::devices::ConfigKey::OverTemperatureProtectionEnabled);
	py_config_key.value("OverTemperatureProtectionActive", sv::devices::ConfigKey::OverTemperatureProtectionActive);
	py_config_key.value("UnderVoltageConditionEnabled", sv::devices::ConfigKey::UnderVoltageConditionEnabled);
	py_config_key.value("UnderVoltageConditionActive", sv::devices::ConfigKey::UnderVoltageConditionActive);
	py_config_key.value("UnderVoltageConditionThreshold", sv::devices::ConfigKey::UnderVoltageConditionThreshold);
	py_config_key.value("ClockEdge", sv::devices::ConfigKey::ClockEdge);
	py_config_key.value("Amplitude", sv::devices::ConfigKey::Amplitude);
	py_config_key.value("Offset", sv::devices::ConfigKey::Offset);
	py_config_key.value("Regulation", sv::devices::ConfigKey::Regulation);
	py_config_key.value("OutputFrequency", sv::devices::ConfigKey::OutputFrequency);
	py_config_key.value("OutputFrequencyTarget", sv::devices::ConfigKey::OutputFrequencyTarget);
	py_config_key.value("MeasuredQuantity", sv::devices::ConfigKey::MeasuredQuantity);
	py_config_key.value("EquivCircuitModel", sv::devices::ConfigKey::EquivCircuitModel);
	py_config_key.value("TriggerLevel", sv::devices::ConfigKey::TriggerLevel);
	py_config_key.value("ExternalClockSource", sv::devices::ConfigKey::ExternalClockSource);
	py_config_key.value("SessionFile", sv::devices::ConfigKey::SessionFile);
	py_config_key.value("CaptureFile", sv::devices::ConfigKey::CaptureFile);
	py_config_key.value("CaptureUnitSize", sv::devices::ConfigKey::CaptureUnitSize);
	py_config_key.value("PowerOff", sv::devices::ConfigKey::PowerOff);
	py_config_key.value("DataSource", sv::devices::ConfigKey::DataSource);
	py_config_key.value("ProbeFactor", sv::devices::ConfigKey::ProbeFactor);
	py_config_key.value("ADCPowerlineCycles", sv::devices::ConfigKey::ADCPowerlineCycles);
	py_config_key.value("DataLog", sv::devices::ConfigKey::DataLog);
	py_config_key.value("DeviceMode", sv::devices::ConfigKey::DeviceMode);
	py_config_key.value("TestMode", sv::devices::ConfigKey::TestMode);
	py_config_key.value("Unknown", sv::devices::ConfigKey::Unknown);

	py::enum_<sv::data::Quantity> py_quantity(m, "Quantity", "Enum of all available quantities.");
	py_quantity.value("Voltage", sv::data::Quantity::Voltage);
	py_quantity.value("Current", sv::data::Quantity::Current);
	py_quantity.value("Resistance", sv::data::Quantity::Resistance);
	py_quantity.value("Capacitance", sv::data::Quantity::Capacitance);
	py_quantity.value("Temperature", sv::data::Quantity::Temperature);
	py_quantity.value("Frequency", sv::data::Quantity::Frequency);
	py_quantity.value("DutyCyle", sv::data::Quantity::DutyCyle);
	py_quantity.value("Continuity", sv::data::Quantity::Continuity);
	py_quantity.value("PulseWidth", sv::data::Quantity::PulseWidth);
	py_quantity.value("Conductance", sv::data::Quantity::Conductance);
	py_quantity.value("Power", sv::data::Quantity::Power);
	py_quantity.value("Work", sv::data::Quantity::Work);
	py_quantity.value("ElectricCharge", sv::data::Quantity::ElectricCharge);
	py_quantity.value("Gain", sv::data::Quantity::Gain);
	py_quantity.value("SoundPressureLevel", sv::data::Quantity::SoundPressureLevel);
	py_quantity.value("CarbonMonoxide", sv::data::Quantity::CarbonMonoxide);
	py_quantity.value("RelativeHumidity", sv::data::Quantity::RelativeHumidity);
	py_quantity.value("Time", sv::data::Quantity::Time);
	py_quantity.value("WindSpeed", sv::data::Quantity::WindSpeed);
	py_quantity.value("Pressure", sv::data::Quantity::Pressure);
	py_quantity.value("ParallelInductance", sv::data::Quantity::ParallelInductance);
	py_quantity.value("ParallelCapacitance", sv::data::Quantity::ParallelCapacitance);
	py_quantity.value("ParallelResistance", sv::data::Quantity::ParallelResistance);
	py_quantity.value("SeriesInductance", sv::data::Quantity::SeriesInductance);
	py_quantity.value("SeriesCapacitance", sv::data::Quantity::SeriesCapacitance);
	py_quantity.value("SeriesResistance", sv::data::Quantity::SeriesResistance);
	py_quantity.value("DissipationFactor", sv::data::Quantity::DissipationFactor);
	py_quantity.value("QualityFactor", sv::data::Quantity::QualityFactor);
	py_quantity.value("PhaseAngle", sv::data::Quantity::PhaseAngle);
	py_quantity.value("Difference", sv::data::Quantity::Difference);
	py_quantity.value("Count", sv::data::Quantity::Count);
	py_quantity.value("PowerFactor", sv::data::Quantity::PowerFactor);
	py_quantity.value("ApparentPower", sv::data::Quantity::ApparentPower);
	py_quantity.value("Mass", sv::data::Quantity::Mass);
	py_quantity.value("HarmonicRatio", sv::data::Quantity::HarmonicRatio);
	py_quantity.value("Unknown", sv::data::Quantity::Unknown);

	py::enum_<sv::data::QuantityFlag> py_quantity_flag(m, "QuantityFlag", "Enum of all available quantity flags.");
	py_quantity_flag.value("AC", sv::data::QuantityFlag::AC);
	py_quantity_flag.value("DC", sv::data::QuantityFlag::DC);
	py_quantity_flag.value("RMS", sv::data::QuantityFlag::RMS);
	py_quantity_flag.value("Diode", sv::data::QuantityFlag::Diode);
	py_quantity_flag.value("Hold", sv::data::QuantityFlag::Hold);
	py_quantity_flag.value("Max", sv::data::QuantityFlag::Max);
	py_quantity_flag.value("Min", sv::data::QuantityFlag::Min);
	py_quantity_flag.value("Autorange", sv::data::QuantityFlag::Autorange);
	py_quantity_flag.value("Relative", sv::data::QuantityFlag::Relative);
	py_quantity_flag.value("SplFreqWeightA", sv::data::QuantityFlag::SplFreqWeightA);
	py_quantity_flag.value("SplFreqWeightC", sv::data::QuantityFlag::SplFreqWeightC);
	py_quantity_flag.value("SplFreqWeightZ", sv::data::QuantityFlag::SplFreqWeightZ);
	py_quantity_flag.value("SplFreqWeightFlat", sv::data::QuantityFlag::SplFreqWeightFlat);
	py_quantity_flag.value("SplTimeWeightS", sv::data::QuantityFlag::SplTimeWeightS);
	py_quantity_flag.value("SplTimeWeightF", sv::data::QuantityFlag::SplTimeWeightF);
	py_quantity_flag.value("SplLAT", sv::data::QuantityFlag::SplLAT);
	py_quantity_flag.value("SplPctOverAlarm", sv::data::QuantityFlag::SplPctOverAlarm);
	py_quantity_flag.value("Duration", sv::data::QuantityFlag::Duration);
	py_quantity_flag.value("Avg", sv::data::QuantityFlag::Avg);
	py_quantity_flag.value("Reference", sv::data::QuantityFlag::Reference);
	py_quantity_flag.value("Unstable", sv::data::QuantityFlag::Unstable);
	py_quantity_flag.value("FourWire", sv::data::QuantityFlag::FourWire);
	py_quantity_flag.value("Unknown", sv::data::QuantityFlag::Unknown);

	py::enum_<sv::data::Unit> py_unit(m, "Unit", "Enum of all available units.");
	py_unit.value("Volt", sv::data::Unit::Volt);
	py_unit.value("Ampere", sv::data::Unit::Ampere);
	py_unit.value("Ohm", sv::data::Unit::Ohm);
	py_unit.value("Farad", sv::data::Unit::Farad);
	py_unit.value("Kelvin", sv::data::Unit::Kelvin);
	py_unit.value("Celsius", sv::data::Unit::Celsius);
	py_unit.value("Fahrenheit", sv::data::Unit::Fahrenheit);
	py_unit.value("Hertz", sv::data::Unit::Hertz);
	py_unit.value("Percentage", sv::data::Unit::Percentage);
	py_unit.value("Boolean", sv::data::Unit::Boolean);
	py_unit.value("Second", sv::data::Unit::Second);
	py_unit.value("Siemens", sv::data::Unit::Siemens);
	py_unit.value("DecibelMW", sv::data::Unit::DecibelMW);
	py_unit.value("DecibelVolt", sv::data::Unit::DecibelVolt);
	py_unit.value("Decibel", sv::data::Unit::Decibel);
	py_unit.value("Unitless", sv::data::Unit::Unitless);
	py_unit.value("DecibelSpl", sv::data::Unit::DecibelSpl);
	py_unit.value("Concentration", sv::data::Unit::Concentration);
	py_unit.value("RevolutionsPerMinute", sv::data::Unit::RevolutionsPerMinute);
	py_unit.value("VoltAmpere", sv::data::Unit::VoltAmpere);
	py_unit.value("Watt", sv::data::Unit::Watt);
	py_unit.value("WattHour", sv::data::Unit::WattHour);
	py_unit.value("Joule", sv::data::Unit::Joule);
	py_unit.value("AmpereHour", sv::data::Unit::AmpereHour);
	py_unit.value("Coulomb", sv::data::Unit::Coulomb);
	py_unit.value("MeterPerSecond", sv::data::Unit::MeterPerSecond);
	py_unit.value("HectoPascal", sv::data::Unit::HectoPascal);
	py_unit.value("Humidity293K", sv::data::Unit::Humidity293K);
	py_unit.value("Degree", sv::data::Unit::Degree);
	py_unit.value("Henry", sv::data::Unit::Henry);
	py_unit.value("Gram", sv::data::Unit::Gram);
	py_unit.value("Carat", sv::data::Unit::Carat);
	py_unit.value("Ounce", sv::data::Unit::Ounce);
	py_unit.value("TroyOunce", sv::data::Unit::TroyOunce);
	py_unit.value("Pound", sv::data::Unit::Pound);
	py_unit.value("Pennyweight", sv::data::Unit::Pennyweight);
	py_unit.value("Grain", sv::data::Unit::Grain);
	py_unit.value("Tael", sv::data::Unit::Tael);
	py_unit.value("Momme", sv::data::Unit::Momme);
	py_unit.value("Tola", sv::data::Unit::Tola);
	py_unit.value("Piece", sv::data::Unit::Piece);
	py_unit.value("Unknown", sv::data::Unit::Unknown);
}
