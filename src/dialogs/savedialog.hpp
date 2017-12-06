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

#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QString>

#include "src/data/basesignal.hpp"

using std::shared_ptr;
using std::vector;

namespace sv {

namespace dialogs {

class SaveDialog : public QDialog
{
	Q_OBJECT

public:
	SaveDialog(const vector<shared_ptr<data::BaseSignal>> selected_signals,
		QWidget *parent = nullptr);

private:
	void setup_ui();
	void save(QString file_name);
	//void closeEvent(QCloseEvent *event);
	void done(int result);

	const vector<shared_ptr<data::BaseSignal>> selected_signals_;

	QDialogButtonBox *button_box_;

private Q_SLOTS:

};

} // namespace dialogs
} // namespace sv

#endif // DIALOGS_SAVEDIALOG_HPP
