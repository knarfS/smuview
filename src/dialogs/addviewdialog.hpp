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

#ifndef DIALOGS_ADDVIEWDIALOG_HPP
#define DIALOGS_ADDVIEWDIALOG_HPP

#include <QDialog>
#include <QDialogButtonBox>

#include "src/session.hpp"

namespace sv {

namespace dialogs {

class AddViewDialog : public QDialog
{
	Q_OBJECT

public:
	AddViewDialog(const Session &session, QWidget *parent = nullptr);

private:
	void setup_ui();

	const Session &session_;

	QDialogButtonBox *button_box_;

public Q_SLOTS:
	void accept() override;

private Q_SLOTS:

};

} // namespace dialogs
} // namespace sv

#endif // DIALOGS_ADDVIEWDIALOG_HPP
