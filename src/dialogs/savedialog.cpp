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

#include <iostream>
#include <fstream>
#include <string>

#include <QDir>
#include <QFileDialog>
#include <QFormLayout>

#include "savedialog.hpp"
#include "src/data/analogsignal.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/device.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/widgets/signaltree.hpp"

using std::ofstream;
using std::string;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace dialogs {

SaveDialog::SaveDialog(const Session &session,
		const vector<shared_ptr<data::AnalogSignal>> selected_signals,
		QWidget *parent) :
	QDialog(parent),
	session_(session),
	selected_signals_(selected_signals)
{
	setup_ui();
}

void SaveDialog::setup_ui(){
	setWindowTitle(tr("Save"));

	QFormLayout *form_layout = new QFormLayout();

	signal_tree_ = new widgets::SignalTree(session_, true, nullptr);
	form_layout->addWidget(signal_tree_);

	timestamps_combined_ = new QCheckBox(tr("Combine all time stamps"));
	form_layout->addRow(timestamps_combined_);

	time_absolut_ = new QCheckBox(tr("Absolut time"));
	form_layout->addRow(time_absolut_);

	separator_edit_ = new QLineEdit();
	separator_edit_->setText(",");
	form_layout->addRow(tr("CSV separator"), separator_edit_);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	form_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(form_layout);
}

void SaveDialog::save(QString file_name, QString separator)
{
	ofstream output_file;
	string str_file_name = file_name.toStdString();
	string str_separator = separator.toStdString();
	size_t signal_count = 0;
	size_t sample_count = 0;

	output_file.open(str_file_name);

	// TODO: No tailing ","

	for (auto signal : selected_signals_) {
		// TODO signal names
		if (signal) {
			sample_count = signal->get_sample_count();
			output_file << "Time" << signal_count << str_separator
				<< signal->name().toStdString() << str_separator;
			signal_count++;
		}
	}
	output_file << std::endl;

	// TODO: we asume here, that the vector size is the same for all vectors....

	for (size_t i = 0; i < sample_count; i++) {
		for (size_t j = 0; j < signal_count; j++) {
			data::sample_t sample = selected_signals_.at(j)->get_sample(i);
			output_file
				<< sample.first << str_separator
				<< sample.first << str_separator;
		}
		output_file << std::endl;
	}

	output_file.close();
}

void SaveDialog::save_combined(QString file_name, QString separator)
{
	ofstream output_file;
	string str_file_name = file_name.toStdString();
	string str_separator = separator.toStdString();
	vector<shared_ptr<data::AnalogSignal>> print_signals;
	size_t signals_count = 0;
	vector<size_t> signal_size;
	vector<size_t> signal_pos;

/*
	auto smart_ptr = std::make_shared<QFile>();
	QVariant var = QVariant::fromValue(smart_ptr);
	// ...
	if (var.canConvert<QObject*>()) {
		QObject *sp = var.value<QObject*>();
		qDebug() << sp->metaObject()->className(); // Prints 'QFile'.
	}

	for (auto item : signal_tree_->selectedItems()) {
		item->data(0, Qt::UserRole).
		print_signals.push_back(
			(shared_ptr<data::AnalogSignal>).value());
	}
*/

	auto devices = session_.devices();
	for (auto device : devices) {
		auto hw_device = static_pointer_cast<devices::HardwareDevice>(device);
		for (shared_ptr<data::BaseSignal> signal : hw_device->all_signals()) {
			print_signals.push_back(
				static_pointer_cast<data::AnalogSignal>(signal));
		}
	}

	output_file.open(str_file_name);

	// TODO signal names
	output_file << "Time";
	for (auto signal : print_signals) {
		output_file << str_separator;
		if (signal) {
			signal_size.push_back(signal->get_sample_count());
			signal_pos.push_back(0);
			signals_count++;
			output_file << signal->name().toStdString();
		}
	}
	output_file << std::endl;

	bool finish = false;
	while (!finish) {
		double next_timestamp = -1;
		for (size_t i=0; i<signals_count; i++) {
			double signal_timestamp = print_signals.at(i)->
				get_sample(signal_pos.at(i)).first;

			if (next_timestamp < 0 || signal_timestamp < next_timestamp)
				next_timestamp = signal_timestamp;
		}
		output_file << next_timestamp;

		for (size_t i=0; i<signals_count; i++) {
			output_file << str_separator;

			if (print_signals.at(i)->get_sample_count() <= signal_pos.at(i)) {
				finish = true;
				continue;
			}
			finish = false;

			data::sample_t sample = print_signals.at(i)->
				get_sample(signal_pos.at(i));

			double signal_timestamp = sample.first;
			if (signal_timestamp == next_timestamp) {
				output_file << sample.second;
				++signal_pos.at(i);
			}
		}
		output_file << std::endl;
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
			save_combined(file_name, separator_edit_->text());
		else
			save(file_name, separator_edit_->text());

		QDialog::accept();
	}
}

} // namespace dialogs
} // namespace sv
