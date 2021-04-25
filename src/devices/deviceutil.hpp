/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2021 Frank Stettner <frank-stettner@gmx.net>
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

typedef map<DeviceType, QString> device_type_name_map_t;
typedef map<ConnectionKey, QString> connection_key_name_map_t;
typedef map<ConfigKey, QString> config_key_name_map_t;

namespace {

// TODO: Use tr(), QCoreApplication::translate(), QT_TR_NOOP() or
//       QT_TRANSLATE_NOOP() for translation.
//       See: http://doc.qt.io/qt-5/i18n-source-translation.html
device_type_name_map_t device_type_name_map = {
	{ DeviceType::LogicAnalyzer, QString("Logic Analyzer") },
	{ DeviceType::Oscilloscope, QString("Oscilloscope") },
	{ DeviceType::Multimeter, QString("Multimeter") },
	{ DeviceType::DemoDev, QString("Demo Device") },
	{ DeviceType::SoundLevelMeter, QString("Soundlevelmeter") },
	{ DeviceType::Thermometer, QString("Thermometer") },
	{ DeviceType::Hygrometer, QString("Hygrometer") },
	{ DeviceType::Energymeter, QString("Energymeter") },
	{ DeviceType::Demodulator, QString("Demodulator") },
	{ DeviceType::PowerSupply, QString("Power Supply") },
	{ DeviceType::LcrMeter, QString("LCR Meter") },
	{ DeviceType::ElectronicLoad, QString("Electronic Load") },
	{ DeviceType::Scale, QString("Scale") },
	{ DeviceType::SignalGenerator, QString("Signal Generator") },
	{ DeviceType::Powermeter, QString("Power Meter") },
	{ DeviceType::Multiplexer, QString("Multiplexer") },
	{ DeviceType::UserDevice, QString("Virtual User Device") },
	{ DeviceType::Unknown, QString("Unknown") },
};

connection_key_name_map_t connection_key_name_map = {
	{ ConnectionKey::Conn, QString("Connection String") },
	{ ConnectionKey::SerialComm, QString("Serial Command") },
	{ ConnectionKey::ModbusAddr, QString("ModBus Address") },
	{ ConnectionKey::LimitMsec, QString("Limit Milliseconds") },
	{ ConnectionKey::LimitSamples, QString("Limit Samples") },
	{ ConnectionKey::LimitFrames, QString("Limit Frames") },
	{ ConnectionKey::Continuous, QString("Continuous") },
	{ ConnectionKey::Unknown, QString("Unknown") },
};

config_key_name_map_t config_key_name_map = {
	{ ConfigKey::Samplerate, QString("Samplerate") },
	{ ConfigKey::CaptureRatio, QString("Capture Ratio") },
	{ ConfigKey::PatternMode, QString("Pattern Mode") },
	{ ConfigKey::RLE, QString("Run-Length Encoding") },
	{ ConfigKey::TriggerSlope, QString("Trigger Slope") },
	{ ConfigKey::Averaging, QString("Averaging") },
	{ ConfigKey::AvgSamples, QString("Averaging Samples") },
	{ ConfigKey::TriggerSource, QString("Trigger Source") },
	{ ConfigKey::HorizTriggerPos, QString("Horizonal Trigger Position") },
	{ ConfigKey::BufferSize, QString("Buffer Size") },
	{ ConfigKey::TimeBase, QString("Time Base") },
	{ ConfigKey::Filter, QString("Filter") },
	{ ConfigKey::VDiv, QString("Vertical Division") },
	{ ConfigKey::Coupling, QString("Coupling") },
	{ ConfigKey::TriggerMatch, QString("Trigger Match") },
	{ ConfigKey::SampleInterval, QString("Sample Interval") },
	{ ConfigKey::NumHDiv, QString("Number Horizontal Divisions") },
	{ ConfigKey::NumVDiv, QString("Number Vertical Divisions") },
	{ ConfigKey::SplWeightFreq, QString("SPL-Weight Frequency") },
	{ ConfigKey::SplWeightTime, QString("SPL-Weight Time") },
	{ ConfigKey::SplMeasurementRange, QString("SPL Measurement Range") },
	{ ConfigKey::HoldMax, QString("Hold Max") },
	{ ConfigKey::HoldMin, QString("Hold Min") },
	{ ConfigKey::VoltageThreshold, QString("Voltage Threshold") },
	{ ConfigKey::ExternalClock, QString("External Clock") },
	{ ConfigKey::Swap, QString("Swap") },
	{ ConfigKey::CenterFrequency, QString("Center Frequency") },
	{ ConfigKey::NumLogicChannels, QString("Number of Logic Channels") },
	{ ConfigKey::NumAnalogChannels, QString("Number of Analog Channels") },
	{ ConfigKey::Voltage, QString("Voltage") },
	{ ConfigKey::VoltageTarget, QString("Voltage Target") },
	{ ConfigKey::Current, QString("Current") },
	{ ConfigKey::CurrentLimit, QString("Current Limit") },
	{ ConfigKey::Enabled, QString("Enabled") },
	{ ConfigKey::ChannelConfig, QString("ChannelConfig") },
	{ ConfigKey::OverVoltageProtectionEnabled, QString("Over Voltage Protection Enabled") },
	{ ConfigKey::OverVoltageProtectionActive, QString("Over Voltage Protection Active") },
	{ ConfigKey::OverVoltageProtectionThreshold, QString("Over Voltage Protection Threshold") },
	{ ConfigKey::OverCurrentProtectionEnabled, QString("Over Current Protection Enabled") },
	{ ConfigKey::OverCurrentProtectionActive, QString("Over Current Protection Active") },
	{ ConfigKey::OverCurrentProtectionThreshold, QString("Over Current Protection Threshold") },
	{ ConfigKey::OverTemperatureProtectionEnabled, QString("Over Temperature Protection Enabled") },
	{ ConfigKey::OverTemperatureProtectionActive, QString("Over Temperature Protection Active") },
	{ ConfigKey::UnderVoltageConditionEnabled, QString("Under Voltage Condition Enabled") },
	{ ConfigKey::UnderVoltageConditionActive, QString("Under Voltage Condition Active") },
	{ ConfigKey::UnderVoltageConditionThreshold, QString("Under Voltage Condition Threshold") },
	{ ConfigKey::ClockEdge, QString("Clock Edge") },
	{ ConfigKey::Amplitude, QString("Amplitude") },
	{ ConfigKey::Regulation, QString("Regulation") },
	{ ConfigKey::OutputFrequency, QString("Output Frequency") },
	{ ConfigKey::OutputFrequencyTarget, QString("Output Frequency Target") },
	{ ConfigKey::MeasuredQuantity, QString("Measured Quantity") },
	{ ConfigKey::EquivCircuitModel, QString("Equivalent Circuit Model") },
	{ ConfigKey::TriggerLevel, QString("Trigger Level") },
	{ ConfigKey::ExternalClockSource, QString("External Clock Source") },
	{ ConfigKey::Offset, QString("Offset") },
	{ ConfigKey::TriggerPattern, QString("Trigger Pattern") },
	{ ConfigKey::HighResolution, QString("High Resolution") },
	{ ConfigKey::PeakDetection, QString("Peak Detection") },
	{ ConfigKey::LogicThreshold, QString("Logic Threshold") },
	{ ConfigKey::LogicThresholdCustom, QString("Logic Threshold Custom") },
	{ ConfigKey::Range, QString("Range") },
	{ ConfigKey::Digits, QString("Digits") },
	{ ConfigKey::SessionFile, QString("Session File") },
	{ ConfigKey::CaptureFile, QString("Capture File") },
	{ ConfigKey::CaptureUnitSize, QString("Capture Unit Size") },
	{ ConfigKey::PowerOff, QString("Power Off") },
	{ ConfigKey::DataSource, QString("Data Source") },
	{ ConfigKey::ProbeFactor, QString("Probe Factor") },
	{ ConfigKey::ADCPowerlineCycles, QString("ADC Powerline Cycles") },
	{ ConfigKey::DataLog, QString("Data Log") },
	{ ConfigKey::DeviceMode, QString("Device Mode") },
	{ ConfigKey::TestMode, QString("Test Mode") },
	{ ConfigKey::Unknown, QString("Unknown") },
};

map<const sigrok::ConfigKey *, DeviceType> sr_config_key_device_type_map = {
	{ sigrok::ConfigKey::LOGIC_ANALYZER, DeviceType::LogicAnalyzer },
	{ sigrok::ConfigKey::OSCILLOSCOPE, DeviceType::Oscilloscope },
	{ sigrok::ConfigKey::MULTIMETER, DeviceType::Multimeter },
	{ sigrok::ConfigKey::DEMO_DEV, DeviceType::DemoDev },
	{ sigrok::ConfigKey::SOUNDLEVELMETER, DeviceType::SoundLevelMeter },
	{ sigrok::ConfigKey::THERMOMETER, DeviceType::Thermometer },
	{ sigrok::ConfigKey::HYGROMETER, DeviceType::Hygrometer },
	{ sigrok::ConfigKey::ENERGYMETER, DeviceType::Energymeter },
	{ sigrok::ConfigKey::DEMODULATOR, DeviceType::Demodulator },
	{ sigrok::ConfigKey::POWER_SUPPLY, DeviceType::PowerSupply },
	{ sigrok::ConfigKey::LCRMETER, DeviceType::LcrMeter },
	{ sigrok::ConfigKey::ELECTRONIC_LOAD, DeviceType::ElectronicLoad },
	{ sigrok::ConfigKey::SCALE, DeviceType::Scale },
	{ sigrok::ConfigKey::SIGNAL_GENERATOR, DeviceType::SignalGenerator },
	{ sigrok::ConfigKey::POWERMETER, DeviceType::Powermeter },
	{ sigrok::ConfigKey::MULTIPLEXER, DeviceType::Multiplexer },
};

map<DeviceType, const sigrok::ConfigKey *> device_type_sr_config_key_map = {
	{ DeviceType::LogicAnalyzer, sigrok::ConfigKey::LOGIC_ANALYZER },
	{ DeviceType::Oscilloscope, sigrok::ConfigKey::OSCILLOSCOPE },
	{ DeviceType::Multimeter, sigrok::ConfigKey::MULTIMETER },
	{ DeviceType::DemoDev, sigrok::ConfigKey::DEMO_DEV },
	{ DeviceType::SoundLevelMeter, sigrok::ConfigKey::SOUNDLEVELMETER },
	{ DeviceType::Thermometer, sigrok::ConfigKey::THERMOMETER },
	{ DeviceType::Hygrometer, sigrok::ConfigKey::HYGROMETER },
	{ DeviceType::Energymeter, sigrok::ConfigKey::ENERGYMETER },
	{ DeviceType::Demodulator, sigrok::ConfigKey::DEMODULATOR },
	{ DeviceType::PowerSupply, sigrok::ConfigKey::POWER_SUPPLY },
	{ DeviceType::LcrMeter, sigrok::ConfigKey::LCRMETER },
	{ DeviceType::ElectronicLoad, sigrok::ConfigKey::ELECTRONIC_LOAD },
	{ DeviceType::Scale, sigrok::ConfigKey::SCALE },
	{ DeviceType::SignalGenerator, sigrok::ConfigKey::SIGNAL_GENERATOR },
	{ DeviceType::Powermeter, sigrok::ConfigKey::POWERMETER },
	{ DeviceType::Multiplexer, sigrok::ConfigKey::MULTIPLEXER },
};

map<const sigrok::ConfigKey *, ConnectionKey> sr_config_key_connection_key_map = {
	{ sigrok::ConfigKey::CONN, ConnectionKey::Conn },
	{ sigrok::ConfigKey::SERIALCOMM, ConnectionKey::SerialComm },
	{ sigrok::ConfigKey::MODBUSADDR, ConnectionKey::ModbusAddr },
	{ sigrok::ConfigKey::LIMIT_MSEC, ConnectionKey::LimitMsec },
	{ sigrok::ConfigKey::LIMIT_SAMPLES, ConnectionKey::LimitSamples },
	{ sigrok::ConfigKey::LIMIT_FRAMES, ConnectionKey::LimitFrames },
	{ sigrok::ConfigKey::CONTINUOUS, ConnectionKey::Continuous },
};

map<ConnectionKey, const sigrok::ConfigKey *> connection_key_sr_config_key_map = {
	{ ConnectionKey::Conn, sigrok::ConfigKey::CONN },
	{ ConnectionKey::SerialComm, sigrok::ConfigKey::SERIALCOMM },
	{ ConnectionKey::ModbusAddr, sigrok::ConfigKey::MODBUSADDR },
	{ ConnectionKey::LimitMsec, sigrok::ConfigKey::LIMIT_MSEC },
	{ ConnectionKey::LimitSamples, sigrok::ConfigKey::LIMIT_SAMPLES },
	{ ConnectionKey::LimitFrames, sigrok::ConfigKey::LIMIT_FRAMES },
	{ ConnectionKey::Continuous, sigrok::ConfigKey::CONTINUOUS },
};

map<const sigrok::ConfigKey *, ConfigKey> sr_config_key_config_key_map = {
	{ sigrok::ConfigKey::SAMPLERATE, ConfigKey::Samplerate },
	{ sigrok::ConfigKey::CAPTURE_RATIO, ConfigKey::CaptureRatio },
	{ sigrok::ConfigKey::PATTERN_MODE, ConfigKey::PatternMode },
	{ sigrok::ConfigKey::RLE, ConfigKey::RLE },
	{ sigrok::ConfigKey::TRIGGER_SLOPE, ConfigKey::TriggerSlope },
	{ sigrok::ConfigKey::AVERAGING, ConfigKey::Averaging },
	{ sigrok::ConfigKey::AVG_SAMPLES, ConfigKey::AvgSamples },
	{ sigrok::ConfigKey::TRIGGER_SOURCE, ConfigKey::TriggerSource },
	{ sigrok::ConfigKey::HORIZ_TRIGGERPOS, ConfigKey::HorizTriggerPos },
	{ sigrok::ConfigKey::BUFFERSIZE, ConfigKey::BufferSize },
	{ sigrok::ConfigKey::TIMEBASE, ConfigKey::TimeBase },
	{ sigrok::ConfigKey::FILTER, ConfigKey::Filter },
	{ sigrok::ConfigKey::VDIV, ConfigKey::VDiv },
	{ sigrok::ConfigKey::COUPLING, ConfigKey::Coupling },
	{ sigrok::ConfigKey::TRIGGER_MATCH, ConfigKey::TriggerMatch },
	{ sigrok::ConfigKey::SAMPLE_INTERVAL, ConfigKey::SampleInterval },
	{ sigrok::ConfigKey::NUM_HDIV, ConfigKey::NumHDiv },
	{ sigrok::ConfigKey::NUM_VDIV, ConfigKey::NumVDiv },
	{ sigrok::ConfigKey::SPL_WEIGHT_FREQ, ConfigKey::SplWeightFreq },
	{ sigrok::ConfigKey::SPL_WEIGHT_TIME, ConfigKey::SplWeightTime },
	{ sigrok::ConfigKey::SPL_MEASUREMENT_RANGE, ConfigKey::SplMeasurementRange },
	{ sigrok::ConfigKey::HOLD_MAX, ConfigKey::HoldMax },
	{ sigrok::ConfigKey::HOLD_MIN, ConfigKey::HoldMin },
	{ sigrok::ConfigKey::VOLTAGE_THRESHOLD, ConfigKey::VoltageThreshold },
	{ sigrok::ConfigKey::EXTERNAL_CLOCK, ConfigKey::ExternalClock },
	{ sigrok::ConfigKey::SWAP, ConfigKey::Swap },
	{ sigrok::ConfigKey::CENTER_FREQUENCY, ConfigKey::CenterFrequency },
	{ sigrok::ConfigKey::NUM_LOGIC_CHANNELS, ConfigKey::NumLogicChannels },
	{ sigrok::ConfigKey::NUM_ANALOG_CHANNELS, ConfigKey::NumAnalogChannels },
	{ sigrok::ConfigKey::VOLTAGE, ConfigKey::Voltage },
	{ sigrok::ConfigKey::VOLTAGE_TARGET, ConfigKey::VoltageTarget },
	{ sigrok::ConfigKey::CURRENT, ConfigKey::Current },
	{ sigrok::ConfigKey::CURRENT_LIMIT, ConfigKey::CurrentLimit },
	{ sigrok::ConfigKey::ENABLED, ConfigKey::Enabled },
	{ sigrok::ConfigKey::CHANNEL_CONFIG, ConfigKey::ChannelConfig },
	{ sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ENABLED, ConfigKey::OverVoltageProtectionEnabled },
	{ sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ACTIVE, ConfigKey::OverVoltageProtectionActive },
	{ sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_THRESHOLD, ConfigKey::OverVoltageProtectionThreshold },
	{ sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ENABLED, ConfigKey::OverCurrentProtectionEnabled },
	{ sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ACTIVE, ConfigKey::OverCurrentProtectionActive },
	{ sigrok::ConfigKey::OVER_CURRENT_PROTECTION_THRESHOLD, ConfigKey::OverCurrentProtectionThreshold },
	{ sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION, ConfigKey::OverTemperatureProtectionEnabled },
	{ sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION_ACTIVE, ConfigKey::OverTemperatureProtectionActive },
	{ sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION, ConfigKey::UnderVoltageConditionEnabled },
	{ sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_ACTIVE, ConfigKey::UnderVoltageConditionActive },
	{ sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD, ConfigKey::UnderVoltageConditionThreshold },
	{ sigrok::ConfigKey::CLOCK_EDGE, ConfigKey::ClockEdge },
	{ sigrok::ConfigKey::AMPLITUDE, ConfigKey::Amplitude },
	{ sigrok::ConfigKey::REGULATION, ConfigKey::Regulation },
	{ sigrok::ConfigKey::OUTPUT_FREQUENCY, ConfigKey::OutputFrequency },
	{ sigrok::ConfigKey::OUTPUT_FREQUENCY_TARGET, ConfigKey::OutputFrequencyTarget },
	{ sigrok::ConfigKey::MEASURED_QUANTITY, ConfigKey::MeasuredQuantity },
	{ sigrok::ConfigKey::EQUIV_CIRCUIT_MODEL, ConfigKey::EquivCircuitModel },
	{ sigrok::ConfigKey::TRIGGER_LEVEL, ConfigKey::TriggerLevel },
	{ sigrok::ConfigKey::EXTERNAL_CLOCK_SOURCE, ConfigKey::ExternalClockSource },
	{ sigrok::ConfigKey::OFFSET, ConfigKey::Offset },
	{ sigrok::ConfigKey::TRIGGER_PATTERN, ConfigKey::TriggerPattern },
	{ sigrok::ConfigKey::HIGH_RESOLUTION, ConfigKey::HighResolution },
	{ sigrok::ConfigKey::PEAK_DETECTION, ConfigKey::PeakDetection },
	{ sigrok::ConfigKey::LOGIC_THRESHOLD, ConfigKey::LogicThreshold },
	{ sigrok::ConfigKey::LOGIC_THRESHOLD_CUSTOM, ConfigKey::LogicThresholdCustom },
	{ sigrok::ConfigKey::RANGE, ConfigKey::Range },
	{ sigrok::ConfigKey::DIGITS, ConfigKey::Digits },
	{ sigrok::ConfigKey::SESSIONFILE, ConfigKey::SessionFile },
	{ sigrok::ConfigKey::CAPTUREFILE, ConfigKey::CaptureFile },
	{ sigrok::ConfigKey::CAPTURE_UNITSIZE, ConfigKey::CaptureUnitSize },
	{ sigrok::ConfigKey::POWER_OFF, ConfigKey::PowerOff },
	{ sigrok::ConfigKey::DATA_SOURCE, ConfigKey::DataSource },
	{ sigrok::ConfigKey::PROBE_FACTOR, ConfigKey::ProbeFactor },
	{ sigrok::ConfigKey::ADC_POWERLINE_CYCLES, ConfigKey::ADCPowerlineCycles },
	{ sigrok::ConfigKey::DATALOG, ConfigKey::DataLog },
	{ sigrok::ConfigKey::DEVICE_MODE, ConfigKey::DeviceMode },
	{ sigrok::ConfigKey::TEST_MODE, ConfigKey::TestMode },
};

map<ConfigKey, const sigrok::ConfigKey *> config_key_sr_config_key_map = {
	{ ConfigKey::Samplerate, sigrok::ConfigKey::SAMPLERATE },
	{ ConfigKey::CaptureRatio, sigrok::ConfigKey::CAPTURE_RATIO },
	{ ConfigKey::PatternMode, sigrok::ConfigKey::PATTERN_MODE },
	{ ConfigKey::RLE, sigrok::ConfigKey::RLE },
	{ ConfigKey::TriggerSlope, sigrok::ConfigKey::TRIGGER_SLOPE },
	{ ConfigKey::Averaging, sigrok::ConfigKey::AVERAGING },
	{ ConfigKey::AvgSamples, sigrok::ConfigKey::AVG_SAMPLES },
	{ ConfigKey::TriggerSource, sigrok::ConfigKey::TRIGGER_SOURCE },
	{ ConfigKey::HorizTriggerPos, sigrok::ConfigKey::HORIZ_TRIGGERPOS },
	{ ConfigKey::BufferSize, sigrok::ConfigKey::BUFFERSIZE },
	{ ConfigKey::TimeBase, sigrok::ConfigKey::TIMEBASE },
	{ ConfigKey::Filter, sigrok::ConfigKey::FILTER },
	{ ConfigKey::VDiv, sigrok::ConfigKey::VDIV },
	{ ConfigKey::Coupling, sigrok::ConfigKey::COUPLING },
	{ ConfigKey::TriggerMatch, sigrok::ConfigKey::TRIGGER_MATCH },
	{ ConfigKey::SampleInterval, sigrok::ConfigKey::SAMPLE_INTERVAL },
	{ ConfigKey::NumHDiv, sigrok::ConfigKey::NUM_HDIV },
	{ ConfigKey::NumVDiv, sigrok::ConfigKey::NUM_VDIV },
	{ ConfigKey::SplWeightFreq, sigrok::ConfigKey::SPL_WEIGHT_FREQ },
	{ ConfigKey::SplWeightTime, sigrok::ConfigKey::SPL_WEIGHT_TIME },
	{ ConfigKey::SplMeasurementRange, sigrok::ConfigKey::SPL_MEASUREMENT_RANGE },
	{ ConfigKey::HoldMax, sigrok::ConfigKey::HOLD_MAX },
	{ ConfigKey::HoldMin, sigrok::ConfigKey::HOLD_MIN },
	{ ConfigKey::VoltageThreshold, sigrok::ConfigKey::VOLTAGE_THRESHOLD },
	{ ConfigKey::ExternalClock, sigrok::ConfigKey::EXTERNAL_CLOCK },
	{ ConfigKey::Swap, sigrok::ConfigKey::SWAP },
	{ ConfigKey::CenterFrequency, sigrok::ConfigKey::CENTER_FREQUENCY },
	{ ConfigKey::NumLogicChannels, sigrok::ConfigKey::NUM_LOGIC_CHANNELS },
	{ ConfigKey::NumAnalogChannels, sigrok::ConfigKey::NUM_ANALOG_CHANNELS },
	{ ConfigKey::Voltage, sigrok::ConfigKey::VOLTAGE },
	{ ConfigKey::VoltageTarget, sigrok::ConfigKey::VOLTAGE_TARGET },
	{ ConfigKey::Current, sigrok::ConfigKey::CURRENT },
	{ ConfigKey::CurrentLimit, sigrok::ConfigKey::CURRENT_LIMIT },
	{ ConfigKey::Enabled, sigrok::ConfigKey::ENABLED },
	{ ConfigKey::ChannelConfig, sigrok::ConfigKey::CHANNEL_CONFIG },
	{ ConfigKey::OverVoltageProtectionEnabled, sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ENABLED },
	{ ConfigKey::OverVoltageProtectionActive, sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_ACTIVE },
	{ ConfigKey::OverVoltageProtectionThreshold, sigrok::ConfigKey::OVER_VOLTAGE_PROTECTION_THRESHOLD },
	{ ConfigKey::OverCurrentProtectionEnabled, sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ENABLED },
	{ ConfigKey::OverCurrentProtectionActive, sigrok::ConfigKey::OVER_CURRENT_PROTECTION_ACTIVE },
	{ ConfigKey::OverCurrentProtectionThreshold, sigrok::ConfigKey::OVER_CURRENT_PROTECTION_THRESHOLD },
	{ ConfigKey::OverTemperatureProtectionEnabled, sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION },
	{ ConfigKey::OverTemperatureProtectionActive, sigrok::ConfigKey::OVER_TEMPERATURE_PROTECTION_ACTIVE },
	{ ConfigKey::UnderVoltageConditionEnabled, sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION },
	{ ConfigKey::UnderVoltageConditionActive, sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_ACTIVE },
	{ ConfigKey::UnderVoltageConditionThreshold, sigrok::ConfigKey::UNDER_VOLTAGE_CONDITION_THRESHOLD },
	{ ConfigKey::ClockEdge, sigrok::ConfigKey::CLOCK_EDGE },
	{ ConfigKey::Amplitude, sigrok::ConfigKey::AMPLITUDE },
	{ ConfigKey::Regulation, sigrok::ConfigKey::REGULATION },
	{ ConfigKey::OutputFrequency, sigrok::ConfigKey::OUTPUT_FREQUENCY },
	{ ConfigKey::OutputFrequencyTarget, sigrok::ConfigKey::OUTPUT_FREQUENCY_TARGET },
	{ ConfigKey::MeasuredQuantity, sigrok::ConfigKey::MEASURED_QUANTITY },
	{ ConfigKey::EquivCircuitModel, sigrok::ConfigKey::EQUIV_CIRCUIT_MODEL },
	{ ConfigKey::TriggerLevel, sigrok::ConfigKey::TRIGGER_LEVEL },
	{ ConfigKey::ExternalClockSource, sigrok::ConfigKey::EXTERNAL_CLOCK_SOURCE },
	{ ConfigKey::Offset, sigrok::ConfigKey::OFFSET },
	{ ConfigKey::TriggerPattern, sigrok::ConfigKey::TRIGGER_PATTERN },
	{ ConfigKey::HighResolution, sigrok::ConfigKey::HIGH_RESOLUTION },
	{ ConfigKey::PeakDetection, sigrok::ConfigKey::PEAK_DETECTION },
	{ ConfigKey::LogicThreshold, sigrok::ConfigKey::LOGIC_THRESHOLD },
	{ ConfigKey::LogicThresholdCustom, sigrok::ConfigKey::LOGIC_THRESHOLD_CUSTOM },
	{ ConfigKey::Range, sigrok::ConfigKey::RANGE },
	{ ConfigKey::Digits, sigrok::ConfigKey::DIGITS },
	{ ConfigKey::SessionFile, sigrok::ConfigKey::SESSIONFILE },
	{ ConfigKey::CaptureFile, sigrok::ConfigKey::CAPTUREFILE },
	{ ConfigKey::CaptureUnitSize, sigrok::ConfigKey::CAPTURE_UNITSIZE },
	{ ConfigKey::PowerOff, sigrok::ConfigKey::POWER_OFF },
	{ ConfigKey::DataSource, sigrok::ConfigKey::DATA_SOURCE },
	{ ConfigKey::ProbeFactor, sigrok::ConfigKey::PROBE_FACTOR },
	{ ConfigKey::ADCPowerlineCycles, sigrok::ConfigKey::ADC_POWERLINE_CYCLES },
	{ ConfigKey::DataLog, sigrok::ConfigKey::DATALOG },
	{ ConfigKey::DeviceMode, sigrok::ConfigKey::DEVICE_MODE },
	{ ConfigKey::TestMode, sigrok::ConfigKey::TEST_MODE },
};

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
	{ ConfigKey::HorizTriggerPos, data::Unit::Unknown },
	{ ConfigKey::BufferSize, data::Unit::Unknown },
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
 * Return all known device type
 *
 * @return The device type name map
 */
device_type_name_map_t get_device_type_name_map();

/**
 * Return all known connection keys
 *
 * @return The connection key name map
 */
connection_key_name_map_t get_connection_key_name_map();

/**
 * Return all known config keys
 *
 * @return The config key name map
 */
config_key_name_map_t get_config_key_name_map();


/**
 * Check if the driver is supported by SmuView.
 *
 * @param sr_driver The sigrok Driver to check.
 *
 * @return true, if the driver is supported.
 */
bool is_supported_driver(shared_ptr<sigrok::Driver> sr_driver);

/**
 * Check if the driver is a power supply or a electronic load.
 *
 * @param sr_driver The sigrok Driver to check.
 *
 * @return true, if the driver is a power supply or a electronic load.
 */
bool is_source_sink_driver(shared_ptr<sigrok::Driver> sr_driver);

/**
 * Check if the driver is a measurement device (dmm, lcr meter, ...).
 *
 * @param sr_driver The sigrok Driver to check.
 *
 * @return true, if the driver is a measurement device.
 */
 bool is_measurement_driver(shared_ptr<sigrok::Driver> sr_driver);

/**
 * Check if the driver is a demo device.
 *
 * @param sr_driver The sigrok Driver to check.
 *
 * @return true, if the driver is a demo device.
 */
 bool is_demo_driver(shared_ptr<sigrok::Driver> sr_driver);

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
