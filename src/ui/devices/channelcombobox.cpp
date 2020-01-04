/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2020 Frank Stettner <frank-stettner@gmx.net>
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
#include <string>
#include <vector>

#include <QComboBox>
#include <QDebug>
#include <QString>
#include <QVariant>

#include "channelcombobox.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"

using std::shared_ptr;
using std::string;
using std::vector;

Q_DECLARE_METATYPE(shared_ptr<sv::channels::BaseChannel>)

namespace sv {
namespace ui {
namespace devices {

ChannelComboBox::ChannelComboBox(
		shared_ptr<sv::devices::BaseDevice> device,
		QString channel_group_name, QWidget *parent) :
	QComboBox(parent),
	device_(device),
	channel_group_name_(channel_group_name),
	filter_active_(false)
{
	setup_ui();
}

void ChannelComboBox::filter_quantity(sv::data::Quantity quantity)
{
	filter_active_ = true;
	filter_quantity_ = quantity;
	// Refill combo box to apply filter.
	this->fill_channels();
}

void ChannelComboBox::select_channel(
	shared_ptr<sv::channels::BaseChannel> channel)
{
	for (int i = 0; i < this->count(); ++i) {
		QVariant data = this->itemData(i, Qt::UserRole);
		auto item_channel = data.value<shared_ptr<sv::channels::BaseChannel>>();
		if (item_channel == channel) {
			this->setCurrentIndex(i);
			break;
		}
	}
}

shared_ptr<sv::channels::BaseChannel> ChannelComboBox::selected_channel() const
{
	QVariant data = this->currentData();
	return data.value<shared_ptr<sv::channels::BaseChannel>>();
}

void ChannelComboBox::setup_ui()
{
	this->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	this->fill_channels();
}

void ChannelComboBox::fill_channels()
{
	this->clear();

	if (device_ == nullptr)
		return;

	if (channel_group_name_ == nullptr)
		channel_group_name_ = "";

	string chg_name_str = channel_group_name_.toStdString();
	if (!device_->channel_group_map().count(chg_name_str))
		return;

	auto ch_list = device_->channel_group_map()[chg_name_str];
	for (const auto &ch : ch_list) {
		// Check if channel contains a signal with the filter quantity.
		if (filter_active_) {
			bool found = false;
			for (const auto &signal : ch->signals()) {
				if (filter_quantity_ == signal->quantity()) {
					found = true;
					break;
				}
			}
			if (!found)
				continue;
		}

		this->addItem(
			QString::fromStdString(ch->name()),
			QVariant::fromValue(ch));
	}
}

void ChannelComboBox::change_device_channel_group(
	shared_ptr<sv::devices::BaseDevice> device, QString channel_group_name)
{
	device_ = device;
	channel_group_name_ = channel_group_name;
	this->fill_channels();
}

} // namespace devices
} // namespace ui
} // namespace sv
