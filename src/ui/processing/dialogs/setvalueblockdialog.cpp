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

#include "setvalueblockdialog.hpp"
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

SetValueBlockDialog::SetValueBlockDialog(shared_ptr<Session> session,
		QWidget *parent) :
	QDialog(parent),
	session_(session)
{
	setup_ui();
	connect_signals();
}

void SetValueBlockDialog::setup_ui()
{
	QIcon mainIcon;
	mainIcon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(mainIcon);
	this->setWindowTitle(tr("Set Value Block"));
	this->setMinimumWidth(550);

	QVBoxLayout *main_layout = new QVBoxLayout();

	// General stuff
	QFormLayout *form_layout = new QFormLayout();
	name_edit_ = new QLineEdit();
	form_layout->addRow(tr("Name"), name_edit_);
	configurable_box_ = new widgets::ConfigurableComboBox(session_);
	form_layout->addRow(tr("Channel"), configurable_box_);
	config_key_box_ = new widgets::ConfigKeyComboBox(
		configurable_box_->selected_configurable());
	form_layout->addRow(tr("Control"), config_key_box_);
	// Values (controlled by SLOT)
	value_double_ = new QDoubleSpinBox();
	value_double_->setHidden(true);
	form_layout->addRow(tr("Double Value"), value_double_);
	value_int_ = new QSpinBox();
	value_int_->setHidden(true);
	form_layout->addRow(tr("Integer Value"), value_int_);
	value_string_ = new QLineEdit();
	value_string_->setHidden(true);
	form_layout->addRow(tr("String Value"), value_string_);

	main_layout->addLayout(form_layout);

	// Buttons
	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(main_layout);
}

void SetValueBlockDialog::connect_signals()
{
	connect(configurable_box_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_configurable_changed()));
	connect(config_key_box_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_config_key_changed()));
}

void SetValueBlockDialog::accept()
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

shared_ptr<devices::Configurable> SetValueBlockDialog::configurable() const
{
	return configurable_box_->selected_configurable();
}

devices::ConfigKey SetValueBlockDialog::config_key() const
{
	return config_key_box_->selected_config_key();
}

double SetValueBlockDialog::value_double() const
{
	return value_double_->value();
}

int SetValueBlockDialog::value_int()
{
	return value_int_->value();
}

QString SetValueBlockDialog::value_string()
{
	return value_string_->text();
}

void SetValueBlockDialog::on_configurable_changed()
{
	config_key_box_->set_configurable(
		configurable_box_->selected_configurable());
}

void SetValueBlockDialog::on_config_key_changed()
{
	devices::ConfigKey ck = config_key_box_->selected_config_key();
	devices::DataType dt =
		devices::deviceutil::get_data_type_for_config_key(ck);

	switch (dt) {
	case devices::DataType::Float:
		//value_double_->setSuffix(QString(" %1").arg("V"));
		value_double_->setDecimals(3);
		value_double_->setMinimum(0);
		value_double_->setMaximum(100);
		value_double_->setSingleStep(0.01);
		value_double_->setHidden(false);
		value_int_->setHidden(true);
		value_string_->setHidden(true);
		break;
	case devices::DataType::Int32:
		//value_int_->setSuffix(QString(" %1").arg("V"));
		value_int_->setMinimum(0);
		value_int_->setMaximum(100);
		value_int_->setSingleStep(1);
		value_int_->setHidden(false);
		value_double_->setHidden(true);
		value_string_->setHidden(true);
		break;
	case devices::DataType::Sting:
		value_string_->setHidden(false);
		value_double_->setHidden(true);
		value_int_->setHidden(true);
		break;
	default:
		return;
	}
}

} // namespace dialogs
} // namespace processing
} // namespace ui
} // namespace sv
