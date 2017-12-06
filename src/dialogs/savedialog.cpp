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
#include "src/data/analogdata.hpp"

using std::ofstream;
using std::string;

namespace sv {
namespace dialogs {

SaveDialog::SaveDialog(
		const vector<shared_ptr<data::BaseSignal>> selected_signals,
		QWidget *parent) :
	QDialog(parent),
	selected_signals_(selected_signals)
{
	setup_ui();
}

void SaveDialog::setup_ui()
{
	setWindowTitle(tr("Save"));

	QFormLayout *form_layout = new QFormLayout();

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	form_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(form_layout);
}

void SaveDialog::save(QString file_name)
{
	ofstream output_file;
	string str_file_name = file_name.toStdString();
	size_t signal_count = 0;
	size_t sample_count = 0;

	// create and open the .csv file
	output_file.open(str_file_name);

	// TODO: No tailing ","

	for (auto signal : selected_signals_)
	{
		// TODO signal names
		if (signal) {
			sample_count = signal->analog_data()->get_sample_count();
			output_file << "Time" << signal_count << ","
				<< "Signal" << signal_count << ",";
			signal_count++;
		}
	}
	output_file << std::endl;

	// TODO: we asume here, that the vector size is the same for all vectors....

	for (size_t i = 0; i < sample_count; i++)
	{
		for (size_t j = 0; j < signal_count; j++) {
			output_file << selected_signals_.at(j)->time_data()->get_sample(i)
				<< "," << selected_signals_.at(j)->analog_data()->get_sample(i)
				<< ",";
		}
		output_file << std::endl;
	}

	// close the output file
	output_file.close();
}

/*
void SaveDialog::closeEvent(QCloseEvent *event)
{
	if (maybeSave()) {
		writeSettings();
		event->accept(); // close window
	} else {
		event->ignore(); // keep window
	}
}
*/

void SaveDialog::done(int result)
{
	if(QDialog::Accepted == result) {
		// Get file name
		QString file_name = QFileDialog::getSaveFileName(this,
			tr("Save CSV-File"), QDir::homePath(), tr("CSV Files (*.csv)"));

		if (file_name.length() > 0) {
			save(file_name);
			QDialog::done(result);
			return;
		} else
			return;
	}
	else {
		// cancel, close or exc was pressed
		QDialog::done(result);
		return;
	}
}

} // namespace dialogs
} // namespace sv
