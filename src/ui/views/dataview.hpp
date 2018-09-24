/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_VIEWS_DATAVIEW_HPP
#define UI_VIEWS_DATAVIEW_HPP

#include <memory>

#include <QAction>
#include <QTableWidget>
#include <QToolBar>

#include "src/ui/views/baseview.hpp"

using std::shared_ptr;

namespace sv {

class Session;

namespace data {
class AnalogSignal;
}

namespace ui {
namespace views {

class DataView : public BaseView
{
	Q_OBJECT

public:
	DataView(const Session& session,
		shared_ptr<sv::data::AnalogSignal> signal,
		QWidget* parent = nullptr);

	QString title() const;

private:
	shared_ptr<sv::data::AnalogSignal> signal_;
	size_t next_signal_pos_;
	bool auto_scroll_;

	QAction *const action_auto_scroll_;
	QAction *const action_add_signal_;
	QToolBar *toolbar_;
	QTableWidget *data_table_;

	void setup_ui();
	void setup_toolbar();
	void connect_signals();

private Q_SLOTS:
	void populate_table();
	void on_action_auto_scroll_triggered();
	void on_action_add_signal_triggered();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_DATAVIEW_HPP
