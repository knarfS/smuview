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

#ifndef UI_PROCESSING_DIALOGS_WAITBLOCKDIALOG_HPP
#define UI_PROCESSING_DIALOGS_WAITBLOCKDIALOG_HPP

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QSpinBox>

#include "src/session.hpp"

namespace sv {
namespace ui {
namespace processing {
namespace dialogs {

class WaitBlockDialog : public QDialog
{
	Q_OBJECT

public:
	WaitBlockDialog(const Session &session,
		QWidget *parent = nullptr);

	QString name();
	int wait_ms() const;

private:
	const Session &session_;

	QLineEdit *name_edit_;
	QSpinBox *wait_ms_;
	QDialogButtonBox *button_box_;

	void setup_ui();
	void connect_signals();

public Q_SLOTS:
	void accept() override;

};

} // namespace dialogs
} // namespace processing
} // namespace ui
} // namespace sv

#endif // UI_PROCESSING_DIALOGS_WAITBLOCKDIALOG_HPP
