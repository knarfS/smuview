/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_VIEWS_FLOWVIEW_HPP
#define UI_VIEWS_FLOWVIEW_HPP

#include <memory>

#include <QAction>
#include <QPoint>
#include <QString>
#include <QToolBar>

#include <FlowScene>
#include <FlowView>
#include <Node>

#include "src/ui/views/baseview.hpp"

using std::shared_ptr;
using QtNodes::Node;

namespace sv {

class Session;

namespace ui {
namespace views {

class FlowView : public BaseView
{
	Q_OBJECT

public:
	FlowView(const Session& session, QWidget* parent = nullptr);

	QString title() const override;

private:
	void setup_ui();
	void setup_toolbar();

	QAction *const action_start_;
	QAction *const action_pause_;
	QAction *const action_stop_;
	QAction *const action_load_;
	QAction *const action_save_;
	QToolBar *toolbar_;
	QtNodes::FlowScene *flow_scene_;
	QtNodes::FlowView *flow_view_widget_;

private Q_SLOTS:
	void on_action_start_triggered();
	void on_action_pause_triggered();
	void on_action_stop_triggered();
	void on_action_save_triggered();
	void on_action_load_triggered();
	void show_node_popup(Node &node, QPoint);

};

} // namespace views
} // namespace ui
} // namespace sv

#endif // UI_VIEWS_FLOWVIEW_HPP
