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

#include <map>

#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QIcon>
#include <QLineEdit>
#include <QString>
#include <QTabWidget>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include "plotconfigdialog.hpp"
#include "src/ui/views/plotview.hpp"
#include "src/ui/widgets/plot/plot.hpp"

Q_DECLARE_METATYPE(sv::ui::widgets::plot::PlotUpdateMode)

namespace sv {
namespace ui {
namespace dialogs {

PlotConfigDialog::PlotConfigDialog(widgets::plot::Plot *plot,
		views::PlotType plot_type, QWidget *parent) :
	QDialog(parent),
	plot_(plot),
	plot_type_(plot_type)
{
	setup_ui();
}

void PlotConfigDialog::setup_ui()
{
	QIcon main_icon;
	main_icon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(main_icon);
	this->setWindowTitle(tr("Plot Config"));
	this->setMinimumWidth(500);

	QVBoxLayout *main_layout = new QVBoxLayout();

	// Tabs
	tab_widget_ = new QTabWidget();
	if (plot_type_ == views::PlotType::TimePlot) {
		this->setup_ui_plot_mode_tab();
	}
	this->setup_ui_markers_tab();
	//this->setup_ui_style_tab();
	tab_widget_->setCurrentIndex(0);
	main_layout->addWidget(tab_widget_);

	// Buttons
	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(main_layout);
}

void PlotConfigDialog::setup_ui_plot_mode_tab()
{
	QString title(tr("Plot mode"));

	QWidget *widget = new QWidget();
	QFormLayout *layout = new QFormLayout();

	plot_update_mode_combobox_ = new QComboBox();
	size_t cb_index = 0;
	for (const auto &update_mode_pair : widgets::plot::plot_update_mode_name_map) {
		plot_update_mode_combobox_->addItem(
			update_mode_pair.second,
			QVariant::fromValue(update_mode_pair.first));
		if (plot_->update_mode() == update_mode_pair.first)
			plot_update_mode_combobox_->setCurrentIndex(cb_index);
		++cb_index;
	}
	connect(plot_update_mode_combobox_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_update_mode_changed()));
	layout->addRow(tr("Plot mode"), plot_update_mode_combobox_);

	time_span_edit_ = new QLineEdit();
	time_span_edit_->setValidator(new QDoubleValidator);
	time_span_edit_->setText(QString("%1").arg(plot_->time_span(), 0, 'f'));
	layout->addRow(tr("Time span"), time_span_edit_);

	add_time_edit_ = new QLineEdit();
	add_time_edit_->setValidator(new QDoubleValidator);
	add_time_edit_->setText(QString("%1").arg(plot_->add_time(), 0, 'f'));
	layout->addRow(tr("Add time"), add_time_edit_);

	switch (plot_->update_mode()) {
	case widgets::plot::PlotUpdateMode::Additive:
		setup_ui_additive();
		break;
	case widgets::plot::PlotUpdateMode::Rolling:
		setup_ui_rolling();
		break;
	case widgets::plot::PlotUpdateMode::Oscilloscope:
		setup_ui_oscilloscope();
		break;
	}

