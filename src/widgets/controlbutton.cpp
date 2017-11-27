/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#include <QApplication>

#include "controlbutton.hpp"
#include "src/devices/hardwaredevice.hpp"

using std::shared_ptr;

namespace sv {
namespace widgets {

	ControlButton::ControlButton(
		bool (devices::HardwareDevice::*get_state_caller)() const,
		void (devices::HardwareDevice::*set_state_caller)(double),
		bool (devices::HardwareDevice::*is_getable_caller)() const,
		bool (devices::HardwareDevice::*is_setable_caller)() const,
		shared_ptr<devices::HardwareDevice> device, QWidget *parent) :
	QPushButton(parent),
	get_state_caller_(get_state_caller),
	set_state_caller_(set_state_caller),
	is_getable_caller_(is_getable_caller),
	is_setable_caller_(is_setable_caller),
	device_(device),
	icon_red_(":/icons/status-red.svg"),
	icon_green_(":/icons/status-green.svg"),
	icon_grey_(":/icons/status-grey.svg")
{
	setup_ui();

	if ((device_.get()->*is_setable_caller_)())
		connect(this, SIGNAL(clicked(bool)), this, SLOT(on_clicked()));
}

void ControlButton::setup_ui()
{
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	this->setMaximumSize(80, 50);

	this->setIconSize(QSize(8, 8));

	if (!is_setable_) {
		this->setDisabled(true);
	}

	if (!is_readable_) {
		this->setIcon(icon_grey_);
		this->setText(tr("On/Off"));
		this->setChecked(false);
	}
}

void ControlButton::on_clicked()
{
	on_state_changed(!state_);
	Q_EMIT state_changed(state_);
}

void ControlButton::on_state_changed(const bool enabled)
{
	if (enabled) {
		this->setIcon(icon_green_);
		this->setText(QApplication::translate("SmuView", "On", Q_NULLPTR));
		this->setChecked(true);
		state_ = true;
	} else {
		this->setIcon(icon_red_);
		this->setText(QApplication::translate("SmuView", "Off", Q_NULLPTR));
		this->setChecked(false);
		state_ = false;
	}
}

} // namespace widgets
} // namespace sv

