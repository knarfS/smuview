/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <QAction>
#include <QCheckBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QList>
#include <QLocale>
#include <QMessageBox>
#include <QSettings>
#include <QSpinBox>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QTimer>
#include <QToolBar>
#include <QUuid>
#include <QVariant>
#include <QVBoxLayout>

#include "sequenceoutputview.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/util.hpp"
#include "src/data/properties/doubleproperty.hpp"
#include "src/ui/datatypes/doublespinbox.hpp"
#include "src/ui/dialogs/generatewaveformdialog.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/viewhelper.hpp"

using std::dynamic_pointer_cast;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sv {
namespace ui {
namespace views {

DoubleSpinBoxDelegate::DoubleSpinBoxDelegate(
		double min, double max, double step, int decimals, QObject *parent) :
	QStyledItemDelegate(parent),
	min_(min),
	max_(max),
	step_(step),
	decimals_(decimals)
{
}

QWidget *DoubleSpinBoxDelegate::createEditor(QWidget *parent,
	const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	(void)option;
	(void)index;
	QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
	editor->setFrame(false);
	editor->setMinimum(min_);
	editor->setMaximum(max_);
	editor->setSingleStep(step_);
	editor->setDecimals(decimals_);
	return editor;
}

QString DoubleSpinBoxDelegate::displayText(
	const QVariant &value, const QLocale &locale) const
{
	(void)locale;
	return QString("%L1").arg(value.toDouble(), 0, 'f', decimals_);
}

void DoubleSpinBoxDelegate::setEditorData(QWidget *editor,
	const QModelIndex &index) const
{
	double value = index.model()->data(index, Qt::EditRole).toDouble();
	QDoubleSpinBox *spin_box = qobject_cast<QDoubleSpinBox *>(editor);
	spin_box->setValue(value);
}

void DoubleSpinBoxDelegate::setModelData(QWidget *editor,
	QAbstractItemModel *model, const QModelIndex &index) const
{
	QDoubleSpinBox *spin_box = qobject_cast<QDoubleSpinBox *>(editor);
	spin_box->interpretText();
	model->setData(index, QVariant(spin_box->value()), Qt::EditRole);
}

void DoubleSpinBoxDelegate::updateEditorGeometry(QWidget *editor,
	const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	(void)index;
	editor->setGeometry(option.rect);
}


SequenceOutputView::SequenceOutputView(Session &session, QUuid uuid,
		QWidget *parent) :
	BaseView(session, uuid, parent),
	property_(nullptr),
	action_run_(new QAction(this)),
	action_add_row_(new QAction(this)),
	action_delete_row_(new QAction(this)),
	action_delete_all_(new QAction(this)),
	action_load_from_file_(new QAction(this)),
	action_generate_waveform_(new QAction(this)),
	sequence_pos_(0)
{
	id_ = "sequenceoutput:" + uuid_.toString(QUuid::WithoutBraces).toStdString();

	setup_ui();
	setup_toolbar();

	timer_ = new QTimer(this);
}

SequenceOutputView::~SequenceOutputView()
{
	stop_timer();
}

QString SequenceOutputView::title() const
{
	QString title = tr("Sequence Output");
	if (property_)
		title = title.append(" ").append(property_->display_name());
	return title;
}

void SequenceOutputView::set_property(
	shared_ptr<data::properties::DoubleProperty> property)
{
	assert(property);

	stop_timer();

	property_ = property;
	sequence_table_->setItemDelegateForColumn(0,
		new DoubleSpinBoxDelegate(property_->min(), property_->max(),
			property_->step(), property_->decimal_places()));

	Q_EMIT title_changed();
}

void SequenceOutputView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	QHBoxLayout *repeat_layout = new QHBoxLayout();
	repeat_layout->addWidget(new QLabel(tr("Repeat")));
	repeat_layout->addSpacing(8);
	repeat_infinite_box_ = new QCheckBox(tr("infinite"));
	repeat_infinite_box_->setChecked(true);
	connect(repeat_infinite_box_, &QCheckBox::stateChanged,
		this, &SequenceOutputView::on_repeat_infinite_changed);
	repeat_layout->addWidget(repeat_infinite_box_);
	repeat_layout->addSpacing(8);
	repeat_count_box_ = new QSpinBox();
	repeat_count_box_->setValue(1);
	repeat_count_box_->setMinimum(1);
	repeat_count_box_->setMaximum(1000000);
	repeat_count_box_->setSuffix(tr(" cycle(s)"));
	repeat_count_box_->setDisabled(true);
	repeat_layout->addWidget(repeat_count_box_);
	repeat_layout->addStretch(1);
	layout->addItem(repeat_layout);

	sequence_table_ = new QTableWidget();
	sequence_table_->setColumnCount(2);
	QTableWidgetItem *value_header_item = new QTableWidgetItem(tr("Value"));
	sequence_table_->setHorizontalHeaderItem(0, value_header_item);
	sequence_table_->horizontalHeader()->setSectionResizeMode(
		0, QHeaderView::Stretch);
	QTableWidgetItem *delay_header_item = new QTableWidgetItem(tr("Delay [s]"));
	sequence_table_->setHorizontalHeaderItem(1, delay_header_item);
	sequence_table_->horizontalHeader()->setSectionResizeMode(
		1, QHeaderView::Stretch);
	sequence_table_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	sequence_table_->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	sequence_table_->setItemDelegateForColumn(1,
		new DoubleSpinBoxDelegate(0, 100000, 0.1, 3));

	//sequence_table_->setRowCount(1);
	layout->addWidget(sequence_table_);

	this->central_widget_->setLayout(layout);
}

void SequenceOutputView::setup_toolbar()
{
	action_run_->setText(tr("Run generator"));
	action_run_->setIcon(
		QIcon::fromTheme("media-playback-start",
		QIcon(":/icons/media-playback-start.png")));
	action_run_->setCheckable(true);
	action_run_->setChecked(false);
	connect(action_run_, &QAction::triggered,
		this, &SequenceOutputView::on_action_run_triggered);

	action_add_row_->setText(tr("Insert row"));
	action_add_row_->setIcon(
		QIcon::fromTheme("edit-table-insert-row-under",
		QIcon(":/icons/edit-table-insert-row-under.png")));
	connect(action_add_row_, &QAction::triggered,
		this, &SequenceOutputView::on_action_add_row);

	action_delete_row_->setText(tr("Delete row"));
	action_delete_row_->setIcon(
		QIcon::fromTheme("edit-table-delete-row",
		QIcon(":/icons/edit-table-delete-row.png")));
	connect(action_delete_row_, &QAction::triggered,
		this, &SequenceOutputView::on_action_delete_row);

	action_delete_all_->setText(tr("Delete all"));
	action_delete_all_->setIcon(
		QIcon::fromTheme("edit-delete",
		QIcon(":/icons/edit-delete.png")));
	connect(action_delete_all_, &QAction::triggered,
		this, &SequenceOutputView::on_action_delete_all);

	action_load_from_file_->setText(tr("Load from file"));
	action_load_from_file_->setIcon(
		QIcon::fromTheme("document-open",
		QIcon(":/icons/document-open.png")));
	connect(action_load_from_file_, &QAction::triggered,
		this, &SequenceOutputView::on_action_load_from_file_triggered);

	action_generate_waveform_->setText(tr("Run generator"));
	action_generate_waveform_->setIcon(
		QIcon::fromTheme("office-chart-line", // TODO: better icon
		QIcon(":/icons/office-chart-line.png")));
	connect(action_generate_waveform_, &QAction::triggered,
		this, &SequenceOutputView::on_action_generate_waveform_triggered);

	toolbar_ = new QToolBar("Generator Toolbar");
	toolbar_->addAction(action_run_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_add_row_);
	toolbar_->addAction(action_delete_row_);
	toolbar_->addAction(action_delete_all_);
	toolbar_->addSeparator();
	toolbar_->addAction(action_load_from_file_);
	toolbar_->addAction(action_generate_waveform_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void SequenceOutputView::save_settings(QSettings &settings) const
{
	BaseView::save_settings(settings);

	if (!property_)
		return;
	SettingsManager::save_property(property_, settings);

	settings.setValue("repeat_infinite",
		QVariant(repeat_infinite_box_->checkState()));
	settings.setValue("repeat_count", QVariant(repeat_count_box_->value()));

	// Save sequence
	int row_count = sequence_table_->rowCount();
	settings.setValue("sequence_row_count", QVariant(row_count));
	for (int pos=0; pos<row_count; pos++) {
		QTableWidgetItem *value_item = sequence_table_->item(pos, 0);
		QTableWidgetItem *delay_item = sequence_table_->item(pos, 1);
		if (!value_item || !delay_item)
			continue;

		settings.beginGroup(QString("sequence_").append(QString::number(pos)));
		settings.setValue("value", value_item->data(0));
		settings.setValue("delay", delay_item->data(0));
		settings.endGroup();
	}
}

void SequenceOutputView::restore_settings(QSettings &settings)
{
	BaseView::restore_settings(settings);

	auto property = SettingsManager::restore_property(session_, settings);
	if (!property)
		return;
	set_property(
		dynamic_pointer_cast<sv::data::properties::DoubleProperty>(property));

	if (settings.contains("repeat_infinite"))
		repeat_infinite_box_->setCheckState(
			settings.value("repeat_infinite").value<Qt::CheckState>());
	if (settings.contains("repeat_count"))
		repeat_count_box_->setValue(settings.value("repeat_count").toInt());

	// Restore sequence
	int row_count = settings.value("sequence_row_count").toInt();
	for (int pos=0; pos<row_count; pos++) {
		settings.beginGroup(QString("sequence_").append(QString::number(pos)));
		QVariant value = settings.value("value");
		QVariant delay = settings.value("delay");
		sequence_table_->insertRow(pos);
		QTableWidgetItem *value_item = new QTableWidgetItem(QString::number(
			value.toDouble(), 'f', 3));
		value_item->setData(0, value);
		sequence_table_->setItem(pos, 0, value_item);
		QTableWidgetItem *delay_item = new QTableWidgetItem(QString::number(
			delay.toDouble(), 'f', property_->decimal_places()));
		delay_item->setData(0, delay);
		sequence_table_->setItem(pos, 1, delay_item);
		settings.endGroup();
	}
}

void SequenceOutputView::start_timer()
{
	if (timer_->isActive()) {
		timer_->stop();
		disconnect(timer_, &QTimer::timeout,
			this, &SequenceOutputView::on_timer_update);
	}

	sequence_pos_ = 0;
	sequence_reperat_count_ = 0;
	if (sequence_table_->rowCount() == 0)
		return;

	connect(timer_, &QTimer::timeout,
		this, &SequenceOutputView::on_timer_update);
	timer_->start();

	action_run_->setText(tr("Stop"));
	action_run_->setIcon(
		QIcon::fromTheme("media-playback-stop",
		QIcon(":/icons/media-playback-stop.png")));
	action_run_->setChecked(true);
}

void SequenceOutputView::stop_timer()
{
	action_run_->setText(tr("Run"));
	action_run_->setIcon(
		QIcon::fromTheme("media-playback-start",
		QIcon(":/icons/media-playback-start.png")));
	action_run_->setChecked(false);

	if (!timer_->isActive())
		return;

	timer_->stop();
	disconnect(timer_, &QTimer::timeout,
		this, &SequenceOutputView::on_timer_update);
	sequence_pos_ = 0;
	sequence_reperat_count_ = 0;
}

void SequenceOutputView::insert_row(int row, double value, double delay)
{
	if (!property_) {
		QMessageBox::warning(this, tr("No property assigned."),
			tr("Please assign a property to this sequence output view first."),
			QMessageBox::Ok);
	}

	sequence_table_->insertRow(row);
	QTableWidgetItem *value_item = new QTableWidgetItem(
		QString::number(value, 'f', 3));
	value_item->setData(0, QVariant(value));
	sequence_table_->setItem(row, 0, value_item);
	QTableWidgetItem *delay_item = new QTableWidgetItem(
		QString::number(delay, 'f', property_->decimal_places()));
	delay_item->setData(0, QVariant(delay));
	sequence_table_->setItem(row, 1, delay_item);
}

void SequenceOutputView::on_timer_update()
{
	if (!property_) {
		stop_timer();
		return;
	}

	bool found_value = sequence_pos_ > 0;
	double value = .0;
	int delay_ms = 0;
	// Cycle through the row until a duration is found.
	do {
		if (sequence_table_->rowCount() == 0) {
			stop_timer();
			return;
		}
		if (sequence_pos_ >= sequence_table_->rowCount()) {
			// Jump up to the first row
			sequence_pos_ = 0;
			if (!found_value) {
				stop_timer();
				return;
			}
			if (!repeat_infinite_box_->isChecked()) {
				sequence_reperat_count_++;
				if (sequence_reperat_count_ >= repeat_count_box_->value()) {
					stop_timer();
					return;
				}
			}
		}

		QTableWidgetItem *value_item = sequence_table_->item(sequence_pos_, 0);
		if (value_item)
			value = value_item->data(0).toDouble();
		QTableWidgetItem *delay_item = sequence_table_->item(sequence_pos_, 1);
		if (delay_item)
			delay_ms = (int)std::round(delay_item->data(0).toDouble() * 1000);

		sequence_table_->selectRow(sequence_pos_);
		sequence_pos_++;
	}
	while (delay_ms <= 0);

	property_->change_value(value);
	timer_->setInterval(delay_ms);
}

void SequenceOutputView::on_repeat_infinite_changed()
{
	if (repeat_infinite_box_->isChecked())
		repeat_count_box_->setDisabled(true);
	else
		repeat_count_box_->setDisabled(false);
}

void SequenceOutputView::on_action_run_triggered()
{
	if (action_run_->isChecked())
		start_timer();
	else
		stop_timer();
}

void SequenceOutputView::on_action_add_row()
{
	int row = sequence_table_->currentRow() + 1;
	insert_row(row, .0, .0);
}

void SequenceOutputView::on_action_delete_row()
{
	// NOTE: If the cells are empty, there is no item to be returned by
	//       selectedItems() and selectedIndexes() is protected...
	const auto items = sequence_table_->selectedItems();
	for (const auto &item : items) {
		sequence_table_->removeRow(item->row());
	}
}

void SequenceOutputView::on_action_delete_all()
{
	sequence_table_->setRowCount(0);
}

void SequenceOutputView::on_action_load_from_file_triggered()
{
	QString file_name = QFileDialog::getOpenFileName(this,
		tr("Open Sequence-File"), QDir::homePath(), tr("CSV Files (*.csv)"));
	if (file_name.length() <= 0)
		return;

	std::ifstream file(file_name.toStdString());
	if (file.is_open()) {
		string line;
		int row = 0;
		while (std::getline(file, line)) {
			auto fields = sv::util::parse_csv_line(line);

			// TODO: Define CSV file format somehow/somewhere...
			// TODO: Parse header
			if (fields.size() < 2)
				continue;
			bool ok;
			// NOTE: QString::toDouble() to avoid locales and use the C locale
			double value = QString::fromStdString(fields[0]).toDouble(&ok);
			if (!ok)
				continue;
			// NOTE: QString::toDouble() to avoid locales and use the C locale
			double delay = QString::fromStdString(fields[1]).toDouble(&ok);
			if (!ok)
				continue;

			insert_row(row, value, delay);
			row++;
		}
	}
	file.close();
}

void SequenceOutputView::on_action_generate_waveform_triggered()
{
	if (!property_) {
		QMessageBox::warning(this, tr("No property assigned."),
			tr("Please assign a property to this sequence output view first."),
			QMessageBox::Ok);
	}

	ui::dialogs::GenerateWaveformDialog dlg(property_);
	if (!dlg.exec())
		return;

	vector<double> sequence_values = dlg.sequence_values();
	vector<double> sequence_delays = dlg.sequence_delays();
	for (size_t i=0; i<sequence_values.size(); ++i) {
		insert_row(i, sequence_values[i], sequence_delays[i]);
	}
}

} // namespace views
} // namespace ui
} // namespace sv
