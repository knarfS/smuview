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

#include <QMainWindow>
#include <QBoxLayout>
#include <QVBoxLayout>

#include "plotview.hpp"
#include "src/session.hpp"
#include "src/data/analogdata.hpp"
#include "src/data/basecurve.hpp"
#include "src/widgets/plot.hpp"

namespace sv {
namespace views {

PlotView::PlotView(const Session &session,
	shared_ptr<data::AnalogData> x_signal_data,
	shared_ptr<data::AnalogData> y_signal_data,
	QWidget *parent) :
		BaseView(session, parent),
	x_signal_data_(x_signal_data),
	y_signal_data_(y_signal_data),
	action_zoom_in_(new QAction(this)),
	action_zoom_out_(new QAction(this)),
	action_zoom_fit_best_(new QAction(this)),
	action_add_marker_(new QAction(this)),
	action_add_diff_marker_(new QAction(this)),
	action_config_graph_(new QAction(this))
{
	setup_ui();
	setup_toolbar();
	connect_signals();
	init_values();

	plot->start();
}

QString PlotView::title() const
{
	return tr("Plot"); // TODO: + Quantity
}

void PlotView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	data::BaseCurve *curve = new data::BaseCurve(
		x_signal_data_, y_signal_data_);

	plot = new widgets::Plot(curve);
	plot->set_plot_mode(widgets::Plot::PlotModes::Additive);
	plot->set_plot_interval(200); // 200ms
	layout->addWidget(plot);

	this->centralWidget->setLayout(layout);
}

void PlotView::setup_toolbar()
{
	action_zoom_in_->setText(tr("Zoom In..."));
	action_zoom_in_->setIcon(
		QIcon::fromTheme("zoom-in",
		QIcon(":/icons/zoom-in.png")));
	//action_zoom_in_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
	//connect(action_zoom_in_, SIGNAL(triggered(bool)),
	//	this, SLOT(on_actionOpen_triggered()));

	action_zoom_out_->setText(tr("Zoom Out..."));
	action_zoom_out_->setIcon(
		QIcon::fromTheme("zoom-out",
		QIcon(":/icons/zoom-out.png")));
	//action_zoom_out_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
	//connect(action_zoom_out_, SIGNAL(triggered(bool)),
	//	this, SLOT(on_actionSaveAs_triggered()));

	action_zoom_fit_best_->setText(tr("Best fit"));
	action_zoom_fit_best_->setIcon(
		QIcon::fromTheme("zoom-fit-best",
		QIcon(":/icons/zoom-fit-best.png")));
	//action_zoom_fit_best_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
	//connect(action_zoom_fit_best_, SIGNAL(triggered(bool)),
	//	this, SLOT(on_actionSaveAs_triggered()));

	action_add_marker_->setText(tr("Add Marker..."));
	action_add_marker_->setIcon(
		QIcon::fromTheme("snap-orthogonal",
		QIcon(":/icons/orthogonal.png")));
	//action_add_marker_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
	connect(action_add_marker_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_marker_triggered()));

	action_add_diff_marker_->setText(tr("Add Diff-Marker..."));
	action_add_diff_marker_->setIcon(
		QIcon::fromTheme("snap-guideline",
		QIcon(":/icons/snap-guideline.png")));
	//action_add_diff_marker_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
	//connect(action_add_diff_marker_, SIGNAL(triggered(bool)),
	//	this, SLOT(on_actionSaveAs_triggered()));

	action_config_graph_->setText(tr("Configure graph"));
	action_config_graph_->setIcon(
		QIcon::fromTheme("configure",
		QIcon(":/icons/configure.png")));
	//action_config_graph_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
	//connect(action_config_graph_, SIGNAL(triggered(bool)),
	//	this, SLOT(on_actionSaveAs_triggered()));

	toolbar = new QToolBar("Device Toolbar");
	toolbar->addAction(action_zoom_in_);
	toolbar->addAction(action_zoom_out_);
	toolbar->addAction(action_zoom_fit_best_);
	toolbar->addSeparator();
	toolbar->addAction(action_add_marker_);
	toolbar->addAction(action_add_diff_marker_);
	toolbar->addSeparator();
	toolbar->addAction(action_config_graph_);
	this->addToolBar(Qt::TopToolBarArea, toolbar);
}

void PlotView::connect_signals()
{
}

void PlotView::init_values()
{
}

void PlotView::on_action_zoom_in_triggered()
{
}

void PlotView::on_action_zoom_out_triggered()
{
}

void PlotView::on_action_zoom_fit_best_triggered()
{
}

// TODO: connect directly to plit?
void PlotView::on_action_add_marker_triggered()
{
	plot->add_marker();
}

void PlotView::on_action_add_diff_marker_triggered()
{
}

void PlotView::on_action_config_graph_triggered()
{
}

} // namespace views
} // namespace sv

