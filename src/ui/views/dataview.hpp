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

#ifndef UI_VIEWS_DATAVIEW_HPP
#define UI_VIEWS_DATAVIEW_HPP

#include <memory>
#include <mutex>
#include <vector>

#include <QAction>
#include <QSettings>
#include <QTableWidget>
#include <QToolBar>
#include <QUuid>

#include "src/ui/views/baseview.hpp"

using std::shared_ptr;
using std::vector;

namespace sv {

class Session;

namespace data {
class AnalogTimeSignal;
}

namespace ui {
namespace views {

class DataView : public BaseView
{
	Q_OBJECT

public:
	DataView(Session& session, QUuid uuid = QUuid(), QWidget* parent = nullptr);

	QString title() const override;
	void add_signal(shared_ptr<sv::data::AnalogTimeSignal> signal);

	void save_settings(QSettings &settings) const override;
	void restore_settings(QSettings &settings) override;

private:
	vector<shared_ptr<sv::data::AnalogTimeSignal>> signals_;
	vector<size_t> next_signal_pos_;
	vector<QTableWidgetItem *> last_timestamp_;
	bool auto_scroll_;
	std::mutex populate_mutex_;

	QAction *const action_auto_scroll_;
	QAction *const action_add_signal_;
	QToolBar *toolbar_;
	QTableWidget *data_table_;

	void setup_ui();
	void setup_toolbar();

private Q_SLOTS:
	void populate_table();
	void on_action_auto_scroll_triggered();
	void on_action_add_signal_triggered();

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_DATAVIEW_HPP
