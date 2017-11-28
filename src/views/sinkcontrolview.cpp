/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "sinkcontrolview.hpp"
#include "src/session.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/widgets/controlbutton.hpp"
#include "src/widgets/led.hpp"
#include "src/widgets/optionalvaluecontrol.hpp"
#include "src/widgets/valuecontrol.hpp"

namespace sv {
namespace views {

SinkControlView::SinkControlView(Session &session,
	shared_ptr<devices::HardwareDevice> device, QWidget *parent) :
		BaseView(session, parent),
	device_(device)
{
	setup_ui();
	connect_signals();
	init_values();
}

void SinkControlView::setup_ui()
{
	double min;
	double max;
	double step;
	QIcon red_icon(":/icons/status-red.svg");
	QIcon green_icon(":/icons/status-green.svg");
	QIcon grey_icon(":/icons/status-grey.svg");

	QVBoxLayout *layout = new QVBoxLayout();

	QGridLayout *infoLayout = new QGridLayout();

	// Enable button
	setEnableButton = new widgets::ControlButton(
		&devices::HardwareDevice::get_enable,
		&devices::HardwareDevice::set_enable,
		&devices::HardwareDevice::is_enable_getable,
		&devices::HardwareDevice::is_enable_setable,
		device_);
	infoLayout->addWidget(setEnableButton, 0, 0, 2, 1,  Qt::AlignLeft);

	// Regulation Leds
	//cvLed = new widgets::Led(false, false, tr("CV"));
	//ledLayout->addWidget(cvLed, 0, 1, Qt::AlignLeft);
	//ccLed = new widgets::Led(true, false, tr("CC"));
	//ledLayout->addWidget(ccLed, 1, 1, Qt::AlignLeft);

	ovpLed = new widgets::Led(
		&devices::HardwareDevice::is_ovp_active,
		&devices::HardwareDevice::has_ovp, device_,
		tr("OVP"), red_icon, grey_icon, grey_icon);
	infoLayout->addWidget(ovpLed, 0, 2, Qt::AlignLeft);
	ocpLed = new widgets::Led(
		&devices::HardwareDevice::is_ocp_active,
		&devices::HardwareDevice::has_ocp, device_,
		tr("OCP"), red_icon, grey_icon, grey_icon);
	infoLayout->addWidget(ocpLed, 1, 2, Qt::AlignLeft);
	otpLed = new widgets::Led(
		&devices::HardwareDevice::is_otp_active,
		&devices::HardwareDevice::has_otp, device_,
		tr("OTP"), red_icon, grey_icon, grey_icon);
	infoLayout->addWidget(otpLed, 0, 3, Qt::AlignLeft);
	uvcLed = new widgets::Led(
		&devices::HardwareDevice::is_uvc_active,
		&devices::HardwareDevice::has_uvc, device_,
		tr("UVC"), red_icon, grey_icon, grey_icon);
	infoLayout->addWidget(uvcLed, 1, 3, Qt::AlignLeft);
	layout->addLayout(infoLayout, 0);

	QHBoxLayout *ctrlLayout = new QHBoxLayout();

	// TODO: generic (CV, CC, CP, CR)
	device_->list_current_limit(min, max, step);
	setValueControl = new widgets::ValueControl(
		tr("Current"), 5, tr("A"), min, max, step);
	ctrlLayout->addWidget(setValueControl, 1, Qt::AlignLeft);
	layout->addLayout(ctrlLayout, 0);

	QHBoxLayout *optCtrlLayout = new QHBoxLayout();

	min = max = step = 0;
	if (device_->is_ovp_threshold_listable())
		device_->list_ovp_threshold(min, max, step);
	ovpControl = new widgets::OptionalValueControl(
		&devices::HardwareDevice::get_ovp_enable,
		&devices::HardwareDevice::set_ovp_enable,
		&devices::HardwareDevice::is_ovp_enable_getable,
		&devices::HardwareDevice::is_ovp_enable_setable,
		&devices::HardwareDevice::get_ovp_threshold,
		&devices::HardwareDevice::set_ovp_threshold,
		&devices::HardwareDevice::is_ovp_threshold_getable,
		&devices::HardwareDevice::is_ovp_threshold_setable,
		device_, tr("OVP"), tr("V"), min, max, step);
	optCtrlLayout->addWidget(ovpControl);

	min = max = step = 0;
	if (device_->is_ocp_threshold_listable())
		device_->list_ocp_threshold(min, max, step);
	ocpControl = new widgets::OptionalValueControl(
		&devices::HardwareDevice::get_ocp_enable,
		&devices::HardwareDevice::set_ocp_enable,
		&devices::HardwareDevice::is_ocp_enable_getable,
		&devices::HardwareDevice::is_ocp_enable_setable,
		&devices::HardwareDevice::get_ocp_threshold,
		&devices::HardwareDevice::set_ocp_threshold,
		&devices::HardwareDevice::is_ocp_threshold_getable,
		&devices::HardwareDevice::is_ocp_threshold_setable,
		device_, tr("OCP"), tr("C"), min, max, step);
	optCtrlLayout->addWidget(ocpControl);

	min = max = step = 0;
	if (device_->is_uvc_threshold_listable())
		device_->list_uvc_threshold(min, max, step);
	uvcControl = new widgets::OptionalValueControl(
		&devices::HardwareDevice::get_uvc_enable,
		&devices::HardwareDevice::set_uvc_enable,
		&devices::HardwareDevice::is_uvc_enable_getable,
		&devices::HardwareDevice::is_uvc_enable_setable,
		&devices::HardwareDevice::get_uvc_threshold,
		&devices::HardwareDevice::set_uvc_threshold,
		&devices::HardwareDevice::is_uvc_threshold_getable,
		&devices::HardwareDevice::is_uvc_threshold_setable,
		device_, tr("UVC"), tr("V"), min, max, step);
	optCtrlLayout->addWidget(uvcControl, 1, Qt::AlignLeft);
	layout->addLayout(optCtrlLayout, 0);
	layout->addStretch(1);

	this->setLayout(layout);
}

void SinkControlView::connect_signals()
{
	// Control elements -> Device
	connect(setEnableButton, SIGNAL(state_changed(const bool)),
		this, SLOT(on_enable_changed(const bool)));
	connect(setValueControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_value_changed(const double)));
	connect(ovpControl, SIGNAL(state_changed(const bool)),
		this, SLOT(on_ovp_enable_changed(const bool)));
	connect(ovpControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_ovp_threshold_changed(const double)));
	connect(ocpControl, SIGNAL(state_changed(const bool)),
		this, SLOT(on_ocp_enable_changed(const bool)));
	connect(ocpControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_ocp_threshold_changed(const double)));
	connect(uvcControl, SIGNAL(state_changed(const bool)),
		this, SLOT(on_uvc_enable_changed(const bool)));
	connect(uvcControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_uvc_threshold_changed(const double)));

	// Device -> Control elements
	connect(device_.get(), SIGNAL(enabled_changed(const bool)),
		setEnableButton, SLOT(on_state_changed(const bool)));
	connect(device_.get(), SIGNAL(current_limit_changed(const double)),
		setValueControl, SLOT(change_value(const double)));
	connect(device_.get(), SIGNAL(ovp_enable_changed(const bool)),
		ovpControl, SLOT(change_state(const bool)));
	connect(device_.get(), SIGNAL(ovp_threshold_changed(const double)),
		ovpControl, SLOT(change_value(const double)));
	connect(device_.get(), SIGNAL(ocp_enable_changed(const bool)),
		ocpControl, SLOT(change_state(const bool)));
	connect(device_.get(), SIGNAL(ocp_threshold_changed(const double)),
		ocpControl, SLOT(change_value(const double)));
	connect(device_.get(), SIGNAL(uvc_enable_changed(const bool)),
		uvcControl, SLOT(change_state(const bool)));
	connect(device_.get(), SIGNAL(uvc_threshold_changed(const double)),
		uvcControl, SLOT(change_value(const double)));

	// Device -> LEDs
	connect(device_.get(), SIGNAL(ovp_active_changed(const bool)),
		ovpLed, SLOT(on_state_changed(const bool)));
	connect(device_.get(), SIGNAL(ocp_active_changed(const bool)),
		ocpLed, SLOT(on_state_changed(const bool)));
	connect(device_.get(), SIGNAL(uvc_active_changed(const bool)),
		uvcLed, SLOT(on_state_changed(const bool)));
	connect(device_.get(), SIGNAL(otp_active_changed(const bool)),
		otpLed, SLOT(on_state_changed(const bool)));
}

