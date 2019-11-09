/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
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
#include "src/python/uiproxy.hpp"

using namespace pybind11::literals; // for the ""_a
namespace py = pybind11;

PYBIND11_EMBEDDED_MODULE(smuview, m) {
	init_Session(m);
	init_Device(m);
	init_Channel(m);
	init_Signal(m);
	init_Configurable(m);
	init_UI(m);
	init_Enums(m);
}

void init_Session(py::module &m)
{
	py::class_<sv::Session>(m, "Session")
		.def("devices", &sv::Session::devices)
		.def("connect_device", &sv::Session::connect_device)
		.def("add_user_device", &sv::Session::add_user_device);
}

void init_Device(py::module &m)
{
	py::class_<sv::devices::BaseDevice, std::shared_ptr<sv::devices::BaseDevice>> base_device(m, "BaseDevice");
	base_device.def("name", &sv::devices::BaseDevice::name);
	base_device.def("id", &sv::devices::BaseDevice::id);
	base_device.def("channels", &sv::devices::BaseDevice::channel_map);
	base_device.def("configurables", &sv::devices::BaseDevice::configurable_map);
	base_device.def("add_user_channel", &sv::devices::BaseDevice::add_user_channel);

	py::class_<sv::devices::HardwareDevice, std::shared_ptr<sv::devices::HardwareDevice>>(m, "HardwareDevice", base_device);

	py::class_<sv::devices::UserDevice, std::shared_ptr<sv::devices::UserDevice>>(m, "UserDevice", base_device);
}

void init_Channel(py::module &m)
{
	py::class_<sv::channels::BaseChannel, std::shared_ptr<sv::channels::BaseChannel>> base_channel(m, "BaseChannel");
	base_channel.def("name", &sv::channels::BaseChannel::name);
	base_channel.def("add_signal",
		(shared_ptr<sv::data::BaseSignal> (sv::channels::BaseChannel::*)
		(sv::data::Quantity, set<sv::data::QuantityFlag>, sv::data::Unit))
			&sv::channels::BaseChannel::add_signal);
	base_channel.def("actual_signal", &sv::channels::BaseChannel::actual_signal);
	base_channel.def("signals", &sv::channels::BaseChannel::signals);

	py::class_<sv::channels::HardwareChannel, std::shared_ptr<sv::channels::HardwareChannel>>(m, "HardwareChannel", base_channel);

	py::class_<sv::channels::UserChannel, std::shared_ptr<sv::channels::UserChannel>>(m, "UserChannel", base_channel)
		.def("push_sample", &sv::channels::UserChannel::push_sample);
}

void init_Signal(py::module &m)
{
	py::class_<sv::data::BaseSignal, std::shared_ptr<sv::data::BaseSignal>> base_signal(m, "BaseSignal");
	base_signal.def("name", &sv::data::BaseSignal::name);
	base_signal.def("sample_count", &sv::data::BaseSignal::sample_count);

	py::class_<sv::data::AnalogTimeSignal, std::shared_ptr<sv::data::AnalogTimeSignal>>(m, "AnalogTimeSignal", base_signal)
		.def("get_sample", &sv::data::AnalogTimeSignal::get_sample)
		.def("get_last_sample", &sv::data::AnalogTimeSignal::get_last_sample)
		.def("push_sample", &sv::data::AnalogTimeSignal::push_sample);

	py::class_<sv::data::AnalogSampleSignal, std::shared_ptr<sv::data::AnalogSampleSignal>>(m, "AnalogSampleSignal", base_signal)
		.def("get_sample", &sv::data::AnalogSampleSignal::get_sample)
		.def("push_sample", &sv::data::AnalogSampleSignal::push_sample);
}

