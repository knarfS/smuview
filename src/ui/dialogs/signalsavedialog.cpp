/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
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
#include <iostream>
#include <fstream>
#include <limits>
#include <memory>
#include <string>

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

	separator_edit_ = new QLineEdit();
	separator_edit_->setText(",");
	form_layout->addRow(tr("CSV separator"), separator_edit_);

	main_layout->addLayout(form_layout);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);

	connect(timestamps_combined_, SIGNAL(stateChanged(int)),
		this, SLOT(toggle_combined()));
	connect(button_box_, &QDialogButtonBox::accepted,
		this, &SignalSaveDialog::accept);
	connect(button_box_, &QDialogButtonBox::rejected,
		this, &SignalSaveDialog::reject);

	this->setLayout(main_layout);
}

void SignalSaveDialog::save(const QString &file_name)
{
	ofstream output_file;
	string str_file_name = file_name.toStdString();
	vector<size_t> sample_counts;

	output_file.open(str_file_name);

	auto signals = device_tree_->checked_signals();
	bool relative_time = !time_absolut_->isChecked();
	string sep = separator_edit_->text().toStdString();
	size_t max_sample_count = 0;

	// Header
	string start_sep;
	string device_header_line;
	string chg_name_header_line;
	string ch_name_header_line;
	string signal_name_header_line;
	for (const auto &signal : signals) {
		// Only handle AnalogSignals
		auto analog_signal =
			dynamic_pointer_cast<sv::data::AnalogTimeSignal>(signal);
		if (!analog_signal)
			continue;

		size_t sample_count = analog_signal->sample_count();
		if (sample_count > max_sample_count)
			max_sample_count = sample_count;
		sample_counts.push_back(sample_count);

		string name = analog_signal->name();
		shared_ptr<sv::channels::BaseChannel> parent_channel =
			analog_signal->parent_channel();

		qWarning() << "SaveDialog::save(): signal.name() = " <<
			QString::fromStdString(name);
		qWarning() << "SaveDialog::save(): signal.parent_channel().name() = " <<
			QString::fromStdString(parent_channel->name());
		qWarning() << "SaveDialog::save(): signal.parent_channel().parent_device().name() = " <<
			QString::fromStdString(parent_channel->parent_device()->name());

		string chg_names;
		string chg_sep;
		for (const auto &chg_name : parent_channel->channel_group_names()) {
			chg_names += chg_sep;
			if (chg_name.empty())
				chg_names += "\"\"";
			else
				chg_names += chg_name;
			// TODO: Ugly workaround. Implement escaping or quotation characters?
			chg_sep = sep == "," ? "; " : ", ";
		}

		device_header_line += start_sep;
		device_header_line += parent_channel->parent_device()->name(); // Time
		device_header_line += sep;
		device_header_line += parent_channel->parent_device()->name(); // Value
		chg_name_header_line += start_sep;
		chg_name_header_line += chg_names; // Time
		chg_name_header_line += sep;
		chg_name_header_line += chg_names; // Value
		ch_name_header_line += start_sep;
		ch_name_header_line += parent_channel->name(); // Time
		ch_name_header_line += sep;
		ch_name_header_line += parent_channel->name(); // Value
		signal_name_header_line += start_sep;
		signal_name_header_line += "Time ";
		signal_name_header_line += name; // Time
		signal_name_header_line += sep;
		signal_name_header_line += name; // Value

		start_sep = sep;
	}
	output_file << device_header_line << std::endl;
	output_file << chg_name_header_line << std::endl;
	output_file << ch_name_header_line << std::endl;
	output_file << signal_name_header_line << std::endl;

	// Data
	// TODO: we asume here, that the vector size is the same for all vectors....
	for (size_t i = 0; i < max_sample_count; i++) {
		start_sep = "";
		QString line("");
		int j = 0;
		for (const auto &signal : signals) {
			// Only handle AnalogSignals
			auto analog_signal =
				dynamic_pointer_cast<sv::data::AnalogTimeSignal>(signal);
			if (!analog_signal)
				continue;

			QString time("");
			QString value("");

			size_t sample_count = sample_counts[j];
			if (i < sample_count-1) {
				// More samples for this signal
				auto sample = analog_signal->get_sample(i, relative_time);
				value = QString("%1").arg(sample.second);
				if (relative_time)
					time = QString("%1").arg(sample.first, 0, 'f', 4);
				else
					time = util::format_time_date(sample.first);
			}

			line.append(QString("%1%2%3%4").arg(
				QString::fromStdString(start_sep), time,
				QString::fromStdString(sep), value));
			start_sep = sep;

			++j;
		}
		output_file << line.toStdString() << std::endl;
	}

	output_file.close();
}

