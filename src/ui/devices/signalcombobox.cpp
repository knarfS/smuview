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

#include <cassert>
#include <map>
#include <memory>
#include <vector>

#include <QDebug>
#include <QVariant>

#include "signalcombobox.hpp"
#include "src/session.hpp"
#include "src/data/basesignal.hpp"
#include "src/channels/basechannel.hpp"

using std::shared_ptr;
using std::vector;

Q_DECLARE_METATYPE(shared_ptr<sv::data::BaseSignal>)

namespace sv {
namespace ui {
namespace devices {

SignalComboBox::SignalComboBox(
		const Session &session, shared_ptr<sv::channels::BaseChannel> channel,
		QWidget *parent) :
	QComboBox(parent),
	session_(session),
	channel_(channel)
{
	setup_ui();
}

void SignalComboBox::select_signal(
	shared_ptr<sv::data::BaseSignal> signal)
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
	assert(channel_);

	for (auto signal_pair : channel_->signal_map()) {
		auto signal = signal_pair.second;
		this->addItem(
			signal->name(), QVariant::fromValue(signal));
	}
}

} // namespace devices
} // namespace ui
} // namespace sv
