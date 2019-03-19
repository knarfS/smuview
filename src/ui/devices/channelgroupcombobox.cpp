/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <QComboBox>
#include <QDebug>
#include <QString>
#include <QVariant>

#include "channelgroupcombobox.hpp"
#include "src/channels/basechannel.hpp"
#include "src/devices/basedevice.hpp"

using std::shared_ptr;

namespace sv {
namespace ui {
namespace devices {

ChannelGroupComboBox::ChannelGroupComboBox(
		shared_ptr<sv::devices::BaseDevice> device,  QWidget *parent) :
	QComboBox(parent),
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

const QString ChannelGroupComboBox::selected_channel_group() const
{
	return this->currentText();
}

void ChannelGroupComboBox::setup_ui()
{
	this->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	this->fill_channel_groups();
}

void ChannelGroupComboBox::fill_channel_groups()
{
	this->clear();

	if (device_ == nullptr)
		return;

	for (const auto &chg_pair : device_->channel_group_map()) {
		this->addItem(QString::fromStdString(chg_pair.first));
	}
}

void ChannelGroupComboBox::change_device(
	shared_ptr<sv::devices::BaseDevice> device)
{
	device_ = device;
	this->fill_channel_groups();
}

} // namespace devices
} // namespace ui
} // namespace sv
