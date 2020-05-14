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

#include <cmath>
#include <map>

#include <QApplication>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLineEdit>
#include <QString>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTabWidget>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include "plotconfigdialog.hpp"
#include "src/data/datautil.hpp"
#include "src/ui/views/plotview.hpp"
#include "src/ui/widgets/plot/plot.hpp"

Q_DECLARE_METATYPE(sv::ui::widgets::plot::PlotUpdateMode)

namespace sv {
namespace ui {
namespace dialogs {

ColorItemDelegate::ColorItemDelegate(QObject *parent) :
	QStyledItemDelegate(parent)
{
}

QWidget *ColorItemDelegate::createEditor(QWidget *parent,
	const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	(void)option;
	(void)index;

	QColorDialog *dialog = new QColorDialog(parent);
    dialog->setModal(true);
    return dialog;
}

void ColorItemDelegate::paint(QPainter *painter,
	const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	// Set background color
	QColor color = index.model()->data(index, Qt::EditRole).value<QColor>();
	painter->fillRect(option.rect, color);

	// Set text and text color depending on background color
	QStyleOptionViewItem item_option(option);
    initStyleOption(&item_option, index);
	item_option.text = color.name();
	float brightness = std::sqrt(
		color.red() * color.red() * 0.241 +
		color.green() * color.green() * 0.691 +
		color.blue() * color.blue() * 0.068);
	if (brightness >= 130)
		item_option.palette.setColor(QPalette::Text, Qt::black);
	else
		item_option.palette.setColor(QPalette::Text, Qt::white);
    QApplication::style()->drawControl(
		QStyle::CE_ItemViewItem, &item_option, painter);
}

void ColorItemDelegate::setEditorData(QWidget *editor,
	const QModelIndex &index) const
{
	QColor color = index.model()->data(index, Qt::EditRole).value<QColor>();
	QColorDialog *dialog = qobject_cast<QColorDialog *>(editor);
	dialog->setCurrentColor(color);
}

void ColorItemDelegate::setModelData(QWidget *editor,
	QAbstractItemModel *model, const QModelIndex &index) const
{
	QColorDialog *dialog = qobject_cast<QColorDialog *>(editor);
	model->setData(index, QVariant(dialog->currentColor()), Qt::EditRole);
}

void ColorItemDelegate::updateEditorGeometry(QWidget *editor,
	const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	(void)index;

	editor->setGeometry(option.rect);
}


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
	this->setup_ui_curve_colors_tab();
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

	// TODO: Plot background color? Axis position? What else?

	widget->setLayout(layout);
	tab_widget_->addTab(widget, title);
}

void PlotConfigDialog::setup_ui_curve_colors_tab()
{
	QString title(tr("Curve Colors"));

	QWidget *widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();

	QTableWidget *color_table_ = new QTableWidget();
	color_table_->setColumnCount(2);

	QTableWidgetItem *quantity_header_item = new QTableWidgetItem(tr("Quantity"));
	quantity_header_item->setTextAlignment(Qt::AlignVCenter);
	color_table_->setHorizontalHeaderItem(0, quantity_header_item);
	color_table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	QTableWidgetItem *color_header_item = new QTableWidgetItem(tr("Color"));
	color_header_item->setTextAlignment(Qt::AlignVCenter);
	color_table_->setHorizontalHeaderItem(1, color_header_item);
	color_table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

	color_table_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	color_table_->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	color_table_->setSelectionMode(QTableWidget::NoSelection);
	color_table_->setItemDelegateForColumn(1, new ColorItemDelegate());

	for (const auto &q_n_pair : data::datautil::get_quantity_name_map()) {
		int last_row = color_table_->rowCount();
		color_table_->insertRow(last_row);
		QTableWidgetItem *quantity_item = new QTableWidgetItem(q_n_pair.second);
		quantity_item->setFlags(quantity_item->flags() ^ Qt::ItemIsEditable);
		color_table_->setItem(last_row, 0, quantity_item);
		QTableWidgetItem *color_item = new QTableWidgetItem(q_n_pair.second);
		color_table_->setItem(last_row, 1, color_item);

		// For Voltage and Current, we want to make AC/DC also configurable.
		if (q_n_pair.first == data::Quantity::Voltage ||
				q_n_pair.first == data::Quantity::Current) {
			for (auto const &qf : {data::QuantityFlag::AC, data::QuantityFlag::DC}) {
				QString name = QString("%1 %2").arg(
					q_n_pair.second, data::datautil::format_quantity_flag(qf));
				int last_row = color_table_->rowCount();
				color_table_->insertRow(last_row);
				QTableWidgetItem *quantity_item = new QTableWidgetItem(name);
				quantity_item->setFlags(quantity_item->flags() ^ Qt::ItemIsEditable);
				color_table_->setItem(last_row, 0, quantity_item);
				QTableWidgetItem *color_item = new QTableWidgetItem(name);
				color_table_->setItem(last_row, 1, color_item);
			}
		}
	}

	layout->addWidget(color_table_);

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

	// TODO: Set color to QSettings?

	QDialog::accept();
}

} // namespace dialogs
} // namespace ui
} // namespace sv
