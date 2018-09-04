/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DEVICES_DEVICEUTIL_HPP
#define DEVICES_DEVICEUTIL_HPP

#include <map>
#include <set>
#include <vector>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QObject>
#include <QString>

using std::map;
using std::set;
using std::vector;

namespace sigrok {
class ConfigKey;
}

namespace sv {
namespace devices {

enum class DeviceTypeKey
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
	/** Offset of a source without strictly-defined MQ. */
	Offset,
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
	/** The device supports setting a sample time limit (how long the sample acquisition should run, in ms). */
	LimitMsec,
	/** The device supports setting a sample number limit (how many samples should be acquired). */
	LimitSamples,
	/** The device supports setting a frame limit (how many frames should be acquired). */
	LimitFrames,
	/** The device supports continuous sampling. */
	Continuous,
	/** The device has internal storage, into which data is logged. */
	DataLog,
	/** Device mode for multi-function devices. */
	DeviceMode,
	/** Self test mode. */
	TestMode,
	/** Unknown config key. */
	Unknown,
};

enum class DataType
{
	UInt64,
	Sting,
	Bool,
	Float,
	RationalPeriod,
	RationalVolt,
	KeyValue,
	Uint64Range,
	DoubleRange,
	Int32,
	MQ,
	Unknown,
};

namespace deviceutil {

typedef map<DeviceTypeKey, QString> device_type_key_name_map_t;
typedef map<ConnectionKey, QString> connection_key_name_map_t;
typedef map<ConfigKey, QString> config_key_name_map_t;

namespace {

// TODO: Use tr(), QCoreApplication::translate(), QT_TR_NOOP() or
//       QT_TRANSLATE_NOOP() for translation.
//       See: http://doc.qt.io/qt-5/i18n-source-translation.html
device_type_key_name_map_t device_type_key_name_map = {
	{ DeviceTypeKey::LogicAnalyzer, QString("Logic Analyzer") },
	{ DeviceTypeKey::Oscilloscope, QString("Oscilloscope") },
	{ DeviceTypeKey::Multimeter, QString("Multimeter") },
	{ DeviceTypeKey::DemoDev, QString("Demo Device") },
	{ DeviceTypeKey::SoundLevelMeter, QString("Soundlevelmeter") },
	{ DeviceTypeKey::Thermometer, QString("Thermometer") },
	{ DeviceTypeKey::Hygrometer, QString("Hygrometer") },
	{ DeviceTypeKey::Energymeter, QString("Energymeter") },
	{ DeviceTypeKey::Demodulator, QString("Demodulator") },
	{ DeviceTypeKey::PowerSupply, QString("Power Supply") },
	{ DeviceTypeKey::LcrMeter, QString("LCR Meter") },
	{ DeviceTypeKey::ElectronicLoad, QString("Electronic Load") },
	{ DeviceTypeKey::Scale, QString("Scale") },
	{ DeviceTypeKey::SignalGenerator, QString("Signal Generator") },
	{ DeviceTypeKey::Powermeter, QString("Power Meter") },
	{ DeviceTypeKey::Unknown, QString("Unknown") },
};

connection_key_name_map_t connection_key_name_map = {
	{ ConnectionKey::Conn, QString("Connection String") },
	{ ConnectionKey::SerialComm, QString("Serial Command") },
	{ ConnectionKey::ModbusAddr, QString("ModBus Address") },
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
	{ ConfigKey::Offset, QString("Offset") },
	{ ConfigKey::Regulation, QString("Regulation") },
	{ ConfigKey::OutputFrequency, QString("Output Frequency") },
	{ ConfigKey::OutputFrequencyTarget, QString("Output Frequency Target") },
	{ ConfigKey::MeasuredQuantity, QString("Measured Quantity") },
	{ ConfigKey::EquivCircuitModel, QString("Equivalent Circuit Model") },
	{ ConfigKey::TriggerLevel, QString("Trigger Level") },
	{ ConfigKey::ExternalClockSource, QString("External Clock Source") },
	{ ConfigKey::SessionFile, QString("Session File") },
	{ ConfigKey::CaptureFile, QString("Capture File") },
	{ ConfigKey::CaptureUnitSize, QString("Capture Unit Size") },
	{ ConfigKey::PowerOff, QString("Power Off") },
	{ ConfigKey::DataSource, QString("Data Source") },
	{ ConfigKey::ProbeFactor, QString("Probe Factor") },
	{ ConfigKey::ADCPowerlineCycles, QString("ADC Powerline Cycles") },
	{ ConfigKey::LimitMsec, QString("Limit Milliseconds") },
	{ ConfigKey::LimitSamples, QString("Limit Samples") },
	{ ConfigKey::LimitFrames, QString("Limit Frames") },
	{ ConfigKey::Continuous, QString("Continuous") },
	{ ConfigKey::DataLog, QString("Data Log") },
	{ ConfigKey::DeviceMode, QString("Device Mode") },
	{ ConfigKey::TestMode, QString("Test Mode") },
	{ ConfigKey::Unknown, QString("Unknown") },
};

map<const sigrok::ConfigKey *, DeviceTypeKey> sr_config_key_device_type_key_map = {
	{ sigrok::ConfigKey::LOGIC_ANALYZER, DeviceTypeKey::LogicAnalyzer },
	{ sigrok::ConfigKey::OSCILLOSCOPE, DeviceTypeKey::Oscilloscope },
	{ sigrok::ConfigKey::MULTIMETER, DeviceTypeKey::Multimeter },
	{ sigrok::ConfigKey::DEMO_DEV, DeviceTypeKey::DemoDev },
	{ sigrok::ConfigKey::SOUNDLEVELMETER, DeviceTypeKey::SoundLevelMeter },
	{ sigrok::ConfigKey::THERMOMETER, DeviceTypeKey::Thermometer },
	{ sigrok::ConfigKey::HYGROMETER, DeviceTypeKey::Hygrometer },
	{ sigrok::ConfigKey::ENERGYMETER, DeviceTypeKey::Energymeter },
	{ sigrok::ConfigKey::DEMODULATOR, DeviceTypeKey::Demodulator },
	{ sigrok::ConfigKey::POWER_SUPPLY, DeviceTypeKey::PowerSupply },
	{ sigrok::ConfigKey::LCRMETER, DeviceTypeKey::LcrMeter },
	{ sigrok::ConfigKey::ELECTRONIC_LOAD, DeviceTypeKey::ElectronicLoad },
	{ sigrok::ConfigKey::SCALE, DeviceTypeKey::Scale },
	{ sigrok::ConfigKey::SIGNAL_GENERATOR, DeviceTypeKey::SignalGenerator },
	{ sigrok::ConfigKey::POWERMETER, DeviceTypeKey::Powermeter },
};

map<DeviceTypeKey, const sigrok::ConfigKey *> device_type_key_sr_config_key_map = {
	{ DeviceTypeKey::LogicAnalyzer, sigrok::ConfigKey::LOGIC_ANALYZER },
	{ DeviceTypeKey::Oscilloscope, sigrok::ConfigKey::OSCILLOSCOPE },
	{ DeviceTypeKey::Multimeter, sigrok::ConfigKey::MULTIMETER },
	{ DeviceTypeKey::DemoDev, sigrok::ConfigKey::DEMO_DEV },
	{ DeviceTypeKey::SoundLevelMeter, sigrok::ConfigKey::SOUNDLEVELMETER },
	{ DeviceTypeKey::Thermometer, sigrok::ConfigKey::THERMOMETER },
	{ DeviceTypeKey::Hygrometer, sigrok::ConfigKey::HYGROMETER },
	{ DeviceTypeKey::Energymeter, sigrok::ConfigKey::ENERGYMETER },
	{ DeviceTypeKey::Demodulator, sigrok::ConfigKey::DEMODULATOR },
	{ DeviceTypeKey::PowerSupply, sigrok::ConfigKey::POWER_SUPPLY },
	{ DeviceTypeKey::LcrMeter, sigrok::ConfigKey::LCRMETER },
	{ DeviceTypeKey::ElectronicLoad, sigrok::ConfigKey::ELECTRONIC_LOAD },
	{ DeviceTypeKey::Scale, sigrok::ConfigKey::SCALE },
	{ DeviceTypeKey::SignalGenerator, sigrok::ConfigKey::SIGNAL_GENERATOR },
	{ DeviceTypeKey::Powermeter, sigrok::ConfigKey::POWERMETER },
};

map<const sigrok::ConfigKey *, ConnectionKey> sr_config_key_connection_key_map = {
	{ sigrok::ConfigKey::CONN, ConnectionKey::Conn },
	{ sigrok::ConfigKey::SERIALCOMM, ConnectionKey::SerialComm },
	{ sigrok::ConfigKey::MODBUSADDR, ConnectionKey::ModbusAddr },
};

map<ConnectionKey, const sigrok::ConfigKey *> connection_key_sr_config_key_map = {
	{ ConnectionKey::Conn, sigrok::ConfigKey::CONN },
	{ ConnectionKey::SerialComm, sigrok::ConfigKey::SERIALCOMM },
	{ ConnectionKey::ModbusAddr, sigrok::ConfigKey::MODBUSADDR },
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
	{ sigrok::ConfigKey::OFFSET, ConfigKey::Offset },
	{ sigrok::ConfigKey::REGULATION, ConfigKey::Regulation },
	{ sigrok::ConfigKey::OUTPUT_FREQUENCY, ConfigKey::OutputFrequency },
	{ sigrok::ConfigKey::OUTPUT_FREQUENCY_TARGET, ConfigKey::OutputFrequencyTarget },
	{ sigrok::ConfigKey::MEASURED_QUANTITY, ConfigKey::MeasuredQuantity },
	{ sigrok::ConfigKey::EQUIV_CIRCUIT_MODEL, ConfigKey::EquivCircuitModel },
	{ sigrok::ConfigKey::TRIGGER_LEVEL, ConfigKey::TriggerLevel },
	{ sigrok::ConfigKey::EXTERNAL_CLOCK_SOURCE, ConfigKey::ExternalClockSource },
	{ sigrok::ConfigKey::SESSIONFILE, ConfigKey::SessionFile },
	{ sigrok::ConfigKey::CAPTUREFILE, ConfigKey::CaptureFile },
	{ sigrok::ConfigKey::CAPTURE_UNITSIZE, ConfigKey::CaptureUnitSize },
	{ sigrok::ConfigKey::POWER_OFF, ConfigKey::PowerOff },
	{ sigrok::ConfigKey::DATA_SOURCE, ConfigKey::DataSource },
	{ sigrok::ConfigKey::PROBE_FACTOR, ConfigKey::ProbeFactor },
	{ sigrok::ConfigKey::ADC_POWERLINE_CYCLES, ConfigKey::ADCPowerlineCycles },
	{ sigrok::ConfigKey::LIMIT_MSEC, ConfigKey::LimitMsec },
	{ sigrok::ConfigKey::LIMIT_SAMPLES, ConfigKey::LimitSamples },
	{ sigrok::ConfigKey::LIMIT_FRAMES, ConfigKey::LimitFrames },
	{ sigrok::ConfigKey::CONTINUOUS, ConfigKey::Continuous },
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
	{ ConfigKey::Offset, sigrok::ConfigKey::OFFSET },
	{ ConfigKey::Regulation, sigrok::ConfigKey::REGULATION },
	{ ConfigKey::OutputFrequency, sigrok::ConfigKey::OUTPUT_FREQUENCY },
	{ ConfigKey::OutputFrequencyTarget, sigrok::ConfigKey::OUTPUT_FREQUENCY_TARGET },
	{ ConfigKey::MeasuredQuantity, sigrok::ConfigKey::MEASURED_QUANTITY },
	{ ConfigKey::EquivCircuitModel, sigrok::ConfigKey::EQUIV_CIRCUIT_MODEL },
	{ ConfigKey::TriggerLevel, sigrok::ConfigKey::TRIGGER_LEVEL },
	{ ConfigKey::ExternalClockSource, sigrok::ConfigKey::EXTERNAL_CLOCK_SOURCE },
	{ ConfigKey::SessionFile, sigrok::ConfigKey::SESSIONFILE },
	{ ConfigKey::CaptureFile, sigrok::ConfigKey::CAPTUREFILE },
	{ ConfigKey::CaptureUnitSize, sigrok::ConfigKey::CAPTURE_UNITSIZE },
	{ ConfigKey::PowerOff, sigrok::ConfigKey::POWER_OFF },
	{ ConfigKey::DataSource, sigrok::ConfigKey::DATA_SOURCE },
	{ ConfigKey::ProbeFactor, sigrok::ConfigKey::PROBE_FACTOR },
	{ ConfigKey::ADCPowerlineCycles, sigrok::ConfigKey::ADC_POWERLINE_CYCLES },
	{ ConfigKey::LimitMsec, sigrok::ConfigKey::LIMIT_MSEC },
	{ ConfigKey::LimitSamples, sigrok::ConfigKey::LIMIT_SAMPLES },
	{ ConfigKey::LimitFrames, sigrok::ConfigKey::LIMIT_FRAMES },
	{ ConfigKey::Continuous, sigrok::ConfigKey::CONTINUOUS },
	{ ConfigKey::DataLog, sigrok::ConfigKey::DATALOG },
	{ ConfigKey::DeviceMode, sigrok::ConfigKey::DEVICE_MODE },
	{ ConfigKey::TestMode, sigrok::ConfigKey::TEST_MODE },
};

map<const sigrok::DataType *, DataType> sr_data_type_data_type_map = {
	{ sigrok::DataType::UINT64, DataType::UInt64 },
	{ sigrok::DataType::STRING, DataType::Sting },
	{ sigrok::DataType::BOOL, DataType::Bool },
	{ sigrok::DataType::FLOAT, DataType::Float },
	{ sigrok::DataType::RATIONAL_PERIOD, DataType::RationalPeriod },
	{ sigrok::DataType::RATIONAL_VOLT, DataType::RationalVolt },
	{ sigrok::DataType::KEYVALUE, DataType::KeyValue },
	{ sigrok::DataType::UINT64_RANGE, DataType::Uint64Range },
	{ sigrok::DataType::DOUBLE_RANGE, DataType::DoubleRange },
	{ sigrok::DataType::INT32, DataType::Int32 },
	{ sigrok::DataType::MQ, DataType::MQ },
};

map<DataType, const sigrok::DataType *> data_type_sr_data_type_map = {
	{ DataType::UInt64, sigrok::DataType::UINT64 },
	{ DataType::Sting, sigrok::DataType::STRING },
	{ DataType::Bool, sigrok::DataType::BOOL },
	{ DataType::Float, sigrok::DataType::FLOAT },
	{ DataType::RationalPeriod, sigrok::DataType::RATIONAL_PERIOD },
	{ DataType::RationalVolt, sigrok::DataType::RATIONAL_VOLT },
	{ DataType::KeyValue, sigrok::DataType::KEYVALUE },
	{ DataType::Uint64Range, sigrok::DataType::UINT64_RANGE },
	{ DataType::DoubleRange, sigrok::DataType::DOUBLE_RANGE },
	{ DataType::Int32, sigrok::DataType::INT32 },
	{ DataType::MQ, sigrok::DataType::MQ },
};

} // namespace

/**
 * Returns all known device type keys
 *
 * @return The device type key name map
 */
device_type_key_name_map_t get_device_type_key_name_map();

/**
 * Returns all known connection keys
 *
 * @return The connection key name map
 */
connection_key_name_map_t get_connection_key_name_map();

/**
 * Returns all known config keys
 *
 * @return The config key name map
 */
config_key_name_map_t get_config_key_name_map();


/**
 * Returns the corresponding DeviceTypeKey for a sigrok ConfigKey
 *
 * @param sr_config_key The sigrok ConfigKey
 *
 * @return The DeviceTypeKey.
 */
DeviceTypeKey get_device_type_key(const sigrok::ConfigKey *sr_config_key);

/**
 * Returns the corresponding DeviceTypeKey for a sigrok ConfigKey (unit32_t)
 *
 * @param sr_config_key The sigrok ConfigKey as uint32_t
 *
 * @return The DeviceTypeKey.
 */
DeviceTypeKey get_device_type_key(uint32_t sr_config_key);

/**
 * Returns the corresponding sigrok ConfigKey for a DeviceTypeKey
 *
 * @param device_type_key The DeviceTypeKey.
 *
 * @return The sigrok ConfigKeyt.
 */
const sigrok::ConfigKey *get_sr_config_key(DeviceTypeKey device_type_key);

/**
 * Returns the corresponding sigrok ConfigKey ID for a DeviceTypeKey
 *
 * @param device_type_key The DeviceTypeKey
 *
 * @return The sigrok ConfigKey ID as uint32_t.
 */
uint32_t get_sr_config_key_id(DeviceTypeKey device_type_key);

/**
 * Checks if the DeviceTypeKey is a known sigrok DeviceTypeKey / ConfigKey
 *
 * @param device_type_key The DeviceTypeKey
 *
 * @return true if it is a known sigrok DeviceTypeKey / ConfigKey
 */
bool is_valid_sr_config_key(DeviceTypeKey device_type_key);


/**
 * Returns the corresponding ConnectionKey for a sigrok ConfigKey
 *
 * @param sr_config_key The sigrok ConfigKey
 *
 * @return The ConnectionKey.
 */
ConnectionKey get_connection_key(const sigrok::ConfigKey *sr_config_key);

/**
 * Returns the corresponding ConnectionKey for a sigrok ConfigKey (unit32_t)
 *
 * @param sr_config_key The sigrok ConfigKey as uint32_t
 *
 * @return The ConnectionKey.
 */
ConnectionKey get_connection_key(uint32_t sr_config_key);

/**
 * Returns the corresponding sigrok ConfigKey for a ConnectionKey
 *
 * @param connection_key The ConnectionKey.
 *
 * @return The sigrok ConfigKey.
 */
const sigrok::ConfigKey *get_sr_config_key(ConnectionKey connection_key);

/**
 * Returns the corresponding sigrok ConfigKey ID for a ConnectionKey
 *
 * @param connection_key The ConnectionKey
 *
 * @return The sigrok ConfigKey ID as uint32_t.
 */
uint32_t get_sr_config_key_id(ConnectionKey connection_key);

/**
 * Checks if the ConnectionKey is a known sigrok ConnectionKey / ConfigKey
 *
 * @param connection_key The ConnectionKey
 *
 * @return true if it is a known sigrok ConnectionKey / ConfigKey
 */
bool is_valid_sr_config_key(ConnectionKey connection_key);


/**
 * Returns the corresponding ConfigKey for a sigrok ConfigKey
 *
 * @param sr_config_key The sigrok ConfigKey
 *
 * @return The ConfigKey.
 */
ConfigKey get_config_key(const sigrok::ConfigKey *sr_config_key);

/**
 * Returns the corresponding ConfigKey for a sigrok ConfigKey (unit32_t)
 *
 * @param sr_config_key The sigrok ConfigKey as uint32_t
 *
 * @return The ConfigKey.
 */
ConfigKey get_config_key(uint32_t sr_config_key);

/**
 * Returns the corresponding sigrok ConfigKey for a ConfigKey
 *
 * @param config_key The ConfigKey.
 *
 * @return The sigrok ConfigKey.
 */
const sigrok::ConfigKey *get_sr_config_key(ConfigKey config_key);

/**
 * Returns the corresponding sigrok ConfigKey ID for a ConfigKey
 *
 * @param config_key The ConfigKey
 *
 * @return The sigrok ConfigKey ID as uint32_t.
 */
uint32_t get_sr_config_key_id(ConfigKey config_key);

/**
 * Checks if the ConfigKey is a known sigrok ConfigKey
 *
 * @param config_key The ConfigKey
 *
 * @return true if it is a known sigrok ConfigKey
 */
bool is_valid_sr_config_key(ConfigKey config_key);


/**
 * Returns the corresponding DataType for a sigrok DataType
 *
 * @param sr_data_type The sigrok DataType
 *
 * @return The DataType.
 */
DataType get_data_type(const sigrok::DataType *sr_data_type);

/**
 * Returns the corresponding DataType for a sigrok DataType (unit32_t)
 *
 * @param sr_data_type The sigrok DataType as uint32_t
 *
 * @return The DataType.
 */
DataType get_data_type(uint32_t sr_data_type);

/**
 * Returns the corresponding sigrok DataType for a DataType
 *
 * @param data_type The DataType.
 *
 * @return The sigrok DataType.
 */
const sigrok::DataType *get_sr_data_type(DataType data_type);

/**
 * Returns the corresponding sigrok DataType ID for a DataType
 *
 * @param data_type The DataType
 *
 * @return The sigrok DataType ID as uint32_t.
 */
uint32_t get_sr_data_type_id(DataType data_type);

/**
 * Checks if the DataType is a known sigrok DataType
 *
 * @param data_type The DataType
 *
 * @return true if it is a known sigrok DataType
 */
bool is_valid_sr_data_type(DataType data_type);


/**
 * Formats a DeviceTypeKey to a string
 *
 * @param device_type_key The DeviceTypeKey to format.
 *
 * @return The formatted device type.
 */
QString format_device_type_key(DeviceTypeKey device_type_key);

/**
 * Formats a ConnectionKey to a string
 *
 * @param connection_key The ConnectionKey to format.
 *
 * @return The formatted ConnectionKey.
 */
QString format_connection_key(ConnectionKey connection_key);

/**
 * Formats a ConfigKey to a string
 *
 * @param config_key The ConfigKey to format.
 *
 * @return The formatted ConfigKey.
 */
QString format_config_key(ConfigKey config_key);


/**
 * Gets the DataType for a ConfigKey
 *
 * @param config_key The ConfigKey.
 *
 * @return The DataType for the ConfigKey.
 */
DataType get_data_type_for_config_key(ConfigKey config_key);

} // namespace deviceutil
} // namespace devices
} // namespace sv

#endif // DEVICES_DEVICEUTIL_HPP
