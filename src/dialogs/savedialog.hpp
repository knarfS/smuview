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

#ifndef DIALOGS_SAVEDIALOG_HPP
#define DIALOGS_SAVEDIALOG_HPP

#include <memory>
#include <vector>

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QString>
#include <QTreeWidget>

#include "src/session.hpp"

using std::shared_ptr;
using std::vector;

namespace sv {

namespace data {
class AnalogSignal;
}

namespace widgets {
class SignalTree;
}

namespace dialogs {

class SaveDialog : public QDialog
{
	Q_OBJECT

public:
	SaveDialog(const Session &session,
		const vector<shared_ptr<data::AnalogSignal>> selected_signals,
		QWidget *parent = nullptr);

private:
	void setup_ui();
	void save(QString file_name);
	void save_combined(QString file_name);

	const Session &session_;
	const vector<shared_ptr<data::AnalogSignal>> selected_signals_;

	widgets::SignalTree *signal_tree_;
	QCheckBox *timestamps_combined_;
	QCheckBox *time_absolut_;
	QLineEdit *separator_edit_;
	QDialogButtonBox *button_box_;

public Q_SLOTS:
	void accept() override;

private Q_SLOTS:

};

} // namespace dialogs
} // namespace sv

#endif // DIALOGS_SAVEDIALOG_HPP
