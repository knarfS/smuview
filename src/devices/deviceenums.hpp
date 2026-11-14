/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2026 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef DEVICES_DEVICEENUMS_HPP
#define DEVICES_DEVICEENUMS_HPP

namespace sv {
namespace devices {

/**
 * All available and supported device types
 */
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

/**
 * All avaiable and supported acquisition states of a device
 */
enum class AcquisitionState
{
	Stopped,
	Running,
	Paused
};

/**
 * All available and supported connection keys, use to connect to devices
 */
enum class ConnectionKey
{
	/** Specification on how to connect to a device. */
	Conn,
	/** Serial communication specification, in the form: */
	SerialComm,
	/** Modbus slave address specification. */
	ModbusAddr,
	/**
	 * The device supports setting a sample time limit (how long the sample
	 * acquisition should run, in ms).
	 */
	LimitMsec,
	/**
	 * The device supports setting a sample number limit (how many samples
	 * should be acquired).
	 */
	LimitSamples,
	/**
	 * The device supports setting a frame limit (how many frames should be
	 * acquired).
	 */
	LimitFrames,
	/** The device supports continuous sampling. */
	Continuous,
	/** Unknown connection key. */
	Unknown,
};

/**
 * All available and supported connection keys, use to controll devices
 */
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
	/**
	 * Channel regulation get: "CV", "CC" or "UR", denoting constant voltage,
	 * constant current or unregulated.
	 */
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
	/**
	 * Which external clock source to use if the device supports multiple
	 * external clock channels.
	 */
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

} // namespace devices
} // namespace sv

#endif // DEVICES_DEVICEENUMS_HPP
