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

#include <memory>
#include <vector>

#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QIcon>
#include <QSize>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "selectsignaldialog.hpp"
#include "src/session.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/widgets/signaltree.hpp"

using std::shared_ptr;
using std::vector;

namespace sv {
namespace dialogs {

SelectSignalDialog::SelectSignalDialog(const Session &session,
		const shared_ptr<devices::BaseDevice> device,
		QWidget *parent) :
	QDialog(parent),
	session_(session),
	device_(device)
{
	setup_ui();
}

void SelectSignalDialog::setup_ui()
{
	QIcon mainIcon;
	mainIcon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(mainIcon);
	this->setWindowTitle(tr("Select Signal"));
	this->setMinimumWidth(500);

	QVBoxLayout *main_layout = new QVBoxLayout;

	signal_tree_ = new widgets::SignalTree(session_, true, true, true, device_);
	main_layout->addWidget(signal_tree_);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(main_layout);
}

vector<shared_ptr<data::BaseSignal>> SelectSignalDialog::signals()
{
	return signals_;
}

void SelectSignalDialog::accept()
{
	for (auto signal : signal_tree_->selected_signals()) {
		signals_.push_back(signal);
	}

	QDialog::accept();
}

} // namespace dialogs
} // namespace sv
