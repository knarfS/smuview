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

#include <QAbstractButton>
#include <QMainWindow>
#include <QSizePolicy>

#include "basetab.hpp"
#include "src/session.hpp"

using std::shared_ptr;

namespace sv {
namespace tabs {

BaseTab::BaseTab(Session &session, QMainWindow *parent) :
		//QWidget(parent),
	session_(session),
	parent_(parent)
{
}

Session& BaseTab::session()
{
	return session_;
}

const Session& BaseTab::session() const
{
	return session_;
}

void BaseTab::save_settings(QSettings &settings) const
{
	(void)settings;
}

void BaseTab::restore_settings(QSettings &settings)
{
	(void)settings;
}

shared_ptr<views::BaseView> BaseTab::add_view(const QString &title,
	shared_ptr<views::BaseView> view, Qt::DockWidgetArea area, Session &session)
{
	//GlobalSettings settings;

	//shared_ptr<MainBar> main_bar = session.main_bar();

	QDockWidget* dock = new QDockWidget(title, parent_);
	dock->setObjectName(title);
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	parent_->addDockWidget(area, dock);

	// Insert a QMainWindow into the dock widget to allow for a tool bar
	QMainWindow *dock_main = new QMainWindow(dock);
	dock_main->setWindowFlags(Qt::Widget);  // Remove Qt::Window flag

	/*
	if (type == views::ViewTypeTrace)
		// This view will be the main view if there's no main bar yet
		view = make_shared<views::trace::View>(session,
			(main_bar ? false : true), dock_main);
	*/

	if (!view)
		return nullptr;

	view_docks_[dock] = view;
	//session.register_view(view);

	dock_main->setCentralWidget(view.get());
	dock->setWidget(dock_main);

	dock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	dock->setContextMenuPolicy(Qt::PreventContextMenu);
	dock->setFeatures(QDockWidget::DockWidgetMovable |
		QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

	QAbstractButton *close_btn =
		dock->findChildren<QAbstractButton*>
			("qt_dockwidget_closebutton").front();

	connect(close_btn, SIGNAL(clicked(bool)),
		this, SLOT(on_view_close_clicked()));

	connect(&session, SIGNAL(trigger_event(util::Timestamp)),
		qobject_cast<views::BaseView*>(view.get()),
		SLOT(trigger_event(util::Timestamp)));

	/*
	if (type == views::ViewTypeTrace) {
		views::trace::View *tv =
			qobject_cast<views::trace::View*>(view.get());

		tv->enable_coloured_bg(settings.value(GlobalSettings::Key_View_ColouredBG).toBool());
		tv->enable_show_sampling_points(settings.value(GlobalSettings::Key_View_ShowSamplingPoints).toBool());
		tv->enable_show_analog_minor_grid(settings.value(GlobalSettings::Key_View_ShowAnalogMinorGrid).toBool());

		if (!main_bar) {
			/ * Initial view, create the main bar * /
			main_bar = make_shared<MainBar>(session, this, tv);
			dock_main->addToolBar(main_bar.get());
			session.set_main_bar(main_bar);

			connect(main_bar.get(), SIGNAL(new_view(Session*)),
				this, SLOT(on_new_view(Session*)));

			main_bar->action_view_show_cursors()->setChecked(tv->cursors_shown());

			/ * For the main view we need to prevent the dock widget from
			 * closing itself when its close button is clicked. This is
			 * so we can confirm with the user first. Regular views don't
			 * need this * /
			close_btn->disconnect(SIGNAL(clicked()), dock, SLOT(close()));
		} else {
			/ * Additional view, create a standard bar * /
			pv::views::trace::StandardBar *standard_bar =
				new pv::views::trace::StandardBar(session, this, tv);
			dock_main->addToolBar(standard_bar);

			standard_bar->action_view_show_cursors()->setChecked(tv->cursors_shown());
		}
	}
	*/

	return view;
}

} // namespace tabs
} // namespace sv
