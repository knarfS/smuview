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

#include "led.hpp"

namespace sv {
namespace widgets {

	Led::Led(
		const bool is_state_getable,
		QString text, QIcon on_icon, QIcon off_icon, QIcon dis_icon,
		QWidget *parent) :
	QWidget(parent),
	is_state_getable_(is_state_getable),
	text_(text),
	on_icon_(on_icon),
	off_icon_(off_icon),
	dis_icon_(dis_icon)
{
	setup_ui();
}

void Led::setup_ui()
{
	QHBoxLayout *layout = new QHBoxLayout();

	led_label_ = new QLabel();
	led_label_->setPixmap(
		dis_icon_.pixmap(16, 16, QIcon::Mode::Disabled, QIcon::State::Off));
	layout->addWidget(led_label_);

	text_label_ = new QLabel(text_);
	text_label_->setDisabled(!is_state_getable_);
	layout->addWidget(text_label_);

	this->setLayout(layout);
}

void Led::change_state(const bool state)
{
	if (state) {
		led_label_->setPixmap(
			on_icon_.pixmap(16, 16, QIcon::Mode::Active, QIcon::State::On));
	}
	else {
		led_label_->setPixmap(
			off_icon_.pixmap(16, 16, QIcon::Mode::Active, QIcon::State::Off));
	}
}

} // namespace widgets
} // namespace sv