void SinkControlView::init_values()
{
	// TODO: move to button (or whatever...)
	if (device_->is_enable_getable())
		setEnableButton->on_state_changed(device_->get_enable());
	if (device_->is_current_limit_getable())
		setValueControl->on_value_changed(device_->get_current_limit());
	if (device_->is_ovp_enable_getable())
		ovpControl->on_state_changed(device_->get_ovp_enable());
	if (device_->is_ovp_threshold_getable())
		ovpControl->on_value_changed(device_->get_ovp_threshold());
	if (device_->is_ocp_enable_getable())
		ocpControl->on_state_changed(device_->get_ocp_enable());
	if (device_->is_ocp_threshold_getable())
		ocpControl->on_value_changed(device_->get_ocp_threshold());
	if (device_->is_uvc_enable_getable())
		uvcControl->on_state_changed(device_->get_uvc_enable());
	if (device_->is_uvc_threshold_getable())
		uvcControl->on_value_changed(device_->get_uvc_threshold());

	// LEDs
	if (device_->has_ovp())
		ovpLed->on_state_changed(device_->is_ovp_active());
	if (device_->has_ocp())
		ocpLed->on_state_changed(device_->is_ocp_active());
	if (device_->has_uvc())
		uvcLed->on_state_changed(device_->is_uvc_active());
	if (device_->has_otp())
		otpLed->on_state_changed(device_->is_otp_active());
}

void SinkControlView::on_enable_changed(const bool enable)
{
	device_->set_enable(enable);
}

void SinkControlView::on_value_changed(const double value)
{
	device_->set_current_limit(value);
}

void SinkControlView::on_ovp_enable_changed(const bool enable)
{
	device_->set_ovp_enable(enable);
}

void SinkControlView::on_ovp_threshold_changed(const double value)
{
	device_->set_ovp_threshold(value);
}

void SinkControlView::on_ocp_enable_changed(const bool enable)
{
	device_->set_ocp_enable(enable);
}

void SinkControlView::on_ocp_threshold_changed(const double value)
{
	device_->set_ocp_threshold(value);
}

void SinkControlView::on_uvc_enable_changed(const bool enable)
{
	device_->set_uvc_enable(enable);
}

void SinkControlView::on_uvc_threshold_changed(const double value)
{
	device_->set_uvc_threshold(value);
}

} // namespace views
} // namespace sv

