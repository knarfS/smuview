/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>
#include <set>

#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QSize>
#include <QString>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include "addplotcurvedialog.hpp"
#include "src/session.hpp"
#include "src/data/basesignal.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/devices/selectsignalwidget.hpp"
#include "src/ui/widgets/plot/plothelper.hpp"
#include "src/ui/widgets/plot/scopeplot.hpp"

using std::shared_ptr;

Q_DECLARE_METATYPE(QwtPlot::Axis)

namespace sv {
namespace ui {
namespace dialogs {

AddPlotCurveDialog::AddPlotCurveDialog(const Session &session,
		const ui::widgets::plot::ScopePlot *plot,
		const shared_ptr<sv::devices::BaseDevice> selected_device,
		QWidget *parent) :
	QDialog(parent),
	session_(session),
	plot_(plot),
	selected_device_(selected_device)
{
	setup_ui();
}

void AddPlotCurveDialog::setup_ui()
{
	QIcon main_icon;
	main_icon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(main_icon);
	this->setWindowTitle(tr("Add Curve"));
	this->setMinimumWidth(500);

	QVBoxLayout *main_layout = new QVBoxLayout();

	QGroupBox *signal_group = new QGroupBox(tr("Signal"));
	QVBoxLayout *signal_layout = new QVBoxLayout();
	signal_widget_ = new devices::SelectSignalWidget(session_);
	signal_widget_->select_device(selected_device_);
	connect(signal_widget_, &devices::SelectSignalWidget::signal_changed,
		this, &AddPlotCurveDialog::on_signal_changed);
	signal_layout->addWidget(signal_widget_);
	signal_group->setLayout(signal_layout);
	main_layout->addWidget(signal_group);

	QHBoxLayout *axis_layout = new QHBoxLayout();

	QGroupBox *y_axis_group = new QGroupBox(tr("Y Axis"));
	QVBoxLayout *y_axis_layout = new QVBoxLayout();
	y_axis_box_ = new QComboBox();
	fill_y_axis_box();
	y_axis_layout->addWidget(y_axis_box_);
	y_axis_group->setLayout(y_axis_layout);
	axis_layout->addWidget(y_axis_group);

	QGroupBox *x_axis_group = new QGroupBox(tr("X Axis"));
	QVBoxLayout *x_axis_layout = new QVBoxLayout();
	x_axis_box_ = new QComboBox();
	fill_x_axis_box();
	x_axis_layout->addWidget(x_axis_box_);
	x_axis_group->setLayout(x_axis_layout);
	axis_layout->addWidget(x_axis_group);

	main_layout->addLayout(axis_layout);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, &QDialogButtonBox::accepted,
		this, &AddPlotCurveDialog::accept);
	connect(button_box_, &QDialogButtonBox::rejected,
		this, &AddPlotCurveDialog::reject);

	this->setLayout(main_layout);
}

shared_ptr<sv::data::BaseSignal> AddPlotCurveDialog::signal() const
{
	return signal_widget_->selected_signal();
}

QwtPlot::Axis AddPlotCurveDialog::x_axis_id() const
{
	return x_axis_box_->currentData().value<QwtPlot::Axis>();
}

QwtPlot::Axis AddPlotCurveDialog::y_axis_id() const
{
	return y_axis_box_->currentData().value<QwtPlot::Axis>();
}

void AddPlotCurveDialog::fill_x_axis_box()
{
	x_axis_box_->clear();

	auto free_x_axes = plot_->get_free_x_axes(
		data::Unit::Second, set<data::QuantityFlag>(), true);
	for (const auto &axis_id : free_x_axes) {
		x_axis_box_->addItem(
			ui::widgets::plot::plothelper::format_axis(axis_id),
			QVariant::fromValue(axis_id));
	}
}

void AddPlotCurveDialog::fill_y_axis_box()
{
	y_axis_box_->clear();

	auto free_y_axes = plot_->get_free_y_axes(
		signal_widget_->selected_signal()->unit(),
		signal_widget_->selected_signal()->quantity_flags(), true);
	for (const auto &axis_id : free_y_axes) {
		y_axis_box_->addItem(
			ui::widgets::plot::plothelper::format_axis(axis_id),
			QVariant::fromValue(axis_id));
	}
}

void AddPlotCurveDialog::on_signal_changed()
{
	fill_x_axis_box();
	fill_y_axis_box();
}

} // namespace dialogs
} // namespace ui
} // namespace sv
