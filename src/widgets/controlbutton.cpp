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

namespace sv {
namespace widgets {

	ControlButton::ControlButton(
		const bool is_state_getable, const bool is_state_setable,
		QWidget *parent) :
	QPushButton(parent),
	state_(false),
	is_state_getable_(is_state_getable),
	is_state_setable_(is_state_setable),
	on_icon_(":/icons/status-green.svg"),
	off_icon_(":/icons/status-red.svg"),
	dis_icon_(":/icons/status-grey.svg")
{
	is_state_enabled_ = is_state_getable_ || is_state_setable_;

	setup_ui();
	connect_signals();
}

void ControlButton::setup_ui()
{
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	this->setIconSize(QSize(8, 8));
	this->setDisabled(!is_state_setable_);
	if (!is_state_getable_) {
		this->setIcon(dis_icon_);
		this->setText(tr("On/Off"));
		this->setChecked(false);
	}
	else {
		if (state_) {
			this->setIcon(on_icon_);
			this->setText(tr("On"));
			this->setChecked(true);
		}
		else {
			this->setIcon(off_icon_);
			this->setText(tr("Off"));
			this->setChecked(false);
		}
	}
}

void ControlButton::connect_signals()
{
	if (is_state_setable_)
		connect(this, SIGNAL(clicked(bool)), this, SLOT(on_state_changed()));
}

void ControlButton::change_state(const bool state)
{
	if (state) {
		this->setIcon(on_icon_);
		this->setText(tr("On"));
		this->setChecked(true);
		state_ = true;
	} else {
		this->setIcon(off_icon_);
		this->setText(tr("Off"));
		this->setChecked(false);
		state_ = false;
	}
}

void ControlButton::on_state_changed()
{
	change_state(!state_);
	Q_EMIT state_changed(state_);
}

} // namespace widgets
} // namespace sv

