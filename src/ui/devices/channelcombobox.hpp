/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DEVICES_CHANNELCOMBOBOX_HPP
#define UI_DEVICES_CHANNELCOMBOBOX_HPP

#include <memory>

#include <QComboBox>
#include <QString>
#include <QWidget>

#include "src/data/datautil.hpp"

using std::shared_ptr;

namespace sv {

namespace channels {
class BaseChannel;
}
namespace devices {
class BaseDevice;
}

namespace ui {
namespace devices {

class ChannelComboBox : public QComboBox
{
	Q_OBJECT

public:
	ChannelComboBox(
		shared_ptr<sv::devices::BaseDevice> device,
		const QString &channel_group, QWidget *parent = nullptr);

	void filter_quantity(sv::data::Quantity quantity);
	void select_channel(shared_ptr<sv::channels::BaseChannel> channel);
	shared_ptr<sv::channels::BaseChannel> selected_channel() const;

private:
	void setup_ui();
	void fill_channels();

	shared_ptr<sv::devices::BaseDevice> device_;
	QString channel_group_;
	bool filter_active_;
	sv::data::Quantity filter_quantity_;

public Q_SLOTS:
	void change_device_channel_group(
		shared_ptr<sv::devices::BaseDevice> device,
		const QString &channel_group);

};

} // namespace devices
} // namespace ui
} // namespace sv

#endif // UI_DEVICES_CHANNELCOMBOBOX_HPP
