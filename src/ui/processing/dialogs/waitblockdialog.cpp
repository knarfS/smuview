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

#include <QDebug>
#include <QFormLayout>
#include <QIcon>
#include <QMessageBox>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "waitblockdialog.hpp"

namespace sv {
namespace ui {
namespace processing {
namespace dialogs {

WaitBlockDialog::WaitBlockDialog(shared_ptr<Session> session,
		QWidget *parent) :
	QDialog(parent),
	session_(session)
{
	setup_ui();
	connect_signals();
}

void WaitBlockDialog::setup_ui()
{
	QIcon mainIcon;
	mainIcon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(mainIcon);
	this->setWindowTitle(tr("Wait Block"));
	this->setMinimumWidth(550);

	QVBoxLayout *main_layout = new QVBoxLayout();

	// General stuff
	QFormLayout *form_layout = new QFormLayout();
	name_edit_ = new QLineEdit();
	form_layout->addRow(tr("Name"), name_edit_);
	wait_ms_ = new QSpinBox();
	wait_ms_->setSuffix(QString(" %1").arg("ms"));
	wait_ms_->setMinimum(0);
	wait_ms_->setMaximum(10000);
	wait_ms_->setSingleStep(1);
	form_layout->addRow(tr("Wait"), wait_ms_);
	main_layout->addLayout(form_layout);

	// Buttons
	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(main_layout);
}

void WaitBlockDialog::connect_signals()
{
}

void WaitBlockDialog::accept()
{
	if (name_edit_->text().size() == 0) {
		QMessageBox::warning(this,
			tr("Block name missing"),
			tr("Please enter a name for the new block."),
			QMessageBox::Ok);
		return;
	}

	QDialog::accept();
}

int WaitBlockDialog::wait_ms() const
{
	return wait_ms_->value();
}

} // namespace dialogs
} // namespace processing
} // namespace ui
} // namespace sv
