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

#ifndef DEVICES_DEVICEUTIL_HPP
#define DEVICES_DEVICEUTIL_HPP

#include <map>
#include <memory>
#include <set>
#include <vector>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QObject>
#include <QString>
#include <QStringLiteral>

#include "src/data/datautil.hpp"

using std::map;
using std::set;
using std::shared_ptr;
using std::vector;

namespace sigrok {
class ConfigKey;
}

namespace sv {
namespace devices {

enum class DeviceType
{
	/** Any device */
	Any,
	/** The device can act as logic analyzer. */
	LogicAnalyzer,
	/** The device can act as an oscilloscope. */
	Oscilloscope,
	/** The device can act as a multimeter. */
	Multimeter,
	/** The device is a demo device. */
	DemoDev,
	/** The device can act as a sound level meter. */
	SoundLevelMeter,
	/** The device can measure temperature. */
	Thermometer,
	/** The device can measure humidity. */
	Hygrometer,
	/** The device can measure energy consumption. */
	Energymeter,
	/** The device can act as a signal demodulator. */
	Demodulator,
	/** The device can act as a programmable power supply. */
	PowerSupply,
	/** The device can act as an LCR meter. */
	LcrMeter,
	/** The device can act as an electronic load. */
	ElectronicLoad,
	/** The device can act as a scale. */
	Scale,
	/** The device can act as a function generator. */
	SignalGenerator,
	/** The device can measure power. */
	Powermeter,
	/** A Multiplexer / Relay Actuator device. */
	Multiplexer,
	/** User device */
	UserDevice,
	/** Unknown device. */
	Unknown,
};

enum class ConnectionKey
{
	/** Specification on how to connect to a device. */
	Conn,
	/** Serial communication specification, in the form: */
	SerialComm,
	/** Modbus slave address specification. */
	ModbusAddr,
	/** The device supports setting a sample time limit (how long the sample acquisition should run, in ms). */
	LimitMsec,
	/** The device supports setting a sample number limit (how many samples should be acquired). */
	LimitSamples,
	/** The device supports setting a frame limit (how many frames should be acquired). */
	LimitFrames,
	/** The device supports continuous sampling. */
	Continuous,
	/** Unknown connection key. */
	Unknown,
};

enum class ConfigKey
{
	/** The device supports setting its samplerate, in Hz. */
	Samplerate,
	/** The device supports setting a pre/post-trigger capture ratio. */
	CaptureRatio,
	/** The device supports setting a pattern (pattern generator mode). */
	PatternMode,
	/** The device supports run-length encoding (RLE). */
	RLE,
	/** The device supports setting trigger slope. */
	TriggerSlope,
	/** The device supports averaging. */
	Averaging,
	/** The device supports setting number of samples to be averaged over. */
	AvgSamples,
	/** Trigger source. */
	TriggerSource,
	/** Horizontal trigger position. */
	HorizTriggerPos,
	/** Buffer size. */
	BufferSize,
	/** Time base. */
	TimeBase,
	/** Filter. */
	Filter,
	/** Volts/div. */
	VDiv,
	/** Coupling. */
	Coupling,
	/** Trigger matches. */
	TriggerMatch,
	/** The device supports setting its sample interval, in ms. */
	SampleInterval,
	/** Number of horizontal divisions, as related to SR_CONF_TIMEBASE. */
	NumHDiv,
	/** Number of vertical divisions, as related to SR_CONF_VDIV. */
	NumVDiv,
	/** Sound pressure level frequency weighting. */
	SplWeightFreq,
	/** Sound pressure level time weighting. */
	SplWeightTime,
	/** Sound pressure level measurement range. */
	SplMeasurementRange,
	/** Max hold mode. */
	HoldMax,
	/** Min hold mode. */
	HoldMin,
	/** Logic low-high threshold range. */
	VoltageThreshold,
	/** The device supports using an external clock. */
	ExternalClock,
	/** The device supports swapping channels. */
	Swap,
	/** Center frequency. */
	CenterFrequency,
	/** The device supports setting the number of logic channels. */
	NumLogicChannels,
	/** The device supports setting the number of analog channels. */
	NumAnalogChannels,
	/** Current voltage. */
	Voltage,
	/** Maximum target voltage. */
	VoltageTarget,
	/** Current current. */
	Current,
	/** Current limit. */
	CurrentLimit,
	/** Enabling/disabling channel. */
	Enabled,
	/** Channel configuration. */
	ChannelConfig,
	/** Over-voltage protection (OVP) feature. */
	OverVoltageProtectionEnabled,
	/** Over-voltage protection (OVP) active. */
	OverVoltageProtectionActive,
	/** Over-voltage protection (OVP) threshold. */
	OverVoltageProtectionThreshold,
	/** Over-current protection (OCP) feature. */
	OverCurrentProtectionEnabled,
	/** Over-current protection (OCP) active. */
	OverCurrentProtectionActive,
	/** Over-current protection (OCP) threshold. */
	OverCurrentProtectionThreshold,
	/** Over-temperature protection (OTP) */
	OverTemperatureProtectionEnabled,
	/** Over-temperature protection (OTP) active. */
	OverTemperatureProtectionActive,
	/** Under-voltage condition. */
	UnderVoltageConditionEnabled,
	/** Under-voltage condition active. */
	UnderVoltageConditionActive,
	/** Under-voltage condition threshold. */
	UnderVoltageConditionThreshold,
	/** Choice of clock edge for external clock ("r" or "f"). */
	ClockEdge,
	/** Amplitude of a source without strictly-defined MQ. */
	Amplitude,
	/** Channel regulation get: "CV", "CC" or "UR", denoting constant voltage, constant current or unregulated. */
	Regulation,
	/** Output frequency in Hz. */
	OutputFrequency,
	/** Output frequency target in Hz. */
	OutputFrequencyTarget,
	/** Measured quantity. */
	MeasuredQuantity,
	/** Equivalent circuit model. */
	EquivCircuitModel,
	/** Trigger level. */
	TriggerLevel,
	/** Which external clock source to use if the device supports multiple external clock channels. */
	ExternalClockSource,
	/** Offset of a source without strictly-defined MQ. */
	Offset,
	/** The device supports setting a pattern for the logic trigger. */
	TriggerPattern,
	/** High resolution mode. */
	HighResolution,
	/** Peak detection. */
	PeakDetection,
	/** Logic threshold: predefined levels (TTL, ECL, CMOS, etc). */
	LogicThreshold,
	/** Logic threshold: custom numerical value. */
	LogicThresholdCustom,
	/** The measurement range of a DMM or the output range of a power supply. */
	Range,
	/** The number of digits (e.g. for a DMM). */
	Digits,

