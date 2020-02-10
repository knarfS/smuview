/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <cassert>
#include <string>

#include <QImageWriter>
#include <QFileDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <qwt_plot_renderer.h>

#include "plotview.hpp"
#include "src/session.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/ui/dialogs/plotconfigdialog.hpp"
#include "src/ui/dialogs/plotdiffmarkerdialog.hpp"
#include "src/ui/dialogs/selectsignaldialog.hpp"
#include "src/ui/widgets/plot/plot.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"
#include "src/ui/widgets/plot/timecurvedata.hpp"
#include "src/ui/widgets/plot/xycurvedata.hpp"

using std::dynamic_pointer_cast;
using std::static_pointer_cast;
using std::string;

Q_DECLARE_METATYPE(sv::ui::widgets::plot::BaseCurveData *)

namespace sv {
namespace ui {
namespace views {

PlotView::PlotView(Session &session,
		shared_ptr<channels::BaseChannel> channel,
		QWidget *parent) :
	BaseView(session, parent),
	initial_channel_(channel),
	action_add_marker_(new QAction(this)),
	action_add_diff_marker_(new QAction(this)),
	action_zoom_best_fit_(new QAction(this)),
	action_add_signal_(new QAction(this)),
	action_save_(new QAction(this)),
	action_config_plot_(new QAction(this)),
	plot_type_(PlotType::TimePlot)
{
	assert(initial_channel_);

	shared_ptr<data::AnalogTimeSignal> signal;
	if (initial_channel_->actual_signal())
		signal = static_pointer_cast<data::AnalogTimeSignal>(
			initial_channel_->actual_signal());

	if (signal)
		curves_.push_back(new widgets::plot::TimeCurveData(signal));

	id_ = "plot_ch:" + initial_channel_->name();

	// Signal (aka Quantity + Flags + Unit) can change, e.g. DMM channels
	connect(initial_channel_.get(),
		SIGNAL(signal_added(shared_ptr<sv::data::BaseSignal>)),
		this, SLOT(on_signal_changed()));
	connect(initial_channel_.get(),
		SIGNAL(signal_changed(shared_ptr<sv::data::BaseSignal>)),
		this, SLOT(on_signal_changed()));

	setup_ui();
	setup_toolbar();
	connect_signals();
	init_values();

	plot_->start();
}

PlotView::PlotView(Session &session,
		shared_ptr<sv::data::AnalogTimeSignal> signal,
		QWidget *parent) :
	BaseView(session, parent),
	initial_channel_(nullptr),
	action_add_marker_(new QAction(this)),
	action_add_diff_marker_(new QAction(this)),
	action_zoom_best_fit_(new QAction(this)),
	action_add_signal_(new QAction(this)),
	action_save_(new QAction(this)),
	action_config_plot_(new QAction(this)),
	plot_type_(PlotType::TimePlot)
{
	assert(signal);

	curves_.push_back(new widgets::plot::TimeCurveData(signal));

	id_ = "plot_sig:" + signal->name();

	setup_ui();
	setup_toolbar();
	connect_signals();
	init_values();

	plot_->start();
}

PlotView::PlotView(Session &session,
		shared_ptr<sv::data::AnalogTimeSignal> x_signal,
		shared_ptr<sv::data::AnalogTimeSignal> y_signal,
		QWidget *parent) :
	BaseView(session, parent),
	initial_channel_(nullptr),
	action_add_marker_(new QAction(this)),
	action_add_diff_marker_(new QAction(this)),
	action_zoom_best_fit_(new QAction(this)),
	action_add_signal_(new QAction(this)),
	action_save_(new QAction(this)),
	action_config_plot_(new QAction(this)),
	plot_type_(PlotType::XYPlot)
{
	assert(x_signal);
	assert(y_signal);

	curves_.push_back(new widgets::plot::XYCurveData(x_signal, y_signal));

	id_ = "plot_xy:" + x_signal->name() + ":" + y_signal->name();

	setup_ui();
	setup_toolbar();
	connect_signals();
	init_values();

	plot_->start();
}

QString PlotView::title() const
{
	QString title;

	if (initial_channel_)
		title = tr("Channel");
	else
		title = tr("Signal");

	if (!curves_.empty())
		title = title.append(" ").append(curves_[0]->name());
	else if (initial_channel_)
		title = title.append(" ").append(initial_channel_->display_name());

	return title;
}

void PlotView::add_time_curve(shared_ptr<sv::data::AnalogTimeSignal> signal)
{
	auto curve = new widgets::plot::TimeCurveData(signal);
	if (plot_->add_curve(curve)) {
		curves_.push_back(curve);
		update_add_marker_menu();
	}
	else {
		QMessageBox::warning(this,
			tr("Cannot add signal"), tr("Cannot add time signal to plot!"),
			QMessageBox::Ok);
	}
}

void PlotView::add_xy_curve(shared_ptr<sv::data::AnalogTimeSignal> y_signal)
{
	// Get the x signal from a existing curve
	if (curves_.size() == 0) {
		QMessageBox::warning(this, tr("Cannot add signal"),
			tr("Cannot add new x signal without an existing x signal!"),
			QMessageBox::Ok);
		return;
	}

	auto x_signal = ((widgets::plot::XYCurveData *)curves_.at(0))->x_t_signal();
	this->add_xy_curve(x_signal, y_signal);
}

void PlotView::add_xy_curve(shared_ptr<sv::data::AnalogTimeSignal> x_signal,
	shared_ptr<sv::data::AnalogTimeSignal> y_signal)
{
	auto curve = new widgets::plot::XYCurveData(x_signal, y_signal);
	if (plot_->add_curve(curve)) {
		curves_.push_back(curve);
		update_add_marker_menu();
	}
	else {
		QMessageBox::warning(this,
			tr("Cannot add signal"), tr("Cannot add xy signal to plot!"),
			QMessageBox::Ok);
	}
}

void PlotView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	plot_ = new widgets::plot::Plot();
	plot_->set_update_mode(widgets::plot::PlotUpdateMode::Additive);
	plot_->set_plot_interval(200); // 200ms

