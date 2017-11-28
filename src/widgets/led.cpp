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

#include <QHBoxLayout>
#include <QDebug>

#include "led.hpp"
#include "src/devices/hardwaredevice.hpp"

using std::shared_ptr;

namespace sv {
namespace widgets {

	Led::Led(
		bool (devices::HardwareDevice::*get_state_caller)() const,
		bool (devices::HardwareDevice::*is_getable_caller)() const,
		shared_ptr<devices::HardwareDevice> device,
		QString text, QIcon on_icon, QIcon off_icon, QIcon dis_icon,
		QWidget *parent) :
	QWidget(parent),
	get_state_caller_(get_state_caller),
	is_getable_caller_(is_getable_caller),
	device_(device),
	text_(text),
	on_icon_(on_icon),
	off_icon_(off_icon),
	dis_icon_(dis_icon)
{
	is_enabled_ = (device_.get()->*is_getable_caller_)();
	if (is_enabled_)
		state_ = (device_.get()->*get_state_caller_)();
	else
		state_ = false;

	setup_ui();
}

void Led::setup_ui()
{
	QHBoxLayout *layout = new QHBoxLayout();

	led_label_ = new QLabel();
	// TODO: get_state
	led_label_->setPixmap(
		dis_icon_.pixmap(16, 16, QIcon::Mode::Disabled, QIcon::State::Off));
	layout->addWidget(led_label_);

	text_label_ = new QLabel(text_);
	text_label_->setDisabled(!is_enabled_);
	layout->addWidget(text_label_);

	this->setLayout(layout);
}

void Led::on_state_changed(const bool enabled)
{
	if (enabled) {
		led_label_->setPixmap(
			on_icon_.pixmap(16, 16, QIcon::Mode::Active, QIcon::State::On));
		state_ = true;
	}
	else {
		led_label_->setPixmap(
			off_icon_.pixmap(16, 16, QIcon::Mode::Active, QIcon::State::Off));
		state_ = false;
	}
}

} // namespace widgets
} // namespace sv

