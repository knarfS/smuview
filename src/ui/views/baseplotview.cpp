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

#include <string>

#include <QImageWriter>
#include <QFileDialog>
#include <QSettings>
#include <QToolButton>
#include <QUuid>
#include <QVBoxLayout>
#include <qwt_plot_renderer.h>

#include "baseplotview.hpp"
#include "src/session.hpp"
#include "src/ui/dialogs/plotconfigdialog.hpp"
#include "src/ui/dialogs/plotdiffmarkerdialog.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/widgets/plot/curve.hpp"
#include "src/ui/widgets/plot/plot.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"

using std::dynamic_pointer_cast;

Q_DECLARE_METATYPE(sv::ui::widgets::plot::Curve *)

namespace sv {
namespace ui {
namespace views {

BasePlotView::BasePlotView(Session &session, QUuid uuid, QWidget *parent) :
	BaseView(session, uuid, parent),
	action_add_marker_(new QAction(this)),
	action_add_diff_marker_(new QAction(this)),
	action_zoom_best_fit_(new QAction(this)),
	action_add_signal_(new QAction(this)),
	action_save_(new QAction(this)),
	action_config_plot_(new QAction(this))
{
	setup_ui();
	setup_toolbar();
	connect_signals();

	plot_->start();
}

void BasePlotView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	plot_ = new widgets::plot::Plot(session_);
	plot_->set_update_mode(widgets::plot::PlotUpdateMode::Additive);
	plot_->set_plot_interval(200); // 200ms

	layout->addWidget(plot_);

	this->central_widget_->setLayout(layout);
}

void BasePlotView::setup_toolbar()
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
	connect(action_add_diff_marker_, &QAction::triggered,
		this, &BasePlotView::on_action_add_diff_marker_triggered);

	action_zoom_best_fit_->setText(tr("Best fit"));
	action_zoom_best_fit_->setIcon(
		QIcon::fromTheme("zoom-fit-best",
		QIcon(":/icons/zoom-fit-best.png")));
	connect(action_zoom_best_fit_, &QAction::triggered,
		this, &BasePlotView::on_action_zoom_best_fit_triggered);

	action_add_signal_->setText(tr("Add Signal"));
	action_add_signal_->setIcon(
		QIcon::fromTheme("office-chart-line",
		QIcon(":/icons/office-chart-line.png")));
	connect(action_add_signal_, &QAction::triggered,
		this, &BasePlotView::on_action_add_signal_triggered);

	action_save_->setText(tr("Save"));
	action_save_->setIcon(
		QIcon::fromTheme("document-save",
		QIcon(":/icons/document-save.png")));
	connect(action_save_, &QAction::triggered,
		this, &BasePlotView::on_action_save_triggered);

	action_config_plot_->setText(tr("Configure Plot"));
	action_config_plot_->setIcon(
		QIcon::fromTheme("configure",
		QIcon(":/icons/configure.png")));
	connect(action_config_plot_, &QAction::triggered,
		this, &BasePlotView::on_action_config_plot_triggered);

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

void BasePlotView::update_add_marker_menu()
{
	// First remove all existing actions
	for (QAction *action : add_marker_menu_->actions()) {
		disconnect(action, &QAction::triggered,
			this, &BasePlotView::on_action_add_marker_triggered);
		add_marker_menu_->removeAction(action);
		delete action;
	}

	// One add marker action for each curve
	for (const auto &curve : plot_->curves()) {
		QAction *action = new QAction(this);
		action->setText(curve->name());
		action->setData(QVariant::fromValue(curve));
		connect(action, &QAction::triggered,
			this, &BasePlotView::on_action_add_marker_triggered);
		add_marker_menu_->addAction(action);
	}
}

void BasePlotView::connect_signals()
{
}

void BasePlotView::save_settings(QSettings &settings) const
{
	BaseView::save_settings(settings);

	settings.setValue("markers_label_alignment",
		plot_->markers_label_alignment());
}

void BasePlotView::restore_settings(QSettings &settings)
{
	BaseView::restore_settings(settings);

	if (settings.contains("markers_label_alignment")) {
		plot_->set_markers_label_alignment(
			settings.value("markers_label_alignment").toInt());
	}
}

void BasePlotView::on_action_add_marker_triggered()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
		plot_->add_marker(action->data().value<widgets::plot::Curve *>());

	if (plot_->marker_curve_map().size() >= 2)
		action_add_diff_marker_->setDisabled(false);
	else
		action_add_diff_marker_->setDisabled(true);
}

void BasePlotView::on_action_add_diff_marker_triggered()
{
	ui::dialogs::PlotDiffMarkerDialog dlg(plot_);
	dlg.exec();
}

void BasePlotView::on_action_zoom_best_fit_triggered()
{
	plot_->set_all_axis_locked(false);
}

void BasePlotView::on_action_save_triggered()
{
	QString filter("SVG Image (*.svg);;PDF File (*.pdf)");
	for (const auto &supported : QImageWriter::supportedImageFormats()) {
		filter += ";;" + supported.toUpper() + " Image (*." + supported + ")";
	}
	QString *selected_filter = new QString("SVG Image (*.svg)");
	QString file_name = QFileDialog::getSaveFileName(this,
		tr("Save Plot"), QDir::homePath(), filter, selected_filter);
	delete selected_filter;
	if (file_name.length() <= 0)
		return;

	// TODO
	QSizeF size(300, 300);
	int resolution = 90;
	QwtPlotRenderer renderer;
	renderer.renderDocument(plot_, file_name, size, resolution);
}

void BasePlotView::on_action_config_plot_triggered()
{
	ui::dialogs::PlotConfigDialog dlg(plot_, plot_type_);
	dlg.exec();
}

} // namespace views
} // namespace ui
} // namespace sv