	for (const auto &curve : curves_)
		plot_->add_curve(curve);

	layout->addWidget(plot_);

	this->central_widget_->setLayout(layout);
}

void PlotView::setup_toolbar()
{
	add_marker_menu_ = new QMenu();
	update_add_marker_menu();

	add_marker_button_ = new QToolButton();
	add_marker_button_->setText(tr("Add marker"));
	add_marker_button_->setIcon(
		QIcon::fromTheme("snap-orthogonal",
		QIcon(":/icons/snap-orthogonal.png")));
	add_marker_button_->setMenu(add_marker_menu_);
	add_marker_button_->setPopupMode(QToolButton::MenuButtonPopup);

	action_add_diff_marker_->setText(tr("Add diff-marker"));
	action_add_diff_marker_->setIcon(
		QIcon::fromTheme("snap-guideline",
		QIcon(":/icons/snap-guideline.png")));
	action_add_diff_marker_->setDisabled(true);
	connect(action_add_diff_marker_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_diff_marker_triggered()));

	action_zoom_best_fit_->setText(tr("Best fit"));
	action_zoom_best_fit_->setIcon(
		QIcon::fromTheme("zoom-fit-best",
		QIcon(":/icons/zoom-fit-best.png")));
	connect(action_zoom_best_fit_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_zoom_best_fit_triggered()));

