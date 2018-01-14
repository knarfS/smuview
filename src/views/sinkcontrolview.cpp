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
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "sinkcontrolview.hpp"
#include "src/session.hpp"
#include "src/devices/configurable.hpp"
#include "src/widgets/controlbutton.hpp"
#include "src/widgets/led.hpp"
#include "src/widgets/optionalvaluecontrol.hpp"
#include "src/widgets/valuecontrol.hpp"

namespace sv {
namespace views {

SinkControlView::SinkControlView(const Session &session,
	shared_ptr<devices::Configurable> configurable, QWidget *parent) :
		BaseView(session, parent),
	configurable_(configurable)
{
	setup_ui();
	connect_signals();
	init_values();
}

QString SinkControlView::title() const
{
	return configurable_->name() + " " + tr("Control");
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
	enableButton = new widgets::ControlButton(
		configurable_->is_enabled_getable(), configurable_->is_enabled_setable());
	infoLayout->addWidget(enableButton, 0, 0, 2, 1,  Qt::AlignLeft);

	// Regulation Leds
	//cvLed = new widgets::Led(false, false, tr("CV"));
	//ledLayout->addWidget(cvLed, 0, 1, Qt::AlignLeft);
	//ccLed = new widgets::Led(true, false, tr("CC"));
	//ledLayout->addWidget(ccLed, 1, 1, Qt::AlignLeft);

	ovpLed = new widgets::Led(configurable_->is_ovp_active_getable(),
		tr("OVP"), red_icon, grey_icon, grey_icon);
	infoLayout->addWidget(ovpLed, 0, 2, Qt::AlignLeft);
	ocpLed = new widgets::Led(configurable_->is_ocp_active_getable(),
		tr("OCP"), red_icon, grey_icon, grey_icon);
	infoLayout->addWidget(ocpLed, 1, 2, Qt::AlignLeft);
	otpLed = new widgets::Led(configurable_->is_otp_active_getable(),
		tr("OTP"), red_icon, grey_icon, grey_icon);
	infoLayout->addWidget(otpLed, 0, 3, Qt::AlignLeft);
	uvcLed = new widgets::Led(configurable_->is_uvc_active_getable(),
		tr("UVC"), red_icon, grey_icon, grey_icon);
	infoLayout->addWidget(uvcLed, 1, 3, Qt::AlignLeft);
	layout->addLayout(infoLayout, 0);

	QHBoxLayout *ctrlLayout = new QHBoxLayout();

	// TODO: generic (CV, CC, CP, CR)
	configurable_->list_current_limit(min, max, step);
	setValueControl = new widgets::ValueControl(
		tr("Current"), 5, tr("A"), min, max, step);
	ctrlLayout->addWidget(setValueControl);

	QStringList regulation_list;
	configurable_->list_regulation(regulation_list);
	regulationBox = new QComboBox();
	regulationBox->addItems(regulation_list);
	/*
	regulationBox->addItem("CV");
	regulationBox->addItem("CC");
	regulationBox->addItem("CR");
	regulationBox->addItem("CP");
	*/
	ctrlLayout->addWidget(regulationBox, 1, Qt::AlignLeft);
	layout->addLayout(ctrlLayout, 0);

	QHBoxLayout *optCtrlLayout = new QHBoxLayout();

	min = max = step = 0;
	if (configurable_->is_ovp_threshold_listable())
		configurable_->list_ovp_threshold(min, max, step);
	ovpControl = new widgets::OptionalValueControl(
		configurable_->is_ovp_enabled_getable(),
		configurable_->is_ovp_enabled_setable(),
		configurable_->is_ovp_threshold_getable(),
		configurable_->is_ovp_threshold_setable(),
		tr("OVP"), tr("V"), min, max, step);
	optCtrlLayout->addWidget(ovpControl);

	min = max = step = 0;
	if (configurable_->is_ocp_threshold_listable())
		configurable_->list_ocp_threshold(min, max, step);
	ocpControl = new widgets::OptionalValueControl(
		configurable_->is_ocp_enabled_getable(),
		configurable_->is_ocp_enabled_setable(),
		configurable_->is_ocp_threshold_getable(),
		configurable_->is_ocp_threshold_setable(),
		tr("OCP"), tr("A"), min, max, step);
	optCtrlLayout->addWidget(ocpControl);

	min = max = step = 0;
	if (configurable_->is_uvc_threshold_listable())
		configurable_->list_uvc_threshold(min, max, step);
	uvcControl = new widgets::OptionalValueControl(
		configurable_->is_uvc_enabled_getable(),
		configurable_->is_uvc_enabled_setable(),
		configurable_->is_uvc_threshold_getable(),
		configurable_->is_uvc_threshold_setable(),
		tr("UVC"), tr("V"), min, max, step);
	optCtrlLayout->addWidget(uvcControl, 1, Qt::AlignLeft);
	layout->addLayout(optCtrlLayout, 0);
	layout->addStretch(1);

	this->centralWidget_->setLayout(layout);
}

void SinkControlView::connect_signals()
{
	// Control elements -> Device
	connect(enableButton, SIGNAL(state_changed(const bool)),
		this, SLOT(on_enabled_changed(const bool)));
	// Regulation
	connect(setValueControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_value_changed(const double)));