	/** Session filename. */
	SessionFile,
	/** The device supports specifying a capturefile to inject. */
	CaptureFile,
	/** The device supports specifying the capturefile unit size. */
	CaptureUnitSize,
	/** Power off the device. */
	PowerOff,
	/** Data source for acquisition. */
	DataSource,
	/** The device supports setting a probe factor. */
	ProbeFactor,
	/** Number of powerline cycles for ADC integration time. */
	ADCPowerlineCycles,
	/** The device has internal storage, into which data is logged. */
	DataLog,
	/** Device mode for multi-function devices. */
	DeviceMode,
	/** Self test mode. */
	TestMode,
	/** Unknown config key. */
	Unknown,
};

namespace deviceutil {

struct DeviceTypeMapping
{
	DeviceType device_type;
	const sigrok::ConfigKey *sr_config_key;
	const QString name;
};

struct ConnectionKeyMapping
{
	ConnectionKey connection_key;
	const sigrok::ConfigKey *sr_config_key;
	const QString name;
};

struct ConfigKeyMapping
{
	ConfigKey config_key;
	const sigrok::ConfigKey *sr_config_key;
	const QString name;
};

namespace {



/**
 * TODO: Find a better way get the Unit/Q/QF from the ConfigKey.
 * Implement in libsr: Add analog.meaning, etc. to the config_key structure.
 */
map<ConfigKey, data::Unit> config_key_unit_map = {
	{ ConfigKey::Samplerate, data::Unit::Hertz },
	{ ConfigKey::CaptureRatio, data::Unit::Unitless },
	{ ConfigKey::PatternMode, data::Unit::Unitless },
	{ ConfigKey::RLE, data::Unit::Boolean },
	{ ConfigKey::TriggerSlope, data::Unit::Unitless },
	{ ConfigKey::Averaging, data::Unit::Boolean },
	{ ConfigKey::AvgSamples, data::Unit::Unitless },
	{ ConfigKey::TriggerSource, data::Unit::Unitless },
	{ ConfigKey::HorizTriggerPos, data::Unit::Second },
	{ ConfigKey::BufferSize, data::Unit::Unitless }, // TODO Byte, Samples or Points!
	{ ConfigKey::TimeBase, data::Unit::Second },
	{ ConfigKey::Filter, data::Unit::Boolean },
	{ ConfigKey::VDiv, data::Unit::Volt },
	{ ConfigKey::Coupling, data::Unit::Unitless },
	{ ConfigKey::TriggerMatch, data::Unit::Unknown },
	{ ConfigKey::SampleInterval, data::Unit::Second },
	{ ConfigKey::NumHDiv, data::Unit::Unitless },
	{ ConfigKey::NumVDiv, data::Unit::Unitless },
	{ ConfigKey::SplWeightFreq, data::Unit::Unitless },
	{ ConfigKey::SplWeightTime, data::Unit::Unitless },
	{ ConfigKey::SplMeasurementRange, data::Unit::Unknown },
	{ ConfigKey::HoldMax, data::Unit::Boolean },
	{ ConfigKey::HoldMin, data::Unit::Boolean },
	{ ConfigKey::VoltageThreshold, data::Unit::Volt },
	{ ConfigKey::ExternalClock, data::Unit::Boolean },
	{ ConfigKey::Swap, data::Unit::Boolean },
	{ ConfigKey::CenterFrequency, data::Unit::Hertz },
	{ ConfigKey::NumLogicChannels, data::Unit::Unitless },
	{ ConfigKey::NumAnalogChannels, data::Unit::Unitless },
	{ ConfigKey::Voltage, data::Unit::Volt },
	{ ConfigKey::VoltageTarget, data::Unit::Volt },
	{ ConfigKey::Current, data::Unit::Ampere },
	{ ConfigKey::CurrentLimit, data::Unit::Ampere },
	{ ConfigKey::Enabled, data::Unit::Boolean },
	{ ConfigKey::ChannelConfig, data::Unit::Unitless },
	{ ConfigKey::OverVoltageProtectionEnabled, data::Unit::Boolean },
	{ ConfigKey::OverVoltageProtectionActive, data::Unit::Boolean },
	{ ConfigKey::OverVoltageProtectionThreshold, data::Unit::Volt },
	{ ConfigKey::OverCurrentProtectionEnabled, data::Unit::Boolean },
	{ ConfigKey::OverCurrentProtectionActive, data::Unit::Boolean },
	{ ConfigKey::OverCurrentProtectionThreshold, data::Unit::Ampere },
	{ ConfigKey::OverTemperatureProtectionEnabled, data::Unit::Boolean },
	{ ConfigKey::OverTemperatureProtectionActive, data::Unit::Boolean },
	{ ConfigKey::UnderVoltageConditionEnabled, data::Unit::Boolean },
	{ ConfigKey::UnderVoltageConditionActive, data::Unit::Boolean },
	{ ConfigKey::UnderVoltageConditionThreshold, data::Unit::Volt },
	{ ConfigKey::ClockEdge, data::Unit::Unitless },
	{ ConfigKey::Amplitude, data::Unit::Unknown },
	{ ConfigKey::Regulation, data::Unit::Unitless },
	{ ConfigKey::OutputFrequency, data::Unit::Hertz },
	{ ConfigKey::OutputFrequencyTarget, data::Unit::Hertz },
	{ ConfigKey::MeasuredQuantity, data::Unit::Unitless },
	{ ConfigKey::EquivCircuitModel, data::Unit::Unitless },
	{ ConfigKey::TriggerLevel, data::Unit::Volt },
	{ ConfigKey::ExternalClockSource, data::Unit::Unitless },
	{ ConfigKey::Offset, data::Unit::Unknown },
	{ ConfigKey::TriggerPattern, data::Unit::Unitless },
	{ ConfigKey::HighResolution, data::Unit::Unitless },
	{ ConfigKey::PeakDetection, data::Unit::Unitless },
	{ ConfigKey::LogicThreshold, data::Unit::Unitless },
	{ ConfigKey::LogicThresholdCustom, data::Unit::Volt },
	{ ConfigKey::Range, data::Unit::Unitless },
	{ ConfigKey::Digits, data::Unit::Unitless },
	{ ConfigKey::SessionFile, data::Unit::Unitless },
	{ ConfigKey::CaptureFile, data::Unit::Unitless },
	{ ConfigKey::CaptureUnitSize, data::Unit::Unknown },
	{ ConfigKey::PowerOff, data::Unit::Boolean },
	{ ConfigKey::DataSource, data::Unit::Unitless },
	{ ConfigKey::ProbeFactor, data::Unit::Unitless },
	{ ConfigKey::ADCPowerlineCycles, data::Unit::Unitless },
	{ ConfigKey::DataLog, data::Unit::Boolean },
	{ ConfigKey::DeviceMode, data::Unit::Unitless },
	{ ConfigKey::TestMode, data::Unit::Unitless },
};

} // namespace

/**
 * Return all known device types
 *
 * @return The device type map
 */
const vector<DeviceTypeMapping> &get_device_type_map();

/**
 * Return all known connection keys
 *
 * @return The connection key map
 */
const vector<ConnectionKeyMapping> &get_connection_key_map();

/**
 * Return all known config keys
 *
 * @return The config key map
 */
const vector<ConfigKeyMapping> &get_config_key_map();

/**
 * Check if the device type is supported by SmuView.
 *
 * @param device_type The type to check.
 *
 * @return true, if the device is supported.
 */
bool is_supported_device(DeviceType device_type);

/**
 * Check if the driver is supported by SmuView.
 *
 * @param sr_driver The sigrok Driver to check.
 *
 * @return true, if the driver is supported.
 */
bool is_supported_driver(shared_ptr<sigrok::Driver> sr_driver);

/**
 * Check if the device type is a power supply or a electronic load.
 *
 * @param device_type The device type to check.
 *
 * @return true, if the device type is a power supply or a electronic load.
 */
bool is_source_sink_device(DeviceType device_type);

/**
 * Check if the driver is a power supply or a electronic load.
 *
 * @param sr_driver The sigrok Driver to check.
 *
 * @return true, if the driver is a power supply or a electronic load.
 */
bool is_source_sink_driver(shared_ptr<sigrok::Driver> sr_driver);

/**
 * Check if the device type is a measurement device (dmm, lcr meter, ...).
 *
 * @param device_type The device type to check.
 *
 * @return true, if the device type is a measurement device.
 */
bool is_measurement_device(DeviceType device_type);

/**
 * Check if the driver is a measurement device (dmm, lcr meter, ...).
 *
 * @param sr_driver The sigrok Driver to check.
 *
 * @return true, if the driver is a measurement device.
 */
bool is_measurement_driver(shared_ptr<sigrok::Driver> sr_driver);

/**
 * Check if the device type is a demo device.
 *
 * @param device_type The device type to check.
 *
 * @return true, if the device type is a demo device.
 */
bool is_demo_device(DeviceType device_type);

/**
 * Check if the driver is a demo device.
 *
 * @param sr_driver The sigrok Driver to check.
 *
 * @return true, if the driver is a demo device.
 */
bool is_demo_driver(shared_ptr<sigrok::Driver> sr_driver);

/**
 * Check if the device type is an oscilloscope.
 *
 * @param device_type The device type to check.
 *
 * @return true, if the device type is an oscilloscope.
 */
bool is_oscilloscope_device(DeviceType device_type);

/**
 * Check if the driver is an oscilloscope.
 *
 * @param sr_driver The sigrok Driver to check.
 *
 * @return true, if the driver is an oscilloscope.
 */
bool is_oscilloscope_driver(shared_ptr<sigrok::Driver> sr_driver);

/**
 * Check if the device type is an user device.
 *
 * @param device_type The device type to check.
 *
 * @return true, if the device type is an user device.
 */
bool is_user_device(DeviceType device_type);

/**
 * Check if the driver is an user device.
 *
 * @param sr_driver The sigrok Driver to check.
 *
 * @return true, if the driver is an user device.
 */
bool is_user_driver(shared_ptr<sigrok::Driver> sr_driver);

/**
 * Return the corresponding DeviceType for a sigrok ConfigKey
 *
 * @param sr_config_key The sigrok ConfigKey
 *
 * @return The DeviceType.
 */
DeviceType get_device_type(const sigrok::ConfigKey *sr_config_key);

/**
 * Return the corresponding DeviceType for a sigrok ConfigKey (unit32_t)
 *
 * @param sr_config_key The sigrok ConfigKey as uint32_t
 *
 * @return The DeviceType.
 */
DeviceType get_device_type(uint32_t sr_config_key);

/**
 * Return all device types for a sigrok driver instance
 *
 * @param sr_driver The sigrok driver
 *
 * @return A set of device types
 */
set<DeviceType> get_device_types(shared_ptr<sigrok::Driver> sr_driver);

/**
 * Return the corresponding sigrok ConfigKey for a DeviceType
 *
 * @param device_type The DeviceType.
 *
 * @return The sigrok ConfigKeyt.
 */
const sigrok::ConfigKey *get_sr_config_key(DeviceType device_type);

/**
 * Return the corresponding sigrok ConfigKey ID for a DeviceType
 *
 * @param device_type The DeviceType
 *
 * @return The sigrok ConfigKey ID as uint32_t.
 */
uint32_t get_sr_config_key_id(DeviceType device_type);

/**
 * Check if the DeviceType is a known sigrok DeviceType / ConfigKey
 *
 * @param device_type The DeviceType
 *
 * @return true if it is a known sigrok DeviceType / ConfigKey
 */
bool is_valid_sr_config_key(DeviceType device_type);


/**
 * Return the corresponding ConnectionKey for a sigrok ConfigKey
 *
 * @param sr_config_key The sigrok ConfigKey
 *
 * @return The ConnectionKey.
 */
ConnectionKey get_connection_key(const sigrok::ConfigKey *sr_config_key);

/**
 * Return the corresponding ConnectionKey for a sigrok ConfigKey (unit32_t)
 *
 * @param sr_config_key The sigrok ConfigKey as uint32_t
 *
 * @return The ConnectionKey.
 */
ConnectionKey get_connection_key(uint32_t sr_config_key);

/**
 * Return the corresponding sigrok ConfigKey for a ConnectionKey
 *
 * @param connection_key The ConnectionKey.
 *
 * @return The sigrok ConfigKey.
 */
const sigrok::ConfigKey *get_sr_config_key(ConnectionKey connection_key);

/**
 * Return the corresponding sigrok ConfigKey ID for a ConnectionKey
 *
 * @param connection_key The ConnectionKey
 *
 * @return The sigrok ConfigKey ID as uint32_t.
 */
uint32_t get_sr_config_key_id(ConnectionKey connection_key);

/**
 * Check if the ConnectionKey is a known sigrok ConnectionKey / ConfigKey
 *
 * @param connection_key The ConnectionKey
 *
 * @return true if it is a known sigrok ConnectionKey / ConfigKey
 */
bool is_valid_sr_config_key(ConnectionKey connection_key);


/**
 * Return the corresponding ConfigKey for a sigrok ConfigKey
 *
 * @param sr_config_key The sigrok ConfigKey
 *
 * @return The ConfigKey.
 */
ConfigKey get_config_key(const sigrok::ConfigKey *sr_config_key);

/**
 * Return the corresponding ConfigKey for a sigrok ConfigKey (unit32_t)
 *
 * @param sr_config_key The sigrok ConfigKey as uint32_t
 *
 * @return The ConfigKey.
 */
ConfigKey get_config_key(uint32_t sr_config_key);

/**
 * Return the corresponding sigrok ConfigKey for a ConfigKey
 *
 * @param config_key The ConfigKey.
 *
 * @return The sigrok ConfigKey.
 */
const sigrok::ConfigKey *get_sr_config_key(ConfigKey config_key);

/**
 * Return the corresponding sigrok ConfigKey ID for a ConfigKey
 *
 * @param config_key The ConfigKey
 *
 * @return The sigrok ConfigKey ID as uint32_t.
 */
uint32_t get_sr_config_key_id(ConfigKey config_key);

/**
 * Check if the ConfigKey is a known sigrok ConfigKey
 *
 * @param config_key The ConfigKey
 *
 * @return true if it is a known sigrok ConfigKey
 */
bool is_valid_sr_config_key(ConfigKey config_key);


/**
 * Format a DeviceType to a string
 *
 * @param device_type The DeviceType to format.
 *
 * @return The formatted device type.
 */
QString format_device_type(DeviceType device_type);

/**
 * Format a ConnectionKey to a string
 *
 * @param connection_key The ConnectionKey to format.
 *
 * @return The formatted ConnectionKey.
 */
QString format_connection_key(ConnectionKey connection_key);

/**
 * Format a ConfigKey to a string
 *
 * @param config_key The ConfigKey to format.
 *
 * @return The formatted ConfigKey.
 */
QString format_config_key(ConfigKey config_key);


/**
 * Get the DataType for a ConfigKey
 *
 * @param config_key The ConfigKey.
 *
 * @return The DataType for the ConfigKey.
 */
data::DataType get_data_type_for_config_key(ConfigKey config_key);

/**
 * Get the Unit for a ConfigKey
 *
 * @param config_key The ConfigKey.
 *
 * @return The Unit for the ConfigKey.
 */
data::Unit get_unit_for_config_key(ConfigKey config_key);

} // namespace deviceutil
} // namespace devices
} // namespace sv

#endif // DEVICES_DEVICEUTIL_HPP