	action_add_signal_->setText(tr("Add Signal"));
	action_add_signal_->setIcon(
		QIcon::fromTheme("office-chart-line",
		QIcon(":/icons/office-chart-line.png")));
	connect(action_add_signal_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_signal_triggered()));

	action_save_->setText(tr("Save"));
	action_save_->setIcon(
		QIcon::fromTheme("document-save",
		QIcon(":/icons/document-save.png")));
	connect(action_save_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_save_triggered()));

	action_config_plot_->setText(tr("Configure Plot"));
	action_config_plot_->setIcon(
		QIcon::fromTheme("configure",
		QIcon(":/icons/configure.png")));
	connect(action_config_plot_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_config_plot_triggered()));

	toolbar_ = new QToolBar("Plot Toolbar");
	toolbar_->addWidget(add_marker_button_);
	toolbar_->addAction(action_add_diff_marker_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_zoom_best_fit_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_add_signal_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_save_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_config_plot_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void PlotView::update_add_marker_menu()
{
	// First remove all existing actions
	for (QAction *action : add_marker_menu_->actions()) {
		disconnect(action, SIGNAL(triggered(bool)),
			this, SLOT(on_action_add_marker_triggered()));
		add_marker_menu_->removeAction(action);
		delete action;
	}

	// One add marker action for each curve
	for (const auto &curve : curves_) {
		QAction *action = new QAction(this);
		action->setText(curve->name());
		action->setData(QVariant::fromValue(curve));
		connect(action, SIGNAL(triggered(bool)),
			this, SLOT(on_action_add_marker_triggered()));
		add_marker_menu_->addAction(action);
	}
}

void PlotView::connect_signals()
{
}

void PlotView::init_values()
{
}

void PlotView::on_signal_changed()
{
	if (!initial_channel_)
		return;
	if (plot_type_ != PlotType::TimePlot)
		return;

	shared_ptr<sv::data::AnalogTimeSignal> signal;
	if (initial_channel_->actual_signal())
		signal = dynamic_pointer_cast<sv::data::AnalogTimeSignal>(
			initial_channel_->actual_signal());
	if (!signal)
		return;

	// Check if new actual_signal is already added to this plot
	for (const auto &curve : curves_) {
		if (((widgets::plot::TimeCurveData *)curve)->signal() == signal)
			return;
	}

	this->parentWidget()->setWindowTitle(this->title());
	auto curve = new widgets::plot::TimeCurveData(signal);
	if (plot_->add_curve(curve)) {
		curves_.push_back(curve);
		update_add_marker_menu();
	}
}

void PlotView::on_action_add_marker_triggered()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action) {
		widgets::plot::BaseCurveData *curve_data =
			action->data().value<widgets::plot::BaseCurveData *>();
		plot_->add_marker(curve_data);
	}

	if (plot_->markers().size() >= 2)
		action_add_diff_marker_->setDisabled(false);
	else
		action_add_diff_marker_->setDisabled(true);
}

void PlotView::on_action_add_diff_marker_triggered()
{
	ui::dialogs::PlotDiffMarkerDialog dlg(plot_);
	dlg.exec();
}

void PlotView::on_action_zoom_best_fit_triggered()
{
	plot_->set_all_axis_locked(false);
}

void PlotView::on_action_add_signal_triggered()
{
	shared_ptr<sv::devices::BaseDevice> selected_device;
	if (initial_channel_)
		selected_device = initial_channel_->parent_device();

	ui::dialogs::SelectSignalDialog dlg(session(), selected_device);
	if (!dlg.exec())
		return;

	for (const auto &signal : dlg.signals()) {
		if (plot_type_ == PlotType::TimePlot) {
			add_time_curve(
				dynamic_pointer_cast<sv::data::AnalogTimeSignal>(signal));
		}
		else {
			add_xy_curve(
				dynamic_pointer_cast<sv::data::AnalogTimeSignal>(signal));
		}
	}
}

void PlotView::on_action_save_triggered()
{
	QString filter("SVG Image (*.svg);;PDF File (*.pdf)");
	for (const auto &supported : QImageWriter::supportedImageFormats()) {
		filter += ";;" + supported.toUpper() + " Image (*." + supported + ")";
	}
	QString *selected_filter = new QString("SVG Image (*.svg)");
	QString file_name = QFileDialog::getSaveFileName(this,
		tr("Save Plot"), QDir::homePath(), filter, selected_filter);
	if (file_name.length() <= 0)
		return;

	// TODO
	QSizeF size(300, 300);
	int resolution = 90;
	QwtPlotRenderer renderer;
	renderer.renderDocument(plot_, file_name, size, resolution);
}

void PlotView::on_action_config_plot_triggered()
{
	ui::dialogs::PlotConfigDialog dlg(plot_, plot_type_);
	dlg.exec();
}

} // namespace views
} // namespace ui
} // namespace sv
