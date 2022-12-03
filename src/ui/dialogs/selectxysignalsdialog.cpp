/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2020-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>

#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QSize>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "selectxysignalsdialog.hpp"
#include "src/session.hpp"
#include "src/data/basesignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/devices/selectsignalwidget.hpp"

using std::shared_ptr;

namespace sv {
namespace ui {
namespace dialogs {

SelectXYSignalsDialog::SelectXYSignalsDialog(const Session &session,
		const shared_ptr<sv::devices::BaseDevice> selected_device,
		QWidget *parent) :
	QDialog(parent),
	session_(session),
	selected_device_(selected_device)
{
	setup_ui();
}

void SelectXYSignalsDialog::setup_ui()
{
	QIcon main_icon;
	main_icon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(main_icon);
	this->setWindowTitle(tr("Select X/Y Signals"));
	this->setMinimumWidth(500);

	QVBoxLayout *main_layout = new QVBoxLayout;
	QHBoxLayout *signals_layout = new QHBoxLayout();

	QGroupBox *x_signal_group = new QGroupBox(tr("X Signal"));
	QVBoxLayout *x_layout = new QVBoxLayout();
	x_signal_widget_ = new ui::devices::SelectSignalWidget(session_);
	x_signal_widget_->select_device(selected_device_);
	x_layout->addWidget(x_signal_widget_);
	x_signal_group->setLayout(x_layout);
	signals_layout->addWidget(x_signal_group);

	QGroupBox *y_signal_group = new QGroupBox(tr("Y Signal"));
	QVBoxLayout *y_layout = new QVBoxLayout();
	y_signal_widget_ = new ui::devices::SelectSignalWidget(session_);
	y_signal_widget_->select_device(selected_device_);
	y_layout->addWidget(y_signal_widget_);
	y_signal_group->setLayout(y_layout);
	signals_layout->addWidget(y_signal_group);
	main_layout->addLayout(signals_layout);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, &QDialogButtonBox::accepted,
		this, &SelectXYSignalsDialog::accept);
	connect(button_box_, &QDialogButtonBox::rejected,
		this, &SelectXYSignalsDialog::reject);

	this->setLayout(main_layout);
}

shared_ptr<sv::data::BaseSignal> SelectXYSignalsDialog::x_signal()
{
	return x_signal_widget_->selected_signal();
}

shared_ptr<sv::data::BaseSignal> SelectXYSignalsDialog::y_signal()
{
	return y_signal_widget_->selected_signal();
}

} // namespace dialogs
} // namespace ui
} // namespace sv
