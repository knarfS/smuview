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

#include <QComboBox>
#include <QDebug>
#include <QVariant>

#include "signalcombobox.hpp"
#include "src/data/basesignal.hpp"
#include "src/channels/basechannel.hpp"

using std::shared_ptr;

Q_DECLARE_METATYPE(shared_ptr<sv::data::BaseSignal>)

namespace sv {
namespace ui {
namespace devices {

SignalComboBox::SignalComboBox(
		shared_ptr<sv::channels::BaseChannel> channel,
		QWidget *parent) :
	QComboBox(parent),
	channel_(channel),
	filter_active_(false)
{
	setup_ui();
}

void SignalComboBox::filter_quantity(sv::data::Quantity quantity)
{
	filter_active_ = true;
	filter_quantity_ = quantity;
	// Refill combo box to apply filter.
	this->fill_signals();
}

void SignalComboBox::select_signal(shared_ptr<sv::data::BaseSignal> signal)
{
	for (int i = 0; i < this->count(); ++i) {
		QVariant data = this->itemData(i, Qt::UserRole);
		auto item_signal = data.value<shared_ptr<sv::data::BaseSignal>>();
		if (item_signal == signal) {
			this->setCurrentIndex(i);
			break;
		}
	}
}

shared_ptr<sv::data::BaseSignal> SignalComboBox::selected_signal() const
{
	QVariant data = this->currentData();
	return data.value<shared_ptr<sv::data::BaseSignal>>();
}

void SignalComboBox::setup_ui()
{
	this->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	this->fill_signals();
}

void SignalComboBox::fill_signals()
{
	this->clear();

	if (channel_ == nullptr)
		return;

	for (const auto &signal_pair : channel_->signal_map()) {
		for (const auto &signal : signal_pair.second) {
			if (filter_active_ && filter_quantity_ != signal->quantity())
				continue;
			this->addItem(signal->display_name(), QVariant::fromValue(signal));
		}
	}
}

void SignalComboBox::change_channel(
	shared_ptr<sv::channels::BaseChannel> channel)
{
	channel_ = channel;
	this->fill_signals();
}

} // namespace devices
} // namespace ui
} // namespace sv
