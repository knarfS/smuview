/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DIALOGS_ADDUSERCHANNELDIALOG_HPP
#define UI_DIALOGS_ADDUSERCHANNELDIALOG_HPP

#include <memory>

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>

#include "src/session.hpp"

using std::shared_ptr;

namespace sv {

namespace devices {
class BaseDevice;
}
namespace channels {
class UserChannel;
}

namespace ui {

namespace data {
class QuantityComboBox;
class QuantityFlagsList;
class UnitComboBox;
}
namespace devices {
class ChannelGroupComboBox;
class DeviceComboBox;
}

namespace dialogs {

class AddUserChannelDialog : public QDialog
{
	Q_OBJECT

public:
	AddUserChannelDialog(const Session &session,
		shared_ptr<sv::devices::BaseDevice> device,
		QWidget *parent = nullptr);

	shared_ptr<channels::UserChannel> channel();

private:
	void setup_ui();

	const Session &session_;
	shared_ptr<sv::devices::BaseDevice> device_;
	shared_ptr<channels::UserChannel> channel_;

	QLineEdit *name_edit_;
	ui::data::QuantityComboBox *quantity_box_;
	ui::data::QuantityFlagsList *quantity_flags_list_;
	ui::data::UnitComboBox *unit_box_;
	ui::devices::DeviceComboBox *device_box_;
	ui::devices::ChannelGroupComboBox *channel_group_box_;
	QDialogButtonBox *button_box_;

public Q_SLOTS:
	void accept() override;

private Q_SLOTS:
	void on_device_changed();

};

} // namespace dialogs
} // namespace ui
} // namespace sv

#endif // UI_DIALOGS_ADDUSERCHANNELDIALOG_HPP