	connect(ovpControl, SIGNAL(state_changed(const bool)),
		this, SLOT(on_ovp_enabled_changed(const bool)));
	connect(ovpControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_ovp_threshold_changed(const double)));
	connect(ocpControl, SIGNAL(state_changed(const bool)),
		this, SLOT(on_ocp_enabled_changed(const bool)));
	connect(ocpControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_ocp_threshold_changed(const double)));
	connect(uvcControl, SIGNAL(state_changed(const bool)),
		this, SLOT(on_uvc_enabled_changed(const bool)));
	connect(uvcControl, SIGNAL(value_changed(const double)),
		this, SLOT(on_uvc_threshold_changed(const double)));

	// Device -> Control elements
	connect(configurable_.get(), SIGNAL(enabled_changed(const bool)),
		enableButton, SLOT(change_state(const bool)));
	// TODO: Regulation
	connect(configurable_.get(), SIGNAL(current_limit_changed(const double)),
		setValueControl, SLOT(change_value(const double)));
	connect(configurable_.get(), SIGNAL(ovp_enabled_changed(const bool)),
		ovpControl, SLOT(change_state(const bool)));
	connect(configurable_.get(), SIGNAL(ovp_threshold_changed(const double)),
		ovpControl, SLOT(change_value(const double)));
	connect(configurable_.get(), SIGNAL(ocp_enabled_changed(const bool)),
		ocpControl, SLOT(change_state(const bool)));
	connect(configurable_.get(), SIGNAL(ocp_threshold_changed(const double)),
		ocpControl, SLOT(change_value(const double)));
	connect(configurable_.get(), SIGNAL(uvc_enabled_changed(const bool)),
		uvcControl, SLOT(change_state(const bool)));
	connect(configurable_.get(), SIGNAL(uvc_threshold_changed(const double)),
		uvcControl, SLOT(change_value(const double)));

	// Device -> LEDs
	connect(configurable_.get(), SIGNAL(ovp_active_changed(const bool)),
		ovpLed, SLOT(change_state(const bool)));
	connect(configurable_.get(), SIGNAL(ocp_active_changed(const bool)),
		ocpLed, SLOT(change_state(const bool)));
	connect(configurable_.get(), SIGNAL(uvc_active_changed(const bool)),
		uvcLed, SLOT(change_state(const bool)));
	connect(configurable_.get(), SIGNAL(otp_active_changed(const bool)),
		otpLed, SLOT(change_state(const bool)));
}

void SinkControlView::init_values()
{
	if (configurable_->is_enabled_getable())
		enableButton->change_state(configurable_->get_enabled());

	//if (configurable_->is_regulation_getable())
	//	qWarning() << "SinkControlView::init_values(): Regulation = " << configurable_->get_regulation();

	if (configurable_->is_current_limit_getable())
		setValueControl->change_value(configurable_->get_current_limit());
	if (configurable_->is_ovp_enabled_getable())
		ovpControl->change_state(configurable_->get_ovp_enabled());
	if (configurable_->is_ovp_threshold_getable())
		ovpControl->change_value(configurable_->get_ovp_threshold());
	if (configurable_->is_ocp_enabled_getable())
		ocpControl->change_state(configurable_->get_ocp_enabled());
	if (configurable_->is_ocp_threshold_getable())
		ocpControl->change_value(configurable_->get_ocp_threshold());
	if (configurable_->is_uvc_enabled_getable())
		uvcControl->change_state(configurable_->get_uvc_enabled());
	if (configurable_->is_uvc_threshold_getable())
		uvcControl->change_value(configurable_->get_uvc_threshold());

	// LEDs
	if (configurable_->is_ovp_active_getable())
		ovpLed->change_state(configurable_->get_ovp_active());
	if (configurable_->is_ocp_active_getable())
		ocpLed->change_state(configurable_->get_ocp_active());
	if (configurable_->is_otp_active_getable())
		otpLed->change_state(configurable_->get_otp_active());
	if (configurable_->is_uvc_active_getable())
		uvcLed->change_state(configurable_->get_uvc_active());
}

void SinkControlView::on_enabled_changed(const bool enabled)
{
	configurable_->set_enabled(enabled);
}

void SinkControlView::on_value_changed(const double value)
{
	configurable_->set_current_limit(value);
}

void SinkControlView::on_ovp_enabled_changed(const bool enabled)
{
	configurable_->set_ovp_enabled(enabled);
}

void SinkControlView::on_ovp_threshold_changed(const double value)
{
	configurable_->set_ovp_threshold(value);
}

void SinkControlView::on_ocp_enabled_changed(const bool enabled)
{
	configurable_->set_ocp_enabled(enabled);
}

void SinkControlView::on_ocp_threshold_changed(const double value)
{
	configurable_->set_ocp_threshold(value);
}

void SinkControlView::on_uvc_enabled_changed(const bool enabled)
{
	configurable_->set_uvc_enabled(enabled);
}

void SinkControlView::on_uvc_threshold_changed(const double value)
{
	configurable_->set_uvc_threshold(value);
}

} // namespace views
} // namespace sv

