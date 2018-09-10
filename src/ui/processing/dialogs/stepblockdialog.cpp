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

#include <cassert>
#include <memory>

#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QMessageBox>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "stepblockdialog.hpp"
#include "src/devices/configurable.hpp"
#include "src/widgets/configkeycombobox.hpp"
#include "src/widgets/configurablecombobox.hpp"

using std::make_shared;
using std::static_pointer_cast;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace ui {
namespace processing {
namespace dialogs {

StepBlockDialog::StepBlockDialog(shared_ptr<Session> session,
		shared_ptr<devices::Configurable> configurable,
		QWidget *parent) :
	QDialog(parent),
	session_(session),
	configurable_(configurable)
{
	assert(configurable);

	setup_ui();
	connect_signals();
}

void StepBlockDialog::setup_ui()
{
	QIcon mainIcon;
	mainIcon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(mainIcon);
	this->setWindowTitle(tr("Step Sequence Block"));
	this->setMinimumWidth(550);

	QVBoxLayout *main_layout = new QVBoxLayout();

	// General stuff
	QFormLayout *form_layout = new QFormLayout();
	name_edit_ = new QLineEdit();
	form_layout->addRow(tr("Name"), name_edit_);
	configurable_box_ = new widgets::ConfigurableComboBox(session_);
	form_layout->addRow(tr("Channel (?)"), configurable_box_);
	config_key_box_ = new widgets::ConfigKeyComboBox(
		configurable_box_->selected_configurable(), false, true, false);
	form_layout->addRow(tr("Control (?)"), config_key_box_);
	start_value_ = new QDoubleSpinBox();
	start_value_->setSuffix(QString(" %1").arg("V"));
	start_value_->setDecimals(3);
	start_value_->setMinimum(0);
	start_value_->setMaximum(100);
	start_value_->setSingleStep(0.01);
	form_layout->addRow(tr("Start value"), start_value_);
	end_value_ = new QDoubleSpinBox();
	//end_value_->setSuffix(QString(" %1").arg("V"));
	end_value_->setDecimals(3);
	end_value_->setMinimum(0);
	end_value_->setMaximum(100);
	end_value_->setSingleStep(0.01);
	form_layout->addRow(tr("End value"), end_value_);
	step_size_ = new QDoubleSpinBox();
	//step_size_->setSuffix(QString(" %1").arg("V"));
	step_size_->setDecimals(3);
	step_size_->setMinimum(0);
	step_size_->setMaximum(100);
	step_size_->setSingleStep(0.01);
	form_layout->addRow(tr("Step size"), step_size_);
	delay_ms_ = new QSpinBox();
	delay_ms_->setSuffix(QString(" %1").arg("ms"));
	delay_ms_->setMinimum(0);
	delay_ms_->setMaximum(10000);
	delay_ms_->setSingleStep(1);
	form_layout->addRow(tr("Delay"), delay_ms_);
	main_layout->addLayout(form_layout);

	// Buttons
	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(main_layout);
}

void StepBlockDialog::connect_signals()
{
	connect(configurable_box_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(configurable_changed()));
}

void StepBlockDialog::accept()
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

shared_ptr<devices::Configurable> StepBlockDialog::configurable() const
{
	return configurable_box_->selected_configurable();
}

devices::ConfigKey StepBlockDialog::config_key() const
{
	return config_key_box_->selected_config_key();
}

double StepBlockDialog::start_value() const
{
	return start_value_->value();
}

double StepBlockDialog::end_value() const
{
	return end_value_->value();
}

double StepBlockDialog::step_size() const
{
	return step_size_->value();
}

int StepBlockDialog::delay_ms() const
{
	return delay_ms_->value();
}

void StepBlockDialog::configurable_changed()
{
	config_key_box_->set_configurable(
		configurable_box_->selected_configurable());
}

} // namespace dialogs
} // namespace processing
} // namespace ui
} // namespace sv