void SignalSaveDialog::save_combined(const QString &file_name)
{
	ofstream output_file;
	string str_file_name = file_name.toStdString();
	vector<size_t> sample_counts;
	vector<size_t> sample_pos;

	output_file.open(str_file_name);

	double combined_timeframe = .0;
	int combined_timeframe_ms = timestamps_combined_timeframe_->value();
	if (combined_timeframe_ms != 0)
		combined_timeframe = ((double)combined_timeframe_ms) / 1000;

	auto signals = device_tree_->checked_signals();
	bool relative_time = !time_absolut_->isChecked();
	string sep = separator_edit_->text().toStdString();

	// Header
	string device_header_line("Time"); // Time
	string chg_name_header_line("Time"); // Time
	string ch_name_header_line("Time"); // Time
	string signal_name_header_line("Time"); // Time
	for (const auto &signal : signals) {
		// Only handle AnalogSignals
		auto analog_signal =
			dynamic_pointer_cast<sv::data::AnalogTimeSignal>(signal);
		if (!analog_signal)
			continue;

		shared_ptr<sv::channels::BaseChannel> parent_channel =
			analog_signal->parent_channel();

		sample_counts.push_back(analog_signal->sample_count());
		sample_pos.push_back(0);

		string chg_names;
		string chg_sep;
		for (const auto &chg_name : parent_channel->channel_group_names()) {
			chg_names += chg_sep;
			if (chg_name.empty())
				chg_names += "\"\"";
			else
				chg_names += chg_name;
			// TODO: Ugly workaround. Implement escaping or quotation characters?
			chg_sep = sep == "," ? "; " : ", ";
		}

		device_header_line += sep;
		device_header_line += parent_channel->parent_device()->name(); // Value
		chg_name_header_line += sep;
		chg_name_header_line += chg_names; // Value
		ch_name_header_line += sep;
		ch_name_header_line += parent_channel->name(); // Value
		signal_name_header_line += sep;
		signal_name_header_line += analog_signal->name(); // Value
	}
	output_file << device_header_line << std::endl;
	output_file << chg_name_header_line << std::endl;
	output_file << ch_name_header_line << std::endl;
	output_file << signal_name_header_line << std::endl;

	// Data
	while (true) {
		double next_timestamp = -1;
		int i = 0;
		for (const auto &signal : signals) {
			// Only handle AnalogSignals
			auto analog_signal =
				dynamic_pointer_cast<sv::data::AnalogTimeSignal>(signal);
			if (!analog_signal)
				continue;

			if (sample_pos[i] >= sample_counts[i]-1)
				continue;

			double timestamp =
				analog_signal->get_sample(sample_pos[i], relative_time).first;
			if (next_timestamp < 0 || timestamp < next_timestamp)
				next_timestamp = timestamp;

			++i;
		}

		if (next_timestamp < 0)
			break;

		// Timestamp
		QString line;
		if (relative_time)
			line = QString("%1").arg(next_timestamp, 0, 'f', 4);
		else
			line = util::format_time_date(next_timestamp);

		// Values
		i = 0;
		for (const auto &signal : signals) {
			// Only handle AnalogSignals
			auto analog_signal =
				dynamic_pointer_cast<sv::data::AnalogTimeSignal>(signal);
			if (!analog_signal)
				continue;

			line.append(QString::fromStdString(sep));

			auto sample =
				analog_signal->get_sample(sample_pos[i], relative_time);
			double timestamp = sample.first;
			if (timestamp + combined_timeframe >= next_timestamp) {
				line.append(QString("%1").arg(sample.second, 0, 'g', -1));
				++sample_pos[i];
			}

			++i;
		}
		output_file << line.toStdString() << std::endl;
	}

	output_file.close();
}

bool SignalSaveDialog::validate_combined_timeframe()
{
	int combined_timeframe_ms = timestamps_combined_timeframe_->value();
	if (combined_timeframe_ms == 0)
		return true;
	const double combined_timeframe = ((double)combined_timeframe_ms) / 1000;

	size_t num_signals = device_tree_->checked_signals().size();
	size_t act_signal = 0;
	QProgressDialog progress(tr("Validating combined timeframe ..."),
		tr("Abort validation"), 0, num_signals, this);
	progress.setMinimumDuration(500);
	progress.setWindowModality(Qt::WindowModal);

	double min_delta = combined_timeframe;
	for (const auto &signal : device_tree_->checked_signals()) {
		progress.setValue(act_signal++);
		size_t count = signal->sample_count();
		if (count < 2)
			continue;

		// Only handle AnalogSignals
		auto analog_signal =
			dynamic_pointer_cast<sv::data::AnalogTimeSignal>(signal);
		if (!analog_signal)
			continue;

		double ts1 = analog_signal->get_sample(0, false).first;
		for (size_t i = 1; i<count; i++) {
			const double ts2 = analog_signal->get_sample(i, false).first;
			const double delta = ts2 - ts1;
			if (delta < min_delta)
				min_delta = delta;
			ts1 = ts2;

			if (progress.wasCanceled())
				return false;
		}
	}

	progress.setValue(num_signals);

	if (min_delta < combined_timeframe) {
		int min_delta_ms = (int)std::floor(min_delta * 1000);
		QMessageBox::critical(this,
			tr("Combination time frame too large"),
			tr("The combination time frame is too large. Time span must be "
				"smaller than %1 ms.").arg(min_delta_ms),
			QMessageBox::Ok);
		timestamps_combined_timeframe_->setValue(min_delta_ms - 1);
		return false;
	}
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
	settings.setValue("csv_separator", separator_edit_->text());
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
		separator_edit_->setText(settings.value("csv_separator").toString());
	}
	if (settings.contains("file_dialog_path")) {
		file_dialog_path_ =
			settings.value("file_dialog_path", QDir::homePath()).toString();
	}

	settings.endGroup();
}

void SignalSaveDialog::accept()
{
	// Get file name
	QString file_name = QFileDialog::getSaveFileName(this,
		tr("Save CSV-File"), file_dialog_path_, tr("CSV Files (*.csv)"));
	if (file_name.isEmpty())
		return;

	file_dialog_path_ = QDir().absoluteFilePath(file_name);

	if (timestamps_combined_->isChecked()) {
		if (!validate_combined_timeframe())
			return;
		save_combined(file_name);
	}
	else {
		save(file_name);
	}

	QDialog::accept();
}

void SignalSaveDialog::done(int r)
{
	QSettings settings;
	save_settings(settings);

	QDialog::done(r);
}

void SignalSaveDialog::toggle_combined()
{
	timestamps_combined_timeframe_->setDisabled(
		!timestamps_combined_->isChecked());
}

} // namespace dialogs
} // namespace ui
} // namespace sv
