/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2016 Soeren Apel <soeren@apelpie.net>
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

#include "viewbase.hpp"
#include "src/session.hpp"
#include "src/util.hpp"

using std::shared_ptr;

namespace sv {
namespace views {

const int ViewBase::MaxViewAutoUpdateRate = 25; // No more than 25 Hz

ViewBase::ViewBase(Session &session, QWidget *parent) :
	session_(session)
{
	(void)parent;

	connect(&session_, SIGNAL(signals_changed()),
		this, SLOT(signals_changed()));
	connect(&session_, SIGNAL(capture_state_changed(int)),
		this, SLOT(capture_state_updated(int)));

	connect(&delayed_view_updater_, SIGNAL(timeout()),
		this, SLOT(perform_delayed_view_update()));
	delayed_view_updater_.setSingleShot(true);
	delayed_view_updater_.setInterval(1000 / MaxViewAutoUpdateRate);
}

Session& ViewBase::session()
{
	return session_;
}

const Session& ViewBase::session() const
{
	return session_;
}

void ViewBase::clear_signals()
{
}

unordered_set< shared_ptr<data::SignalBase> > ViewBase::signalbases() const
{
	return signalbases_;
}

void ViewBase::clear_signalbases()
{
	for (shared_ptr<data::SignalBase> signalbase : signalbases_) {
		disconnect(signalbase.get(), SIGNAL(samples_cleared()),
			this, SLOT(on_data_updated()));
		disconnect(signalbase.get(), SIGNAL(samples_added(QObject*, uint64_t, uint64_t)),
			this, SLOT(on_data_updated()));
	}

	signalbases_.clear();
}

void ViewBase::add_signalbase(const shared_ptr<data::SignalBase> signalbase)
{
	signalbases_.insert(signalbase);

	connect(signalbase.get(), SIGNAL(samples_cleared()),
		this, SLOT(on_data_updated()));
	connect(signalbase.get(), SIGNAL(samples_added(QObject*, uint64_t, uint64_t)),
		this, SLOT(on_data_updated()));
}

void ViewBase::save_settings(QSettings &settings) const
{
	(void)settings;
}

void ViewBase::restore_settings(QSettings &settings)
{
	(void)settings;
}

void ViewBase::trigger_event(util::Timestamp location)
{
	(void)location;
}

void ViewBase::signals_changed()
{
}

void ViewBase::capture_state_updated(int state)
{
	(void)state;
}

void ViewBase::perform_delayed_view_update()
{
}

void ViewBase::on_data_updated()
{
	if (!delayed_view_updater_.isActive())
		delayed_view_updater_.start();
}

} // namespace views
} // namespace sv