void init_Configurable(py::module &m)
{
	/*
	 * TODO:
	 *  - list
	 */

	py::class_<sv::devices::Configurable, std::shared_ptr<sv::devices::Configurable>>(m, "Configurable")
		.def("name", &sv::devices::Configurable::name)
		.def("set_config", &sv::devices::Configurable::set_config<bool>)
		.def("set_config", &sv::devices::Configurable::set_config<int32_t>)
		.def("set_config", &sv::devices::Configurable::set_config<uint64_t>)
		.def("set_config", &sv::devices::Configurable::set_config<double>)
		.def("set_config", &sv::devices::Configurable::set_config<std::string>)
		.def("get_config", &sv::devices::Configurable::set_config<bool>)
		.def("get_config", &sv::devices::Configurable::set_config<int32_t>)
		.def("get_config", &sv::devices::Configurable::set_config<uint64_t>)
		.def("get_config", &sv::devices::Configurable::set_config<double>)
		.def("get_config", &sv::devices::Configurable::set_config<std::string>);
}

void init_UI(py::module &m)
{
	py::class_<sv::python::UiProxy>(m, "UiProxy")
		.def("add_device_tab", &sv::python::UiProxy::ui_add_device_tab)
		.def("add_data_view", &sv::python::UiProxy::ui_add_data_view)
		.def("add_control_view", &sv::python::UiProxy::ui_add_control_view)
		.def("add_plot_view",
			(std::string (sv::python::UiProxy::*) (std::string, Qt::DockWidgetArea, shared_ptr<sv::channels::BaseChannel>))
			&sv::python::UiProxy::ui_add_plot_view)
		.def("add_plot_view",
			(std::string (sv::python::UiProxy::*) (std::string, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>))
			&sv::python::UiProxy::ui_add_plot_view)
		.def("add_plot_view",
			(std::string (sv::python::UiProxy::*) (std::string, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>, shared_ptr<sv::data::AnalogTimeSignal>))
			&sv::python::UiProxy::ui_add_plot_view)
		.def("add_power_panel_view", &sv::python::UiProxy::ui_add_power_panel_view)
		.def("add_value_panel_view",
			(std::string (sv::python::UiProxy::*) (std::string, Qt::DockWidgetArea, shared_ptr<sv::channels::BaseChannel>))
			&sv::python::UiProxy::ui_add_value_panel_view)
		.def("add_value_panel_view",
			(std::string (sv::python::UiProxy::*) (std::string, Qt::DockWidgetArea, shared_ptr<sv::data::AnalogTimeSignal>))
			&sv::python::UiProxy::ui_add_value_panel_view)
		.def("add_signal_to_plot", &sv::python::UiProxy::ui_add_signal_to_plot);

	// Qt enumerations
	py::enum_<Qt::DockWidgetArea>(m, "DockArea")
		.value("LeftDocktArea", Qt::DockWidgetArea::LeftDockWidgetArea)
		.value("RightDockArea", Qt::DockWidgetArea::RightDockWidgetArea)
		.value("TopDockArea", Qt::DockWidgetArea::TopDockWidgetArea)
		.value("BottomDockArea", Qt::DockWidgetArea::BottomDockWidgetArea);
		//.value("AllDockAreas", Qt::DockWidgetArea::AllDockWidgetAreas)
		//.value("NoDockArea", Qt::DockWidgetArea::NoDockWidgetArea);

	/*
	 * TODO:
	 *  - add signal to plot
	 */
}

