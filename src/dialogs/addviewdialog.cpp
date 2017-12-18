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

#include <QFormLayout>

#include "addviewdialog.hpp"

namespace sv {
namespace dialogs {

AddViewDialog::AddViewDialog(const Session &session, QWidget *parent) :
	QDialog(parent),
	session_(session)
{
	setup_ui();
}

void AddViewDialog::setup_ui()
{
	setWindowTitle(tr("Add View"));

	QFormLayout *form_layout = new QFormLayout();

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	form_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(form_layout);
}

void AddViewDialog::accept()
{
	QDialog::accept();
}

} // namespace dialogs
} // namespace sv
