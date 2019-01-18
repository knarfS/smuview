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

#include <QAction>
#include <QDebug>
#include <QToolBar>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "signaltreeview.hpp"
#include "src/session.hpp"
#include "src/ui/devices/signaltree.hpp"

namespace sv {
namespace ui {
namespace views {

SignalTreeView::SignalTreeView(const Session &session, QWidget *parent) :
	BaseView(session, parent),
	action_add_device_(new QAction(this)),
	action_add_user_device_(new QAction(this)),
	action_delete_device_(new QAction(this)),
	action_add_user_channel_(new QAction(this)),
	action_add_math_channel_(new QAction(this)),
	action_delete_channel_(new QAction(this))
{
	setup_ui();
	setup_toolbar();
	connect_signals();
}


QString SignalTreeView::title() const
{
	return tr("Devices && Signals");
}

void SignalTreeView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	signal_tree_ = new devices::SignalTree(
		session_, true, false, false, nullptr);
	signal_tree_->enable_context_menu(true);
	layout->addWidget(signal_tree_);

	this->central_widget_->setLayout(layout);
}

void SignalTreeView::setup_toolbar()
{
	action_add_device_->setText(tr("Add device"));
	action_add_device_->setIcon(
		QIcon::fromTheme("document-new",
		QIcon(":/icons/document-new.png")));
	connect(action_add_device_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_device_triggered()));

	action_add_user_device_->setText(tr("Add user device"));
	action_add_user_device_->setIcon(
		QIcon::fromTheme("tab-new-background",
		QIcon(":/icons/tab-new-background.png")));
	connect(action_add_user_device_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_user_device_triggered()));

	/* TODO: Implement
	action_delete_device_->setText(tr("Delete device"));
	action_delete_device_->setIcon(
		QIcon::fromTheme("office-chart-line",
		QIcon(":/icons/office-chart-line.png")));
	connect(action_delete_device_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_delete_device_triggered()));
	*/

	action_add_user_channel_->setText(tr("Add user channel"));
	action_add_user_channel_->setIcon(
		QIcon::fromTheme("office-chart-line-percentage",
		QIcon(":/icons/office-chart-line-percentage")));
	connect(action_add_user_channel_, SIGNAL(triggered(bool)),
		signal_tree_, SLOT(on_add_user_channel()));

	action_add_math_channel_->setText(tr("Add math channel"));
	action_add_math_channel_->setIcon(
		QIcon::fromTheme("office-chart-line-percentage",
		QIcon(":/icons/office-chart-line-percentage.png")));
	connect(action_add_math_channel_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_math_channel_triggered()));

	/* TODO: Implement
	action_delete_channel_->setText(tr("Delete channel"));
	action_delete_channel_->setIcon(
		QIcon::fromTheme("go-bottom",
		QIcon(":/icons/go-bottom.png")));
	connect(action_delete_channel_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_delete_channel_triggered()));
	*/

	toolbar_ = new QToolBar("SignalTree View Toolbar");
	toolbar_->addAction(action_add_device_);
	toolbar_->addAction(action_add_user_device_);
	// TODO: Implement:
	//toolbar_->addAction(action_delete_device_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_add_user_channel_);
	toolbar_->addAction(action_add_math_channel_);
	// TODO: Implement:
	//toolbar_->addAction(action_delete_channel_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void SignalTreeView::update_toolbar()
{
	//QTreeWidgetItem *item = signal_tree_->currentItem();
}

void SignalTreeView::connect_signals()
{
	connect(&session_, SIGNAL(device_added(shared_ptr<sv::devices::BaseDevice>)),
		signal_tree_, SLOT(on_device_added(shared_ptr<sv::devices::BaseDevice>)));

	connect(signal_tree_, SIGNAL(itemSelectionChanged()),
		this, SLOT(update_toolbar()));
}

void SignalTreeView::on_action_add_device_triggered()
{

}

void SignalTreeView::on_action_add_user_device_triggered()
{

}

void SignalTreeView::on_action_delete_device_triggered()
{

}

void SignalTreeView::on_action_add_math_channel_triggered()
{

}

void SignalTreeView::on_action_delete_channel_triggered()
{

}

} // namespace views
} // namespace ui
} // namespace sv

