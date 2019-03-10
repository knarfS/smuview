/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <map>
#include <memory>

#include <QDebug>
#include <QString>

#include "channelgroupcombobox.hpp"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/devices/basedevice.hpp"

using std::shared_ptr;

namespace sv {
namespace ui {
namespace devices {

ChannelGroupComboBox::ChannelGroupComboBox(const Session &session,
		shared_ptr<sv::devices::BaseDevice> device,  QWidget *parent) :
	QComboBox(parent),
	session_(session),
	device_(device)
{
	this->setup_ui();
}

void ChannelGroupComboBox::select_channel_group(QString channel_group)
{
	for (int i = 0; i < this->count(); ++i) {
		QString cg = this->itemText(i);
		if (cg == channel_group) {
			this->setCurrentIndex(i);
			break;
		}
	}
}

const QString ChannelGroupComboBox::selected_channel_group()
{
	return this->currentText();
}

void ChannelGroupComboBox::setup_ui()
{
	if (device_) {
		for (const auto &cg_name_map : device_->channel_group_name_map()) {
			this->addItem(QString::fromStdString(cg_name_map.first));
		}
	}
}

void ChannelGroupComboBox::change_device(
	shared_ptr<sv::devices::BaseDevice> device)
{
	device_ = device;
	for (int i = this->count(); i >= 0; --i) {
		this->removeItem(i);
	}
	this->setup_ui();
}

} // namespace devices
} // namespace ui
} // namespace sv
