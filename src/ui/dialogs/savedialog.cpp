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

#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "savedialog.hpp"
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

SaveDialog::SaveDialog(const Session &session,
		const shared_ptr<sv::devices::BaseDevice> selected_device,
		QWidget *parent) :
	QDialog(parent),
	session_(session),
	selected_device_(selected_device)
{
	setup_ui();
}

void SaveDialog::setup_ui()
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

	timestamps_combined_ = new QCheckBox(tr("Combine all time stamps"));
	form_layout->addRow("", timestamps_combined_);

	time_absolut_ = new QCheckBox(tr("Absolut time"));
	form_layout->addRow("", time_absolut_);

	separator_edit_ = new QLineEdit();
	separator_edit_->setText(",");
	form_layout->addRow(tr("CSV separator"), separator_edit_);

	main_layout->addLayout(form_layout);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(main_layout);
}

void SaveDialog::save(QString file_name)
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
	string start_sep("");
	string device_header_line("");
	string chg_name_header_line("");
	string ch_name_header_line("");
	string signal_name_header_line("");
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

		string chg_names("");
		string chg_sep("");
		for (const auto &chg_name : parent_channel->channel_group_names()) {
			chg_names += chg_sep;
			if (chg_name.empty())
				chg_names += "\"\"";
			else
				chg_names += chg_name;
			chg_sep = ", ";
		}

		device_header_line += start_sep + parent_channel->parent_device()->name(); // Time
		device_header_line += sep + parent_channel->parent_device()->name(); // Value
		chg_name_header_line += start_sep + chg_names; // Time
		chg_name_header_line += sep + chg_names; // Value
		ch_name_header_line += start_sep + parent_channel->name(); // Time
		ch_name_header_line += sep + parent_channel->name(); // Value
		signal_name_header_line += start_sep + "Time " + name; // Time
		signal_name_header_line += sep + name; // Value

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
					time = QString("%1").arg(sample.first);
				else
					time = util::format_time_date(sample.first);
			}

			line.append(QString("%1%2%3%4").
				arg(QString::fromStdString(start_sep)).arg(time).
				arg(QString::fromStdString(sep)).arg(value));
			start_sep = sep;

			++j;
		}
		output_file << line.toStdString() << std::endl;
	}

	output_file.close();
}

void SaveDialog::save_combined(QString file_name)
{
	ofstream output_file;
	string str_file_name = file_name.toStdString();
	vector<size_t> sample_counts;
	vector<size_t> sample_pos;

	output_file.open(str_file_name);

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

		string chg_names("");
		string chg_sep("");
		for (const auto &chg_name : parent_channel->channel_group_names()) {
			chg_names += chg_sep;
			if (chg_name.empty())
				chg_names += "\"\"";
			else
				chg_names += chg_name;
			chg_sep = ", ";
		}

		device_header_line += sep + parent_channel->parent_device()->name(); // Value
		chg_name_header_line += sep + chg_names; // Value
		ch_name_header_line += sep + parent_channel->name(); // Value
		signal_name_header_line += sep + analog_signal->name(); // Value
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
			if (timestamp == next_timestamp) {
				line.append(QString("%1").arg(sample.second, 0, 'g', -1));
				++sample_pos[i];
			}

			++i;
		}
		output_file << line.toStdString() << std::endl;
	}

	output_file.close();
}

void SaveDialog::accept()
{
	// Get file name
	QString file_name = QFileDialog::getSaveFileName(this,
		tr("Save CSV-File"), QDir::homePath(), tr("CSV Files (*.csv)"));

	if (file_name.length() > 0) {
		if (timestamps_combined_->isChecked())
			save_combined(file_name);
		else
			save(file_name);

		QDialog::accept();
	}
}

} // namespace dialogs
} // namespace ui
} // namespace sv
