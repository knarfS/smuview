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

#include <cassert>
#include <memory>
#include <string>

#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "adduserchanneldialog.hpp"
#include "src/channels/userchannel.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/data/quantitycombobox.hpp"
#include "src/ui/data/quantityflagslist.hpp"
#include "src/ui/data/unitcombobox.hpp"
#include "src/ui/devices/channelgroupcombobox.hpp"
#include "src/ui/devices/devicecombobox.hpp"

using std::shared_ptr;
using std::string;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace ui {
namespace dialogs {

AddUserChannelDialog::AddUserChannelDialog(const Session &session,
		shared_ptr<sv::devices::BaseDevice> device,
		QWidget *parent) :
	QDialog(parent),
	session_(session),
	device_(device)
{
	assert(device);

	setup_ui();
}

void AddUserChannelDialog::setup_ui()
{
	QIcon main_icon;
	main_icon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(main_icon);
	this->setWindowTitle(tr("Add User Channel"));
	this->setMinimumWidth(550);

	QVBoxLayout *main_layout = new QVBoxLayout();

	// General stuff
	QFormLayout *form_layout = new QFormLayout();
	name_edit_ = new QLineEdit();
	form_layout->addRow(tr("Name"), name_edit_);
	main_layout->addLayout(form_layout);

	// Measured Quantity
	QGroupBox *mq_group = new QGroupBox(tr("Measured Quantity"));
	QFormLayout *mq_layout = new QFormLayout();
	quantity_box_ = new ui::data::QuantityComboBox();
	mq_layout->addRow(tr("Quantity"), quantity_box_);
	quantity_flags_list_ = new ui::data::QuantityFlagsList();
	mq_layout->addRow(tr("Quantity Flags"), quantity_flags_list_);
	unit_box_ = new ui::data::UnitComboBox();
	mq_layout->addRow(tr("Unit"), unit_box_);
	mq_group->setLayout(mq_layout);
	main_layout->addWidget(mq_group);

	// Add to...
	QGroupBox *add_to_group = new QGroupBox(tr("Add to..."));
	QFormLayout *add_to_layout = new QFormLayout();
	device_box_ = new ui::devices::DeviceComboBox(session_);
	device_box_->select_device(device_);
	add_to_layout->addRow(tr("Device"), device_box_);
	channel_group_box_ = new ui::devices::ChannelGroupComboBox(device_);
	channel_group_box_->addItem(QString(tr("User")));
	connect(device_box_, &ui::devices::DeviceComboBox::device_changed,
		this, &AddUserChannelDialog::on_device_changed);
	add_to_layout->addRow(tr("Channel Group"), channel_group_box_);
	add_to_group->setLayout(add_to_layout);
	main_layout->addWidget(add_to_group);

	// Buttons
	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, &QDialogButtonBox::accepted,
		this, &AddUserChannelDialog::accept);
	connect(button_box_, &QDialogButtonBox::rejected,
		this, &AddUserChannelDialog::reject);

	this->setLayout(main_layout);
}

shared_ptr<channels::UserChannel> AddUserChannelDialog::channel()
{
	return channel_;
}

void AddUserChannelDialog::accept()
{
	if (name_edit_->text().size() == 0) {
		QMessageBox::warning(this,
			tr("Channel name missing"),
			tr("Please enter a name for the new channel."),
			QMessageBox::Ok);
		return;
	}

	auto device = device_box_->selected_device();

	channel_ = device->add_user_channel(
		name_edit_->text().toStdString(),
		channel_group_box_->selected_channel_group().toStdString());

	QDialog::accept();
}

void AddUserChannelDialog::on_device_changed()
{
	channel_group_box_->change_device(device_box_->selected_device());
}

} // namespace dialogs
} // namespace ui
} // namespace sv
