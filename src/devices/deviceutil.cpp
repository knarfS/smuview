/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2026 Frank Stettner <frank-stettner@gmx.net>
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

#include <cassert>
#include <libsigrokcxx/libsigrokcxx.hpp>
#include <memory>

#include "deviceutil.hpp"
#include "src/data/datautil.hpp"

using std::shared_ptr;

namespace sv {
namespace devices {
namespace deviceutil {

// TODO: Use tr(), QCoreApplication::translate(), QT_TR_NOOP() or
//       QT_TRANSLATE_NOOP() for translation.
//       See: http://doc.qt.io/qt-5/i18n-source-translation.html

static const std::vector<DeviceTypeMapping> device_type_map = {
	{ DeviceType::LogicAnalyzer,   sigrok::ConfigKey::LOGIC_ANALYZER,   "Logic Analyzer" },
	{ DeviceType::Oscilloscope,    sigrok::ConfigKey::OSCILLOSCOPE,     "Oscilloscope" },
	{ DeviceType::Multimeter,      sigrok::ConfigKey::MULTIMETER,       "Multimeter" },
	{ DeviceType::DemoDev,         sigrok::ConfigKey::DEMO_DEV,         "Demo Device" },
	{ DeviceType::SoundLevelMeter, sigrok::ConfigKey::SOUNDLEVELMETER,  "Soundlevelmeter" },
	{ DeviceType::Thermometer,     sigrok::ConfigKey::THERMOMETER,      "Thermometer" },
	{ DeviceType::Hygrometer,      sigrok::ConfigKey::HYGROMETER,       "Hygrometer" },
	{ DeviceType::Energymeter,     sigrok::ConfigKey::ENERGYMETER,      "Energymeter" },
	{ DeviceType::Demodulator,     sigrok::ConfigKey::DEMODULATOR,      "Demodulator" },
	{ DeviceType::PowerSupply,     sigrok::ConfigKey::POWER_SUPPLY,     "Power Supply" },
	{ DeviceType::LcrMeter,        sigrok::ConfigKey::LCRMETER,         "LCR Meter" },
	{ DeviceType::ElectronicLoad,  sigrok::ConfigKey::ELECTRONIC_LOAD,  "Electronic Load" },
	{ DeviceType::Scale,           sigrok::ConfigKey::SCALE,            "Scale" },
	{ DeviceType::SignalGenerator, sigrok::ConfigKey::SIGNAL_GENERATOR, "Signal Generator" },
	{ DeviceType::Powermeter,      sigrok::ConfigKey::POWERMETER,       "Power Meter" },
	{ DeviceType::Multiplexer,     sigrok::ConfigKey::MULTIPLEXER,      "Multiplexer" },
	{ DeviceType::Any,             nullptr,                             "Any device type" },
	{ DeviceType::UserDevice,      nullptr,                             "Virtual User Device" },
	{ DeviceType::Unknown,         nullptr,                             "Unknown"},
};

static const std::vector<ConnectionKeyMapping> connection_key_map = {
	{ ConnectionKey::Conn,         sigrok::ConfigKey::CONN,          "Connection String" },
	{ ConnectionKey::SerialComm,   sigrok::ConfigKey::SERIALCOMM,    "Serial Command" },
	{ ConnectionKey::ModbusAddr,   sigrok::ConfigKey::MODBUSADDR,    "ModBus Address" },
	{ ConnectionKey::LimitMsec,    sigrok::ConfigKey::LIMIT_MSEC,    "Limit Milliseconds" },
	{ ConnectionKey::LimitSamples, sigrok::ConfigKey::LIMIT_SAMPLES, "Limit Samples" },
	{ ConnectionKey::LimitFrames,  sigrok::ConfigKey::LIMIT_FRAMES,  "Limit Frames" },
	{ ConnectionKey::Continuous,   sigrok::ConfigKey::CONTINUOUS,    "Continuous" },
	{ ConnectionKey::Unknown,      nullptr,                          "Unknown" },
};

static const std::vector<ConfigKeyMapping> config_key_map = {
	{ ConfigKey::Samplerate,                       sigrok::ConfigKey::SAMPLERATE,                         "Samplerate" },
	{ ConfigKey::CaptureRatio,                     sigrok::ConfigKey::CAPTURE_RATIO,                      "Capture Ratio" },
	{ ConfigKey::PatternMode,                      sigrok::ConfigKey::PATTERN_MODE,                       "Pattern Mode" },
	{ ConfigKey::RLE,                              sigrok::ConfigKey::RLE,                                "Run-Length Encoding" },
	{ ConfigKey::TriggerSlope,                     sigrok::ConfigKey::TRIGGER_SLOPE,                      "Trigger Slope" },
	{ ConfigKey::Averaging,                        sigrok::ConfigKey::AVERAGING,                          "Averaging" },
	{ ConfigKey::AvgSamples,                       sigrok::ConfigKey::AVG_SAMPLES,                        "Averaging Samples" },
	{ ConfigKey::TriggerSource,                    sigrok::ConfigKey::TRIGGER_SOURCE,                     "Trigger Source" },
	{ ConfigKey::HorizTriggerPos,                  sigrok::ConfigKey::HORIZ_TRIGGERPOS,                   "Horizonal Trigger Position" },
	{ ConfigKey::BufferSize,                       sigrok::ConfigKey::BUFFERSIZE,                         "Buffer Size" },
	{ ConfigKey::TimeBase,                         sigrok::ConfigKey::TIMEBASE,                           "Time Base" },
	{ ConfigKey::Filter,                           sigrok::ConfigKey::FILTER,                             "Filter" },
	{ ConfigKey::VDiv,                             sigrok::ConfigKey::VDIV,                               "Vertical Division" },
	{ ConfigKey::Coupling,                         sigrok::ConfigKey::COUPLING,                           "Coupling" },
	{ ConfigKey::TriggerMatch,                     sigrok::ConfigKey::TRIGGER_MATCH,                      "Trigger Match" },
	{ ConfigKey::SampleInterval,                   sigrok::ConfigKey::SAMPLE_INTERVAL,                    "Sample Interval" },
	{ ConfigKey::NumHDiv,                          sigrok::ConfigKey::NUM_HDIV,                           "Number Horizontal Divisions" },
	{ ConfigKey::NumVDiv,                          sigrok::ConfigKey::NUM_VDIV,                           "Number Vertical Divisions" },
	{ ConfigKey::SplWeightFreq,                    sigrok::ConfigKey::SPL_WEIGHT_FREQ,                    "SPL-Weight Frequency" },
	{ ConfigKey::SplWeightTime,                    sigrok::ConfigKey::SPL_WEIGHT_TIME,                    "SPL-Weight Time" },
	{ ConfigKey::SplMeasurementRange,              sigrok::ConfigKey::SPL_MEASUREMENT_RANGE,              "SPL Measurement Range" },
	{ ConfigKey::HoldMax,                          sigrok::ConfigKey::HOLD_MAX,                           "Hold Max" },
	{ ConfigKey::HoldMin,                          sigrok::ConfigKey::HOLD_MIN,                           "Hold Min" },
	{ ConfigKey::VoltageThreshold,                 sigrok::ConfigKey::VOLTAGE_THRESHOLD,                  "Voltage Threshold" },
	{ ConfigKey::ExternalClock,                    sigrok::ConfigKey::EXTERNAL_CLOCK,                     "External Clock" },
	{ ConfigKey::Swap,                             sigrok::ConfigKey::SWAP,                               "Swap" },
	{ ConfigKey::CenterFrequency,                  sigrok::ConfigKey::CENTER_FREQUENCY,                   "Center Frequency" },
	{ ConfigKey::NumLogicChannels,                 sigrok::ConfigKey::NUM_LOGIC_CHANNELS,                 "Number of Logic Channels" },
	{ ConfigKey::NumAnalogChannels,                sigrok::ConfigKey::NUM_ANALOG_CHANNELS,                "Number of Analog Channels" },
	{ ConfigKey::Voltage,                          sigrok::ConfigKey::VOLTAGE,                            "Voltage" },
	{ ConfigKey::VoltageTarget,                    sigrok::ConfigKey::VOLTAGE_TARGET,                     "Voltage Target" },
	{ ConfigKey::Current,                          sigrok::ConfigKey::CURRENT,                            "Current" },
	{ ConfigKey::CurrentLimit,                     sigrok::ConfigKey::CURRENT_LIMIT,                      "Current Limit" },
	{ ConfigKey::Enabled,                          sigrok::ConfigKey::ENABLED,                            "Enabled" },
	{ ConfigKey::ChannelConfig,                    sigrok::ConfigKey::CHANNEL_CONFIG,                     "ChannelConfig" },
	{ ConfigKey::OverVoltageProtectionEnabled,     sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ENABLED,    "Over Voltage Protection Enabled" },
	{ ConfigKey::OverVoltageProtectionActive,      sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ACTIVE,     "Over Voltage Protection Active" },
	{ ConfigKey::OverVoltageProtectionThreshold,   sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_THRESHOLD,  "Over Voltage Protection Threshold" },
	{ ConfigKey::OverCurrentProtectionEnabled,     sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ENABLED,    "Over Current Protection Enabled" },
	{ ConfigKey::OverCurrentProtectionActive,      sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ACTIVE,     "Over Current Protection Active" },
	{ ConfigKey::OverCurrentProtectionThreshold,   sigrok::ConfigKey::OVER_CURRENT_PROTECTION_THRESHOLD,  "Over Current Protection Threshold" },
	{ ConfigKey::OverTemperatureProtectionEnabled, sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION,        "Over Temperature Protection Enabled" },
	{ ConfigKey::OverTemperatureProtectionActive,  sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION_ACTIVE, "Over Temperature Protection Active" },
	{ ConfigKey::UnderVoltageConditionEnabled,     sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION,            "Under Voltage Condition Enabled" },
	{ ConfigKey::UnderVoltageConditionActive,      sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_ACTIVE,     "Under Voltage Condition Active" },
	{ ConfigKey::UnderVoltageConditionThreshold,   sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD,  "Under Voltage Condition Threshold" },
	{ ConfigKey::ClockEdge,                        sigrok::ConfigKey::CLOCK_EDGE,                         "Clock Edge" },
	{ ConfigKey::Amplitude,                        sigrok::ConfigKey::AMPLITUDE,                          "Amplitude" },
	{ ConfigKey::Regulation,                       sigrok::ConfigKey::REGULATION,                         "Regulation" },
	{ ConfigKey::OutputFrequency,                  sigrok::ConfigKey::OUTPUT_FREQUENCY,                   "Output Frequency" },
	{ ConfigKey::OutputFrequencyTarget,            sigrok::ConfigKey::OUTPUT_FREQUENCY_TARGET,            "Output Frequency Target" },
	{ ConfigKey::MeasuredQuantity,                 sigrok::ConfigKey::MEASURED_QUANTITY,                  "Measured Quantity" },
	{ ConfigKey::EquivCircuitModel,                sigrok::ConfigKey::EQUIV_CIRCUIT_MODEL,                "Equivalent Circuit Model" },
	{ ConfigKey::TriggerLevel,                     sigrok::ConfigKey::TRIGGER_LEVEL,                      "Trigger Level" },
	{ ConfigKey::ExternalClockSource,              sigrok::ConfigKey::EXTERNAL_CLOCK_SOURCE,              "External Clock Source" },
	{ ConfigKey::Offset,                           sigrok::ConfigKey::OFFSET,                             "Offset" },
	{ ConfigKey::TriggerPattern,                   sigrok::ConfigKey::TRIGGER_PATTERN,                    "Trigger Pattern" },
	{ ConfigKey::HighResolution,                   sigrok::ConfigKey::HIGH_RESOLUTION,                    "High Resolution" },
	{ ConfigKey::PeakDetection,                    sigrok::ConfigKey::PEAK_DETECTION,                     "Peak Detection" },
	{ ConfigKey::LogicThreshold,                   sigrok::ConfigKey::LOGIC_THRESHOLD,                    "Logic Threshold" },
	{ ConfigKey::LogicThresholdCustom,             sigrok::ConfigKey::LOGIC_THRESHOLD_CUSTOM,             "Logic Threshold Custom" },
	{ ConfigKey::Range,                            sigrok::ConfigKey::RANGE,                              "Range" },
	{ ConfigKey::Digits,                           sigrok::ConfigKey::DIGITS,                             "Digits" },
	{ ConfigKey::SessionFile,                      sigrok::ConfigKey::SESSIONFILE,                        "Session File" },
	{ ConfigKey::CaptureFile,                      sigrok::ConfigKey::CAPTUREFILE,                        "Capture File" },
	{ ConfigKey::CaptureUnitSize,                  sigrok::ConfigKey::CAPTURE_UNITSIZE,                   "Capture Unit Size" },
	{ ConfigKey::PowerOff,                         sigrok::ConfigKey::POWER_OFF,                          "Power Off" },
	{ ConfigKey::DataSource,                       sigrok::ConfigKey::DATA_SOURCE,                        "Data Source" },
	{ ConfigKey::ProbeFactor,                      sigrok::ConfigKey::PROBE_FACTOR,                       "Probe Factor" },
	{ ConfigKey::ADCPowerlineCycles,               sigrok::ConfigKey::ADC_POWERLINE_CYCLES,               "ADC Powerline Cycles" },
	{ ConfigKey::DataLog,                          sigrok::ConfigKey::DATALOG,                            "Data Log" },
	{ ConfigKey::DeviceMode,                       sigrok::ConfigKey::DEVICE_MODE,                        "Device Mode" },
	{ ConfigKey::TestMode,                         sigrok::ConfigKey::TEST_MODE,                          "Test Mode" },
	{ ConfigKey::Unknown,                          nullptr,                                               "Unknown" },
};

const vector<DeviceTypeMapping> &get_device_type_map()
{
	return device_type_map;
}

const vector<ConnectionKeyMapping> &get_connection_key_map()
{
	return connection_key_map;
}

const vector<ConfigKeyMapping> &get_config_key_map()
{
	return config_key_map;
}

bool is_supported_device(DeviceType device_type)
{
	return is_source_sink_device(device_type)
		|| is_measurement_device(device_type)
		|| is_demo_device(device_type)
		|| is_oscilloscope_device(device_type);
}

bool is_supported_driver(shared_ptr<sigrok::Driver> sr_driver)
{
	assert(sr_driver);

	const auto &device_types = get_device_types(sr_driver);
	return std::any_of(
		device_types.begin(), device_types.end(), is_supported_device);
}

bool is_source_sink_device(DeviceType device_type)
{
	return device_type == DeviceType::PowerSupply
		|| device_type == DeviceType::ElectronicLoad;
}

bool is_source_sink_driver(shared_ptr<sigrok::Driver> sr_driver)
{
	assert(sr_driver);

	const auto &device_types = get_device_types(sr_driver);
	return std::any_of(
		device_types.begin(), device_types.end(), is_source_sink_device);
}

bool is_measurement_device(DeviceType device_type)
{
	return device_type == DeviceType::Multimeter
		|| device_type == DeviceType::SoundLevelMeter
		|| device_type == DeviceType::Thermometer
		|| device_type == DeviceType::Hygrometer
		|| device_type == DeviceType::Energymeter
		|| device_type == DeviceType::LcrMeter
		|| device_type == DeviceType::Scale
		|| device_type == DeviceType::SignalGenerator
		|| device_type == DeviceType::Powermeter
		|| device_type == DeviceType::Multiplexer
		|| device_type == DeviceType::DemoDev;
}

bool is_measurement_driver(shared_ptr<sigrok::Driver> sr_driver)
{
	assert(sr_driver);

	const auto &device_types = get_device_types(sr_driver);
	return std::any_of(
		device_types.begin(), device_types.end(), is_measurement_device);
}

bool is_demo_device(DeviceType device_type)
{
	return device_type == DeviceType::DemoDev;
}

bool is_demo_driver(shared_ptr<sigrok::Driver> sr_driver)
{
	assert(sr_driver);

	const auto &device_types = get_device_types(sr_driver);
	return std::any_of(
		device_types.begin(), device_types.end(), is_demo_device);
}

bool is_oscilloscope_device(DeviceType device_type)
{
	return device_type == DeviceType::Oscilloscope;
}

bool is_oscilloscope_driver(shared_ptr<sigrok::Driver> sr_driver)
{
	assert(sr_driver);

	const auto &device_types = get_device_types(sr_driver);
	return std::any_of(
		device_types.begin(), device_types.end(), is_oscilloscope_device);
}

bool is_user_device(DeviceType device_type)
{
	return device_type == DeviceType::UserDevice;
}

bool is_user_driver(shared_ptr<sigrok::Driver> sr_driver)
{
	assert(sr_driver);

	const auto &device_types = get_device_types(sr_driver);
	return std::any_of(
		device_types.begin(), device_types.end(), is_user_device);
}

DeviceType get_device_type(const sigrok::ConfigKey *sr_config_key)
{
	for (const auto &entry : device_type_map)
		if (entry.sr_config_key == sr_config_key)
			return entry.device_type;
	return DeviceType::Unknown;
}

DeviceType get_device_type(uint32_t sr_config_key)
{
	const sigrok::ConfigKey *sr_ck = sigrok::ConfigKey::get(
		static_cast<int>(sr_config_key));
	return get_device_type(sr_ck);
}

set<DeviceType> get_device_types(shared_ptr<sigrok::Driver> sr_driver)
{
	set<DeviceType> device_types;
	for (const auto &sr_config_key : sr_driver->config_keys()) {
		const DeviceType device_type = get_device_type(sr_config_key);
		if (device_type != DeviceType::Unknown)
			device_types.insert(device_type);
	}
	return device_types;
}

const sigrok::ConfigKey *get_sr_config_key(DeviceType device_type)
{
	for (const auto &entry : device_type_map)
		if (entry.device_type == device_type)
			return entry.sr_config_key;
	return nullptr;
}

uint32_t get_sr_config_key_id(DeviceType device_type)
{
	const sigrok::ConfigKey *sr_ck = get_sr_config_key(device_type);
	if (sr_ck != nullptr)
		return sr_ck->id();
	return 0;
}

bool is_valid_sr_config_key(DeviceType device_type)
{
	for (const auto &entry : device_type_map)
		if (entry.device_type == device_type)
			return entry.sr_config_key != nullptr;
	return false;
}


ConnectionKey get_connection_key(const sigrok::ConfigKey *sr_config_key)
{
	for (const auto &entry : connection_key_map)
		if (entry.sr_config_key == sr_config_key)
			return entry.connection_key;
	return ConnectionKey::Unknown;
}

ConnectionKey get_connection_key(uint32_t sr_config_key)
{
	const sigrok::ConfigKey *sr_ck = sigrok::ConfigKey::get(
		static_cast<int>(sr_config_key));
	return get_connection_key(sr_ck);
}

const sigrok::ConfigKey *get_sr_config_key(ConnectionKey connection_key)
{
	for (const auto &entry : connection_key_map)
		if (entry.connection_key == connection_key)
			return entry.sr_config_key;
	return nullptr;
}

uint32_t get_sr_config_key_id(ConnectionKey connection_key)
{
	const sigrok::ConfigKey *sr_ck = get_sr_config_key(connection_key);
	if (sr_ck != nullptr)
		return sr_ck->id();
	return 0;
}

bool is_valid_sr_config_key(ConnectionKey connection_key)
{
	for (const auto &entry : connection_key_map)
		if (entry.connection_key == connection_key)
			return entry.sr_config_key != nullptr;
	return false;
}


ConfigKey get_config_key(const sigrok::ConfigKey *sr_config_key)
{
	for (const auto &entry : config_key_map)
		if (entry.sr_config_key == sr_config_key)
			return entry.config_key;
	return ConfigKey::Unknown;
}

ConfigKey get_config_key(uint32_t sr_config_key)
{
	const sigrok::ConfigKey *sr_ck = sigrok::ConfigKey::get(
		static_cast<int>(sr_config_key));
	return get_config_key(sr_ck);
}

const sigrok::ConfigKey *get_sr_config_key(ConfigKey config_key)
{
	for (const auto &entry : config_key_map)
		if (entry.config_key == config_key)
			return entry.sr_config_key;
	return nullptr;
}

uint32_t get_sr_config_key_id(ConfigKey config_key)
{
	const sigrok::ConfigKey *sr_ck = get_sr_config_key(config_key);
	if (sr_ck != nullptr)
		return sr_ck->id();
	return 0;
}

bool is_valid_sr_config_key(ConfigKey config_key)
{
	for (const auto &entry : config_key_map)
		if (entry.config_key == config_key)
			return entry.sr_config_key != nullptr;
	return false;
}


QString format_device_type(DeviceType device_type)
{
	for (const auto &entry : device_type_map)
		if (entry.device_type == device_type)
			return entry.name;

	return format_device_type(DeviceType::Unknown);
}

QString format_connection_key(ConnectionKey connection_key)
{
	for (const auto &entry : connection_key_map)
		if (entry.connection_key == connection_key)
			return entry.name;

	return format_connection_key(ConnectionKey::Unknown);
}

QString format_config_key(ConfigKey config_key)
{
	for (const auto &entry : config_key_map)
		if (entry.config_key == config_key)
			return entry.name;

	return format_config_key(ConfigKey::Unknown);
}


data::DataType get_data_type_for_config_key(ConfigKey config_key)
{
	const sigrok::ConfigKey *sr_ck = get_sr_config_key(config_key);
	if (!sr_ck)
		return data::DataType::Unknown;

	return data::datautil::get_data_type(sr_ck->data_type());
}

data::Unit get_unit_for_config_key(ConfigKey config_key)
{
	if (config_key_unit_map.count(config_key) > 0)
		return config_key_unit_map[config_key];
	return data::Unit::Unknown;
}

} // namespace deviceutil
} // namespace devices
} // namespace sv
