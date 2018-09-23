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

#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "stepblockdialog.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/properties/baseproperty.hpp"
#include "src/ui/datatypes/basewidget.hpp"
#include "src/ui/datatypes/datatypehelper.hpp"
#include "src/ui/devices/selectconfigkeyform.hpp"

using std::shared_ptr;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace ui {
namespace processing {
namespace dialogs {

StepBlockDialog::StepBlockDialog(const Session &session, QWidget *parent) :
	QDialog(parent),
	session_(session)
{
	setup_ui();
	connect_signals();
}

void StepBlockDialog::setup_ui()
{
	QIcon main_icon;
	main_icon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(main_icon);
	this->setWindowTitle(tr("Step Sequence Block"));
	this->setMinimumWidth(550);

	QVBoxLayout *main_layout = new QVBoxLayout();

	// General stuff
	form_layout_ = new QFormLayout();
	name_edit_ = new QLineEdit();
	form_layout_->addRow(tr("Name"), name_edit_);

	// Config Key
	QGroupBox *ck_group = new QGroupBox(tr("Config Key"));
	config_key_form_ = new ui::devices::SelectConfigKeyForm(
		session_, false, true, false);
	ck_group->setLayout(config_key_form_);
	form_layout_->addRow(ck_group);

	property_ = config_key_form_->get_property();
	start_value_box_ = datatypes::datatypehelper::get_widget_for_property(
		property_, false, false);
	form_layout_->addRow(tr("Start value"), start_value_box_);
	end_value_box_ = datatypes::datatypehelper::get_widget_for_property(
		property_, false, false);
	form_layout_->addRow(tr("End value"), end_value_box_);
	step_size_box_ = datatypes::datatypehelper::get_widget_for_property(
		property_, false, false);
	form_layout_->addRow(tr("Step size"), step_size_box_);

	delay_ms_box_ = new QSpinBox();
	delay_ms_box_->setSuffix(QString(" %1").arg("ms"));
	delay_ms_box_->setMinimum(0);
	form_layout_->addRow(tr("Delay"), delay_ms_box_);
	main_layout->addLayout(form_layout_);

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
	connect(config_key_form_, SIGNAL(current_config_key_changed()),
		this, SLOT(on_config_key_changed()));
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


shared_ptr<sv::devices::properties::BaseProperty>
	StepBlockDialog::property() const
{
	return property_;
}

QVariant StepBlockDialog::start_value() const
{
	const datatypes::BaseWidget *base_widget =
		dynamic_cast<datatypes::BaseWidget *>(start_value_box_);
	return base_widget->variant_value();
}

QVariant StepBlockDialog::end_value() const
{
	const datatypes::BaseWidget *base_widget =
		dynamic_cast<datatypes::BaseWidget *>(end_value_box_);
	return base_widget->variant_value();
}

QVariant StepBlockDialog::step_size() const
{
	const datatypes::BaseWidget *base_widget =
		dynamic_cast<datatypes::BaseWidget *>(step_size_box_);
	return base_widget->variant_value();
}

uint StepBlockDialog::delay_ms() const
{
	return delay_ms_box_->value();
}

void StepBlockDialog::on_config_key_changed()
{
	property_ = config_key_form_->get_property();

	// Dummy widgets if there is no widget for this property. Otherwise the NULL
	// widget can't be replaced with another property widget.
	QWidget *new_start_value_box = new QWidget();
	QWidget *new_end_value_box = new QWidget();
	QWidget *new_step_size_box = new QWidget();
	if (property_ != nullptr) {
		new_start_value_box = datatypes::datatypehelper::get_widget_for_property(
			property_, false, false);
		new_end_value_box = datatypes::datatypehelper::get_widget_for_property(
			property_, false, false);
		new_step_size_box = datatypes::datatypehelper::get_widget_for_property(
			property_, false, false);
	}

	form_layout_->replaceWidget(start_value_box_, new_start_value_box);
	form_layout_->replaceWidget(end_value_box_, new_end_value_box);
	form_layout_->replaceWidget(step_size_box_, new_step_size_box);
	delete start_value_box_;
	delete end_value_box_;
	delete step_size_box_;
	start_value_box_ = new_start_value_box;
	end_value_box_ = new_end_value_box;
	step_size_box_ = new_step_size_box;
}

} // namespace dialogs
} // namespace processing
} // namespace ui
} // namespace sv
