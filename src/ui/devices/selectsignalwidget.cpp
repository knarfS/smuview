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

#include <memory>

#include <QDebug>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include "selectsignalwidget.hpp"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/devices/channelcombobox.hpp"
#include "src/ui/devices/channelgroupcombobox.hpp"
#include "src/ui/devices/devicecombobox.hpp"
#include "src/ui/devices/signalcombobox.hpp"

using std::shared_ptr;

Q_DECLARE_METATYPE(shared_ptr<sv::data::BaseSignal>)

namespace sv {
namespace ui {
namespace devices {

SelectSignalWidget::SelectSignalWidget(
		const Session &session, QWidget *parent) :
	QWidget(parent),
	session_(session)
{
	setup_ui();
	connect_signals();
}

void SelectSignalWidget::filter_quantity(sv::data::Quantity quantity)
{
	// NOTE: First filter the signal box and then the channel box for
	//       signal/slots to work correctly!
	signal_box_->filter_quantity(quantity);
	channel_box_->filter_quantity(quantity);
}

void SelectSignalWidget::select_device(
	shared_ptr<sv::devices::BaseDevice> device)
{
	device_box_->select_device(device);
}

shared_ptr<sv::data::BaseSignal> SelectSignalWidget::selected_signal() const
{
	return signal_box_->selected_signal();
}

void SelectSignalWidget::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	device_box_ = new DeviceComboBox(session_);
	layout->addWidget(device_box_);

	channel_group_box_ = new ChannelGroupComboBox(
		device_box_->selected_device());
	layout->addWidget(channel_group_box_);

	channel_box_ = new ChannelComboBox(
		device_box_->selected_device(),
		channel_group_box_->selected_channel_group());
	layout->addWidget(channel_box_);

	signal_box_ = new SignalComboBox(channel_box_->selected_channel());
	layout->addWidget(signal_box_);

	this->setLayout(layout);
}

void SelectSignalWidget::connect_signals()
{
	connect(device_box_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_device_changed()));
	connect(channel_group_box_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_channel_group_changed()));
	connect(channel_box_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_channel_changed()));
}

void SelectSignalWidget::on_device_changed()
{
	channel_group_box_->change_device(device_box_->selected_device());
}

void SelectSignalWidget::on_channel_group_changed()
{
	channel_box_->change_device_channel_group(
		device_box_->selected_device(),
		channel_group_box_->selected_channel_group());
}

void SelectSignalWidget::on_channel_changed()
{
	signal_box_->change_channel(channel_box_->selected_channel());
}

} // namespace devices
} // namespace ui
} // namespace sv