	widget->setLayout(layout);
	tab_widget_->addTab(widget, title);
}

void PlotConfigDialog::setup_ui_markers_tab()
{
	QString title(tr("Markers"));

	QWidget *widget = new QWidget();
	QFormLayout *layout = new QFormLayout();

	markers_box_pos_combobox_ = new QComboBox();
	markers_box_pos_combobox_->addItem(tr("Top left"), QVariant::fromValue(
		(int)(Qt::AlignmentFlag::AlignTop | Qt::AlignmentFlag::AlignLeft)));
	markers_box_pos_combobox_->addItem(tr("Top center"), QVariant::fromValue(
		(int)(Qt::AlignmentFlag::AlignTop | Qt::AlignmentFlag::AlignHCenter)));
	markers_box_pos_combobox_->addItem(tr("Top right"), QVariant::fromValue(
		(int)(Qt::AlignmentFlag::AlignTop | Qt::AlignmentFlag::AlignRight)));
	markers_box_pos_combobox_->addItem(tr("Center left"), QVariant::fromValue(
		(int)(Qt::AlignmentFlag::AlignVCenter | Qt::AlignmentFlag::AlignLeft)));
	markers_box_pos_combobox_->addItem(tr("Center"), QVariant::fromValue(
		(int)(Qt::AlignmentFlag::AlignVCenter | Qt::AlignmentFlag::AlignHCenter)));
	markers_box_pos_combobox_->addItem(tr("Center right"), QVariant::fromValue(
		(int)(Qt::AlignmentFlag::AlignVCenter | Qt::AlignmentFlag::AlignRight)));
	markers_box_pos_combobox_->addItem(tr("Bottom left"), QVariant::fromValue(
		(int)(Qt::AlignmentFlag::AlignBottom | Qt::AlignmentFlag::AlignLeft)));
	markers_box_pos_combobox_->addItem(tr("Bottom center"), QVariant::fromValue(
		(int)(Qt::AlignmentFlag::AlignBottom | Qt::AlignmentFlag::AlignHCenter)));
	markers_box_pos_combobox_->addItem(tr("Bottom right"), QVariant::fromValue(
		(int)(Qt::AlignmentFlag::AlignBottom | Qt::AlignmentFlag::AlignRight)));

	for (int i = 0; i < markers_box_pos_combobox_->count(); ++i) {
		QVariant data_var = markers_box_pos_combobox_->itemData(i);
		if (data_var.toInt() == plot_->markers_label_alignment()) {
			markers_box_pos_combobox_->setCurrentIndex(i);
			break;
		}
	}

	layout->addRow(tr("Info box position"), markers_box_pos_combobox_);
	widget->setLayout(layout);
	tab_widget_->addTab(widget, title);
}

void PlotConfigDialog::setup_ui_style_tab()
{
	QString title(tr("Style"));

	QWidget *widget = new QWidget();
	QFormLayout *layout = new QFormLayout();

	widget->setLayout(layout);
	tab_widget_->addTab(widget, title);
}

void PlotConfigDialog::setup_ui_additive()
{
	time_span_edit_->setDisabled(true);
	add_time_edit_->setDisabled(false);
}

void PlotConfigDialog::setup_ui_rolling()
{
	time_span_edit_->setDisabled(false);
	add_time_edit_->setDisabled(false);
}

void PlotConfigDialog::setup_ui_oscilloscope()
{
	time_span_edit_->setDisabled(false);
	add_time_edit_->setDisabled(true);
}

void PlotConfigDialog::on_update_mode_changed()
{
	QVariant update_mode_var = plot_update_mode_combobox_->currentData();
	switch (update_mode_var.value<sv::ui::widgets::plot::PlotUpdateMode>()) {
	case widgets::plot::PlotUpdateMode::Additive:
		setup_ui_additive();
		break;
	case widgets::plot::PlotUpdateMode::Rolling:
		setup_ui_rolling();
		break;
	case widgets::plot::PlotUpdateMode::Oscilloscope:
		setup_ui_oscilloscope();
		break;
	}
}

void PlotConfigDialog::accept()
{
	if (plot_type_ == views::PlotType::TimePlot) {
		QVariant update_mode_var = plot_update_mode_combobox_->currentData();
		sv::ui::widgets::plot::PlotUpdateMode update_mode =
			update_mode_var.value<sv::ui::widgets::plot::PlotUpdateMode>();
		plot_->set_update_mode(update_mode);
		if (update_mode == widgets::plot::PlotUpdateMode::Rolling ||
				update_mode == widgets::plot::PlotUpdateMode::Oscilloscope)
			plot_->set_time_span(time_span_edit_->text().toDouble());
		if (update_mode == widgets::plot::PlotUpdateMode::Additive ||
				update_mode == widgets::plot::PlotUpdateMode::Rolling)
			plot_->set_add_time(add_time_edit_->text().toDouble());
	}

	plot_->set_markers_label_alignment(
		markers_box_pos_combobox_->currentData().toInt());

	QDialog::accept();
}

} // namespace dialogs
} // namespace ui
} // namespace sv
