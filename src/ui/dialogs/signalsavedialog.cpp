/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2026 Frank Stettner <frank-stettner@gmx.net>
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

#include <fstream>
#include <limits>
#include <memory>
#include <string>

#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

#include "signalsavedialog.hpp"
#include "src/settingsmanager.hpp"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogtimesignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/ui/devices/devicetree/devicetreeview.hpp"
#include "src/utils/exportutil.hpp"

using std::dynamic_pointer_cast;
using std::ofstream;
using std::string;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace ui {
namespace dialogs {

SignalSaveDialog::SignalSaveDialog(const Session &session,
		const shared_ptr<sv::devices::BaseDevice> selected_device,
		QWidget *parent) :
	QDialog(parent),
	session_(session),
	selected_device_(selected_device)
{
	setup_ui();

	QSettings settings;
	if (SettingsManager::restore_settings() &&
			settings.childGroups().contains("SignalSaveDialog")) {
		restore_settings(settings);
	}
	else {
		file_dialog_path_ = QDir::homePath();
	}
}

void SignalSaveDialog::setup_ui()
{
	QIcon main_icon;
	main_icon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(main_icon);
	this->setWindowTitle(tr("Save Signals"));
	this->setMinimumWidth(450);
	this->setMinimumHeight(400);

	QVBoxLayout *main_layout = new QVBoxLayout;

	device_tree_ = new ui::devices::devicetree::DeviceTreeView(
		session_, false, false, false, true, false, false, false, false);
	device_tree_->expand_device(selected_device_);
	device_tree_->check_signals(selected_device_->signals());
	main_layout->addWidget(device_tree_);

	QFormLayout *form_layout = new QFormLayout();

	timestamps_combined_ = new QCheckBox(tr("Combine all timestamps"));
	form_layout->addRow("", timestamps_combined_);

	timestamps_combined_timeframe_ = new QSpinBox();
	timestamps_combined_timeframe_->setValue(0);
	timestamps_combined_timeframe_->setRange(
		0, std::numeric_limits<int32_t>::max());
	timestamps_combined_timeframe_->setSuffix(" ms");
	timestamps_combined_timeframe_->setDisabled(
		!timestamps_combined_->isChecked());
	form_layout->addRow(tr("Combination time frame"),
		timestamps_combined_timeframe_);

	time_absolut_ = new QCheckBox(tr("Absolut time"));
	form_layout->addRow("", time_absolut_);

	separator_box_ = new QComboBox();
	separator_box_->addItem(tr("Comma (,)"), ",");
	separator_box_->addItem(tr("Semicolon (;)"), ";");
	separator_box_->addItem(tr("Tab (\\t)"), "\t");
	separator_box_->addItem(tr("Pipe (|)"), "|");
	separator_box_->addItem(tr("Space"), " ");
	separator_box_->addItem(tr("Custom..."));
	form_layout->addRow(tr("CSV separator"), separator_box_);
	separator_edit_ = new QLineEdit();
	separator_edit_->setMaxLength(1);
	separator_edit_->setFixedWidth(40);
	separator_edit_->setDisabled(true);
	separator_edit_->setText("");
	form_layout->addRow(tr("Custom separator"), separator_edit_);

	main_layout->addLayout(form_layout);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);


#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(timestamps_combined_, &QCheckBox::checkStateChanged,
		this, &SignalSaveDialog::on_combined_toggled);
#else
	connect(timestamps_combined_, &QCheckBox::stateChanged,
		this, &SignalSaveDialog::on_combined_toggled);
#endif
	connect(separator_box_, QOverload<int>::of(&QComboBox::currentIndexChanged),
		 this, &SignalSaveDialog::on_separator_changed);
	connect(button_box_, &QDialogButtonBox::accepted,
		this, &SignalSaveDialog::accept);
	connect(button_box_, &QDialogButtonBox::rejected,
		this, &SignalSaveDialog::reject);

	this->setLayout(main_layout);
}

