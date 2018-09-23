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

#ifndef UI_DEVICES_SELECTSIGNALWIDGET_HPP
#define UI_DEVICES_SELECTSIGNALWIDGET_HPP

#include <memory>

#include <QWidget>

using std::shared_ptr;

namespace sv {

class Session;

namespace data {
class BaseSignal;
}

namespace devices {
class BaseDevice;
}

namespace ui {
namespace devices {

class ChannelComboBox;
class ChannelGroupComboBox;
class DeviceComboBox;
class SignalComboBox;

class SelectSignalWidget : public QWidget
{
	Q_OBJECT

public:
	SelectSignalWidget(const Session &session, QWidget *parent = nullptr);

	void select_device(shared_ptr<sv::devices::BaseDevice>);
	shared_ptr<sv::data::BaseSignal> selected_signal() const;

private:
	const Session &session_;

	DeviceComboBox *device_box_;
	ChannelGroupComboBox *channel_group_box_;
	ChannelComboBox *channel_box_;
	SignalComboBox *signal_box_;

	void setup_ui();
	void connect_signals();

private Q_SLOTS:
	void on_device_changed();
	void on_channel_group_changed();
	void on_channel_changed();

};

} // namespace devices
} // namespace ui
} // namespace sv

#endif // UI_DEVICES_SELECTSIGNALWIDGET_HPP

