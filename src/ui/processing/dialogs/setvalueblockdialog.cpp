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
#include "src/devices/properties/baseproperty.hpp"
#include "src/ui/datatypes/datatypehelper.hpp"
#include "src/widgets/configkeycombobox.hpp"
#include "src/widgets/configurablecombobox.hpp"

using std::make_shared;
using std::shared_ptr;
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
	form_layout_ = new QFormLayout();
	name_edit_ = new QLineEdit();
	form_layout_->addRow(tr("Name"), name_edit_);
	configurable_box_ = new widgets::ConfigurableComboBox(session_);
	form_layout_->addRow(tr("Channel"), configurable_box_);
	config_key_box_ = new widgets::ConfigKeyComboBox(
		configurable_box_->selected_configurable(), false, true, false);
	form_layout_->addRow(tr("Control"), config_key_box_);

	property_ = configurable_box_->selected_configurable()->get_property(
		config_key_box_->selected_config_key());
	value_widget_ = datatypes::datatypehelper::get_widget_for_property(
		property_, false, false);
	form_layout_->addRow(tr("Value"), value_widget_);

	main_layout->addLayout(form_layout_);

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
	connect(config_key_box_,
		SIGNAL(current_config_key_changed(const devices::ConfigKey)),
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

shared_ptr<devices::properties::BaseProperty>
	SetValueBlockDialog::property() const
{
	return property_;
}

shared_ptr<devices::Configurable> SetValueBlockDialog::configurable() const
{
	return configurable_box_->selected_configurable();
}

devices::ConfigKey SetValueBlockDialog::config_key() const
{
	return config_key_box_->selected_config_key();
}

QVariant SetValueBlockDialog::value() const
{
	return QVariant((double)0.5); //value_widget_->value();
}

void SetValueBlockDialog::on_configurable_changed()
{
	config_key_box_->set_configurable(
		configurable_box_->selected_configurable());
}

void SetValueBlockDialog::on_config_key_changed()
{
	auto c = configurable_box_->selected_configurable();
	auto ck = config_key_box_->selected_config_key();
	property_ = c->get_property(ck);

	// Dummy widget if there is no widget for this property. Otherwise the NULL
	// widget can't be replaced with another property widget.
	QWidget *new_value_widget = new QWidget();
	if (property_ != nullptr) {
		new_value_widget = datatypes::datatypehelper::get_widget_for_property(
			property_, false, false);
	}

	form_layout_->replaceWidget(value_widget_, new_value_widget);
	delete value_widget_;
	value_widget_ = new_value_widget;
}

} // namespace dialogs
} // namespace processing
} // namespace ui
} // namespace sv
