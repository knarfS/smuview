/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "devicetab.hpp"
#include "src/session.hpp"

using std::shared_ptr;

namespace sv {
namespace tabs {

DeviceTab::DeviceTab(Session &session,
		shared_ptr<devices::HardwareDevice> device, QMainWindow *parent) :
		BaseTab(session, parent),
	device_(device)
{
	connect(&session_, SIGNAL(signals_changed()),
		this, SLOT(signals_changed()));
	connect(&session_, SIGNAL(capture_state_changed(int)),
		this, SLOT(capture_state_updated(int)));
}

void DeviceTab::clear_signals()
{
}

unordered_set< shared_ptr<data::SignalBase> > DeviceTab::signalbases() const
{
	return signalbases_;
}

void DeviceTab::clear_signalbases()
{
	for (shared_ptr<data::SignalBase> signalbase : signalbases_) {
		disconnect(signalbase.get(), SIGNAL(samples_cleared()),
			this, SLOT(on_data_updated()));
		disconnect(signalbase.get(), SIGNAL(samples_added(QObject*, uint64_t, uint64_t)),
			this, SLOT(on_data_updated()));
	}

	signalbases_.clear();
}

void DeviceTab::add_signalbase(const shared_ptr<data::SignalBase> signalbase)
{
	signalbases_.insert(signalbase);

	connect(signalbase.get(), SIGNAL(samples_cleared()),
		this, SLOT(on_data_updated()));
	connect(signalbase.get(), SIGNAL(samples_added(QObject*, uint64_t, uint64_t)),
		this, SLOT(on_data_updated()));
}

} // namespace tabs
} // namespace sv