void init_Enums(py::module &m)
{
	py::enum_<sv::devices::ConfigKey>(m, "ConfigKey")
		.value("Samplerate", sv::devices::ConfigKey::Samplerate)
		.value("CaptureRatio", sv::devices::ConfigKey::CaptureRatio)
		.value("PatternMode", sv::devices::ConfigKey::PatternMode)
		.value("RLE", sv::devices::ConfigKey::RLE)
		.value("TriggerSlope", sv::devices::ConfigKey::TriggerSlope)
		.value("Averaging", sv::devices::ConfigKey::Averaging)
		.value("AvgSamples", sv::devices::ConfigKey::AvgSamples)
		.value("TriggerSource", sv::devices::ConfigKey::TriggerSource)
		.value("HorizTriggerPos", sv::devices::ConfigKey::HorizTriggerPos)
		.value("BufferSize", sv::devices::ConfigKey::BufferSize)
		.value("TimeBase", sv::devices::ConfigKey::TimeBase)
		.value("Filter", sv::devices::ConfigKey::Filter)
		.value("VDiv", sv::devices::ConfigKey::VDiv)
		.value("Coupling", sv::devices::ConfigKey::Coupling)
		.value("TriggerMatch", sv::devices::ConfigKey::TriggerMatch)
		.value("SampleInterval", sv::devices::ConfigKey::SampleInterval)
		.value("NumHDiv", sv::devices::ConfigKey::NumHDiv)
		.value("NumVDiv", sv::devices::ConfigKey::NumVDiv)
		.value("SplWeightFreq", sv::devices::ConfigKey::SplWeightFreq)
		.value("SplWeightTime", sv::devices::ConfigKey::SplWeightTime)
		.value("SplMeasurementRange", sv::devices::ConfigKey::SplMeasurementRange)
		.value("HoldMax", sv::devices::ConfigKey::HoldMax)
		.value("HoldMin", sv::devices::ConfigKey::HoldMin)
		.value("VoltageThreshold", sv::devices::ConfigKey::VoltageThreshold)
		.value("ExternalClock", sv::devices::ConfigKey::ExternalClock)
		.value("Swap", sv::devices::ConfigKey::Swap)
		.value("CenterFrequency", sv::devices::ConfigKey::CenterFrequency)
		.value("NumLogicChannels", sv::devices::ConfigKey::NumLogicChannels)
		.value("NumAnalogChannels", sv::devices::ConfigKey::NumAnalogChannels)
		.value("Voltage", sv::devices::ConfigKey::Voltage)
		.value("VoltageTarget", sv::devices::ConfigKey::VoltageTarget)
		.value("Current", sv::devices::ConfigKey::Current)
		.value("CurrentLimit", sv::devices::ConfigKey::CurrentLimit)
		.value("Enabled", sv::devices::ConfigKey::Enabled)
		.value("ChannelConfig", sv::devices::ConfigKey::ChannelConfig)
		.value("OverVoltageProtectionEnabled", sv::devices::ConfigKey::OverVoltageProtectionEnabled)
		.value("OverVoltageProtectionActive", sv::devices::ConfigKey::OverVoltageProtectionActive)
		.value("OverVoltageProtectionThreshold", sv::devices::ConfigKey::OverVoltageProtectionThreshold)
		.value("OverCurrentProtectionEnabled", sv::devices::ConfigKey::OverCurrentProtectionEnabled)
		.value("OverCurrentProtectionActive", sv::devices::ConfigKey::OverCurrentProtectionActive)
		.value("OverCurrentProtectionThreshold", sv::devices::ConfigKey::OverCurrentProtectionThreshold)
		.value("OverTemperatureProtectionEnabled", sv::devices::ConfigKey::OverTemperatureProtectionEnabled)
		.value("OverTemperatureProtectionActive", sv::devices::ConfigKey::OverTemperatureProtectionActive)
		.value("UnderVoltageConditionEnabled", sv::devices::ConfigKey::UnderVoltageConditionEnabled)
		.value("UnderVoltageConditionActive", sv::devices::ConfigKey::UnderVoltageConditionActive)
		.value("UnderVoltageConditionThreshold", sv::devices::ConfigKey::UnderVoltageConditionThreshold)
		.value("ClockEdge", sv::devices::ConfigKey::ClockEdge)
		.value("Amplitude", sv::devices::ConfigKey::Amplitude)
		.value("Offset", sv::devices::ConfigKey::Offset)
		.value("Regulation", sv::devices::ConfigKey::Regulation)
		.value("OutputFrequency", sv::devices::ConfigKey::OutputFrequency)
		.value("OutputFrequencyTarget", sv::devices::ConfigKey::OutputFrequencyTarget)
		.value("MeasuredQuantity", sv::devices::ConfigKey::MeasuredQuantity)
		.value("EquivCircuitModel", sv::devices::ConfigKey::EquivCircuitModel)
		.value("TriggerLevel", sv::devices::ConfigKey::TriggerLevel)
		.value("ExternalClockSource", sv::devices::ConfigKey::ExternalClockSource)
		.value("SessionFile", sv::devices::ConfigKey::SessionFile)
		.value("CaptureFile", sv::devices::ConfigKey::CaptureFile)
		.value("CaptureUnitSize", sv::devices::ConfigKey::CaptureUnitSize)
		.value("PowerOff", sv::devices::ConfigKey::PowerOff)
		.value("DataSource", sv::devices::ConfigKey::DataSource)
		.value("ProbeFactor", sv::devices::ConfigKey::ProbeFactor)
		.value("ADCPowerlineCycles", sv::devices::ConfigKey::ADCPowerlineCycles)
		.value("DataLog", sv::devices::ConfigKey::DataLog)
		.value("DeviceMode", sv::devices::ConfigKey::DeviceMode)
		.value("TestMode", sv::devices::ConfigKey::TestMode)
		.value("Unknown", sv::devices::ConfigKey::Unknown)
		.export_values(); // TODO

	py::enum_<sv::data::Quantity>(m, "Quantity")
		.value("Voltage", sv::data::Quantity::Voltage)
		.value("Current", sv::data::Quantity::Current)
		.value("Resistance", sv::data::Quantity::Resistance)
		.value("Capacitance", sv::data::Quantity::Capacitance)
		.value("Temperature", sv::data::Quantity::Temperature)
		.value("Frequency", sv::data::Quantity::Frequency)
		.value("DutyCyle", sv::data::Quantity::DutyCyle)
		.value("Continuity", sv::data::Quantity::Continuity)
		.value("PulseWidth", sv::data::Quantity::PulseWidth)
		.value("Conductance", sv::data::Quantity::Conductance)
		.value("Power", sv::data::Quantity::Power)
		.value("Work", sv::data::Quantity::Work)
		.value("ElectricCharge", sv::data::Quantity::ElectricCharge)
		.value("Gain", sv::data::Quantity::Gain)
		.value("SoundPressureLevel", sv::data::Quantity::SoundPressureLevel)
		.value("CarbonMonoxide", sv::data::Quantity::CarbonMonoxide)
		.value("RelativeHumidity", sv::data::Quantity::RelativeHumidity)
		.value("Time", sv::data::Quantity::Time)
		.value("WindSpeed", sv::data::Quantity::WindSpeed)
		.value("Pressure", sv::data::Quantity::Pressure)
		.value("ParallelInductance", sv::data::Quantity::ParallelInductance)
		.value("ParallelCapacitance", sv::data::Quantity::ParallelCapacitance)
		.value("ParallelResistance", sv::data::Quantity::ParallelResistance)
		.value("SeriesInductance", sv::data::Quantity::SeriesInductance)
		.value("SeriesCapacitance", sv::data::Quantity::SeriesCapacitance)
		.value("SeriesResistance", sv::data::Quantity::SeriesResistance)
		.value("DissipationFactor", sv::data::Quantity::DissipationFactor)
		.value("QualityFactor", sv::data::Quantity::QualityFactor)
		.value("PhaseAngle", sv::data::Quantity::PhaseAngle)
		.value("Difference", sv::data::Quantity::Difference)
		.value("Count", sv::data::Quantity::Count)
		.value("PowerFactor", sv::data::Quantity::PowerFactor)
		.value("ApparentPower", sv::data::Quantity::ApparentPower)
		.value("Mass", sv::data::Quantity::Mass)
		.value("HarmonicRatio", sv::data::Quantity::HarmonicRatio)
		.value("Unknown", sv::data::Quantity::Unknown);

	py::enum_<sv::data::QuantityFlag>(m, "QuantityFlag")
		.value("AC", sv::data::QuantityFlag::AC)
		.value("DC", sv::data::QuantityFlag::DC)
		.value("RMS", sv::data::QuantityFlag::RMS)
		.value("Diode", sv::data::QuantityFlag::Diode)
		.value("Hold", sv::data::QuantityFlag::Hold)
		.value("Max", sv::data::QuantityFlag::Max)
		.value("Min", sv::data::QuantityFlag::Min)
		.value("Autorange", sv::data::QuantityFlag::Autorange)
		.value("Relative", sv::data::QuantityFlag::Relative)
		.value("SplFreqWeightA", sv::data::QuantityFlag::SplFreqWeightA)
		.value("SplFreqWeightC", sv::data::QuantityFlag::SplFreqWeightC)
		.value("SplFreqWeightZ", sv::data::QuantityFlag::SplFreqWeightZ)
		.value("SplFreqWeightFlat", sv::data::QuantityFlag::SplFreqWeightFlat)
		.value("SplTimeWeightS", sv::data::QuantityFlag::SplTimeWeightS)
		.value("SplTimeWeightF", sv::data::QuantityFlag::SplTimeWeightF)
		.value("SplLAT", sv::data::QuantityFlag::SplLAT)
		.value("SplPctOverAlarm", sv::data::QuantityFlag::SplPctOverAlarm)
		.value("Duration", sv::data::QuantityFlag::Duration)
		.value("Avg", sv::data::QuantityFlag::Avg)
		.value("Reference", sv::data::QuantityFlag::Reference)
		.value("Unstable", sv::data::QuantityFlag::Unstable)
		.value("FourWire", sv::data::QuantityFlag::FourWire)
		.value("Unknown", sv::data::QuantityFlag::Unknown);

	py::enum_<sv::data::Unit>(m, "Unit")
		.value("Volt", sv::data::Unit::Volt)
		.value("Ampere", sv::data::Unit::Ampere)
		.value("Ohm", sv::data::Unit::Ohm)
		.value("Farad", sv::data::Unit::Farad)
		.value("Kelvin", sv::data::Unit::Kelvin)
		.value("Celsius", sv::data::Unit::Celsius)
		.value("Fahrenheit", sv::data::Unit::Fahrenheit)
		.value("Hertz", sv::data::Unit::Hertz)
		.value("Percentage", sv::data::Unit::Percentage)
		.value("Boolean", sv::data::Unit::Boolean)
		.value("Second", sv::data::Unit::Second)
		.value("Siemens", sv::data::Unit::Siemens)
		.value("DecibelMW", sv::data::Unit::DecibelMW)
		.value("DecibelVolt", sv::data::Unit::DecibelVolt)
		.value("Decibel", sv::data::Unit::Decibel)
		.value("Unitless", sv::data::Unit::Unitless)
		.value("DecibelSpl", sv::data::Unit::DecibelSpl)
		.value("Concentration", sv::data::Unit::Concentration)
		.value("RevolutionsPerMinute", sv::data::Unit::RevolutionsPerMinute)
		.value("VoltAmpere", sv::data::Unit::VoltAmpere)
		.value("Watt", sv::data::Unit::Watt)
		.value("WattHour", sv::data::Unit::WattHour)
		.value("Joule", sv::data::Unit::Joule)
		.value("AmpereHour", sv::data::Unit::AmpereHour)
		.value("Coulomb", sv::data::Unit::Coulomb)
		.value("MeterPerSecond", sv::data::Unit::MeterPerSecond)
		.value("HectoPascal", sv::data::Unit::HectoPascal)
		.value("Humidity293K", sv::data::Unit::Humidity293K)
		.value("Degree", sv::data::Unit::Degree)
		.value("Henry", sv::data::Unit::Henry)
		.value("Gram", sv::data::Unit::Gram)
		.value("Carat", sv::data::Unit::Carat)
		.value("Ounce", sv::data::Unit::Ounce)
		.value("TroyOunce", sv::data::Unit::TroyOunce)
		.value("Pound", sv::data::Unit::Pound)
		.value("Pennyweight", sv::data::Unit::Pennyweight)
		.value("Grain", sv::data::Unit::Grain)
		.value("Tael", sv::data::Unit::Tael)
		.value("Momme", sv::data::Unit::Momme)
		.value("Tola", sv::data::Unit::Tola)
		.value("Piece", sv::data::Unit::Piece)
		.value("Unknown", sv::data::Unit::Unknown);
}