bool SignalSaveDialog::save_csv(const QString &file_name)
{
	vector<shared_ptr<sv::data::AnalogTimeSignal>> time_signals;
	for (const auto &signal : device_tree_->checked_signals()) {
		auto analog_signal =
			dynamic_pointer_cast<sv::data::AnalogTimeSignal>(signal);
		if (analog_signal)
			time_signals.push_back(analog_signal);
	}

	double combined_timeframe = 0.;
	int combined_timeframe_ms = timestamps_combined_timeframe_->value();
	if (combined_timeframe_ms != 0)
		combined_timeframe = ((double)combined_timeframe_ms) / 1000;

	bool relative_time = !time_absolut_->isChecked();

	QString separator_str = separator_box_->currentData().isValid()
		? separator_box_->currentData().toString()
		: separator_edit_->text();
	char separator = separator_str.isEmpty()
		? ',' : separator_str.at(0).toLatin1();

	ofstream output_file(file_name.toStdString());
	if (!output_file.is_open()) {
		QMessageBox::warning(this,
			tr("Failed to open file"),
			tr("An error occured while opening the file %1.").arg(file_name),
			QMessageBox::Ok);
		return false;
	}

	if (timestamps_combined_->isChecked()) {
		auto aligned_data = sv::utils::exportutil::align_timestamps(
			time_signals, relative_time, combined_timeframe);
		sv::utils::exportutil::export_csv_aligned(
			output_file, aligned_data, separator);
	}
	else {
		sv::utils::exportutil::export_csv_parallel(
			output_file, time_signals, separator, relative_time);
	}

	output_file.close();

	return true;
}

void SignalSaveDialog::save_settings(QSettings &settings) const
{
	settings.beginGroup("SignalSaveDialog");
	settings.remove("");  // Remove all keys in this group

	settings.setValue("timestamps_combined", timestamps_combined_->isChecked());
	settings.setValue("timestamps_combined_timeframe",
		timestamps_combined_timeframe_->value());
	settings.setValue("time_absolut", time_absolut_->isChecked());
	settings.setValue("csv_separator", separator_box_->currentData().isValid()
		? separator_box_->currentData().toString()
		: separator_edit_->text());
	settings.setValue("file_dialog_path", file_dialog_path_);

	settings.endGroup();
}

void SignalSaveDialog::restore_settings(QSettings &settings)
{
	settings.beginGroup("SignalSaveDialog");

	if (settings.contains("timestamps_combined")) {
		timestamps_combined_->setChecked(
			settings.value("timestamps_combined").toBool());
	}
	if (settings.contains("timestamps_combined_timeframe")) {
		timestamps_combined_timeframe_->setValue(
			settings.value("timestamps_combined_timeframe").toInt());
	}
	if (settings.contains("time_absolut")) {
		time_absolut_->setChecked(settings.value("time_absolut").toBool());
	}
	if (settings.contains("csv_separator")) {
		QString sep = settings.value("csv_separator").toString();
		int index = separator_box_->findData(sep);
		if (index < 0) {
			separator_edit_->setText(sep);
			separator_edit_->setDisabled(false);
			separator_box_->setCurrentIndex(
				separator_box_->count() - 1);
		}
		else
			separator_box_->setCurrentIndex(index);
	}
	if (settings.contains("file_dialog_path")) {
		file_dialog_path_ =
			settings.value("file_dialog_path", QDir::homePath()).toString();
	}

	settings.endGroup();
}

void SignalSaveDialog::accept()
{
	// TODO: validate inputs: separator not \", \n, \r, !Latin1

	// Get file name
	QString file_name = QFileDialog::getSaveFileName(this,
		tr("Save CSV-File"), file_dialog_path_, tr("CSV Files (*.csv)"));
	if (file_name.isEmpty())
		return;

	file_dialog_path_ = QDir().absoluteFilePath(file_name);
	if (!save_csv(file_name))
		QDialog::reject();
	else
		QDialog::accept();
}

void SignalSaveDialog::done(int result)
{
	QSettings settings;
	save_settings(settings);

	QDialog::done(result);
}

void SignalSaveDialog::on_combined_toggled()
{
	timestamps_combined_timeframe_->setDisabled(
		!timestamps_combined_->isChecked());
}

void SignalSaveDialog::on_separator_changed()
{
	separator_edit_->setDisabled(
		separator_box_->currentData().isValid());
}

} // namespace dialogs
} // namespace ui
} // namespace sv
