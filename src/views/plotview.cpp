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

#include <cassert>

#include <QMainWindow>
#include <QBoxLayout>
#include <QVBoxLayout>

#include "plotview.hpp"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basecurve.hpp"
#include "src/data/timecurve.hpp"
#include "src/data/xycurve.hpp"
#include "src/widgets/plot.hpp"

using std::dynamic_pointer_cast;
using std::static_pointer_cast;

namespace sv {
namespace views {

PlotView::PlotView(const Session &session,
		shared_ptr<channels::BaseChannel> channel,
		QWidget *parent) :
	BaseView(session, parent),
	channel_(channel),
	action_zoom_in_(new QAction(this)),
	action_zoom_out_(new QAction(this)),
	action_zoom_fit_best_(new QAction(this)),
	action_add_marker_(new QAction(this)),
	action_add_diff_marker_(new QAction(this)),
	action_config_graph_(new QAction(this))
{
	assert(channel_);

	shared_ptr<data::AnalogSignal> signal;
	if (channel_->actual_signal())
		signal = static_pointer_cast<data::AnalogSignal>(
			channel_->actual_signal());

	if (signal)
		curve_ = (data::BaseCurve *)(new data::TimeCurve(signal));
	else
		curve_ = nullptr;

	// Signal (Quantity + Unit) can change, e.g. DMM signals
	connect(channel_.get(), SIGNAL(signal_changed()),
			this, SLOT(on_signal_changed()));

	setup_ui();
	setup_toolbar();
	connect_signals();
	init_values();

	plot_->start();
}

PlotView::PlotView(const Session& session,
		shared_ptr<data::AnalogSignal> signal,
		QWidget* parent) :
	BaseView(session, parent),
	channel_(nullptr),
	action_zoom_in_(new QAction(this)),
	action_zoom_out_(new QAction(this)),
	action_zoom_fit_best_(new QAction(this)),
	action_add_marker_(new QAction(this)),
	action_add_diff_marker_(new QAction(this)),
	action_config_graph_(new QAction(this))
{
	assert(signal);

	curve_ = (data::BaseCurve *)(new data::TimeCurve(signal));

	setup_ui();
	setup_toolbar();
	connect_signals();
	init_values();

	plot_->start();
}

PlotView::PlotView(const Session& session,
		shared_ptr<data::AnalogSignal> x_signal,
		shared_ptr<data::AnalogSignal> y_signal,
		QWidget* parent) :
	BaseView(session, parent),
	channel_(nullptr),
	action_zoom_in_(new QAction(this)),
	action_zoom_out_(new QAction(this)),
	action_zoom_fit_best_(new QAction(this)),
	action_add_marker_(new QAction(this)),
	action_add_diff_marker_(new QAction(this)),
	action_config_graph_(new QAction(this))
{
	assert(x_signal);
	assert(y_signal);

	curve_ = (data::BaseCurve *)(new data::XYCurve(x_signal, y_signal));

	setup_ui();
	setup_toolbar();
	connect_signals();
	init_values();

	plot_->start();
}

QString PlotView::title() const
{
	QString title;

	if (channel_)
		title = tr("Channel");
	else
		title = tr("Signal");

	if (curve_)
		title = title.append(" ").append(curve_->name());
	else if (channel_)
		title = title.append(" ").append(channel_->display_name());

	return title;
}

void PlotView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	plot_ = new widgets::Plot(curve_);
	plot_->set_plot_mode(widgets::Plot::PlotModes::Additive);
	plot_->set_plot_interval(200); // 200ms
	layout->addWidget(plot_);

	this->centralWidget_->setLayout(layout);
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

	toolbar_ = new QToolBar("Device Toolbar");
	toolbar_->addAction(action_zoom_in_);
	toolbar_->addAction(action_zoom_out_);
	toolbar_->addAction(action_zoom_fit_best_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_add_marker_);
	toolbar_->addAction(action_add_diff_marker_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_config_graph_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void PlotView::connect_signals()
{
}

void PlotView::init_values()
{
}

void PlotView::on_signal_changed()
{
	if (!channel_)
		return;

	shared_ptr<data::AnalogSignal> signal;
	if (channel_->actual_signal())
		signal = dynamic_pointer_cast<data::AnalogSignal>(
			channel_->actual_signal());

	this->parentWidget()->setWindowTitle(this->title());

	if (signal) {
		curve_ = (data::BaseCurve *)(new data::TimeCurve(signal));
		plot_->set_curve_data(curve_);
	}
	else
		curve_ = nullptr;
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

// TODO: connect directly to plot?
void PlotView::on_action_add_marker_triggered()
{
	plot_->add_marker();
}

void PlotView::on_action_add_diff_marker_triggered()
{
}

void PlotView::on_action_config_graph_triggered()
{
}

} // namespace views
} // namespace sv

