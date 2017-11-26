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

namespace sv {
namespace widgets {

Led::Led(const bool state, const bool is_getable, QString text,
		QWidget *parent) :
	QWidget(state),
	state_(false),
	is_getable_(is_getable),
	text_(text),
	icon_red_(":/icons/status-red.svg"),
	icon_green_(":/icons/status-green.svg"),
	icon_grey_(":/icons/status-grey.svg")
{
	setup_ui();
}

void Led::setup_ui()
{
	QHBoxLayout *layout = new QHBoxLayout();

	ledLabel_ = new QLabel();
	ledLabel_->setPixmap(icon_grey_.pixmap(16, 16, QIcon::Mode::Disabled, QIcon::State::Off));
	layout->addWidget(ledLabel_);

	textLabel_ = new QLabel(text_);
	layout->addWidget(textLabel_);

	this->setLayout(layout);
}

void Led::on_state_changed(const bool enabled)
{
	qWarning() << "Led::on_state_changed(): enabled = " << enabled;
	if (enabled) {
		qWarning() << "Led::on_state_changed(): led = green";
		ledLabel_->setPixmap(icon_green_.pixmap(16, 16, QIcon::Mode::Active, QIcon::State::On));
		state_ = true;
	} else {
		qWarning() << "Led::on_state_changed(): led = red";
		ledLabel_->setPixmap(icon_red_.pixmap(16, 16, QIcon::Mode::Active, QIcon::State::Off));
		state_ = false;
	}
}

} // namespace widgets
} // namespace sv

