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

#ifndef VIEWS_VIEWBASE_HPP
#define VIEWS_VIEWBASE_HPP

#include <cstdint>
#include <memory>
#include <unordered_set>
#include <vector>

#include <QTimer>
#include <QWidget>

#include "src/util.hpp"
#include "src/data/signalbase.hpp"

using std::shared_ptr;
using std::unordered_set;

namespace sv {

class Session;

namespace views {

enum ViewType {
	ViewTypeSource,
	ViewTypeSink,
	ViewTypeMeasurement,
	ViewTypeGraph
};

class ViewBase : public QWidget
{
	Q_OBJECT

private:
	static const int MaxViewAutoUpdateRate;

public:
	explicit ViewBase(Session &session, QWidget *parent = nullptr);

	Session& session();
	const Session& session() const;

	virtual void clear_signals();

	/**
	 * Returns the signal bases contained in this view.
	 */
	unordered_set< shared_ptr<data::SignalBase> > signalbases() const;

	virtual void clear_signalbases();

	virtual void add_signalbase(const shared_ptr<data::SignalBase> signalbase);

	virtual void save_settings(QSettings &settings) const;

	virtual void restore_settings(QSettings &settings);

public Q_SLOTS:
	virtual void trigger_event(util::Timestamp location);
	virtual void signals_changed();
	virtual void capture_state_updated(int state);
	virtual void perform_delayed_view_update();

private Q_SLOTS:
	void on_data_updated();

protected:
	Session &session_;

	util::TimeUnit time_unit_;

	unordered_set< shared_ptr<data::SignalBase> > signalbases_;

	QTimer delayed_view_updater_;
};

} // namespace views
} // namespace sv

#endif // VIEWS_SOURCEVIEW_HPP
