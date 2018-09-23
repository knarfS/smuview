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

#ifndef UI_PROCESSING_DIALOGS_STEPBLOCKDIALOG_HPP
#define UI_PROCESSING_DIALOGS_STEPBLOCKDIALOG_HPP

#include <memory>

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QVariant>

#include "src/session.hpp"
#include "src/devices/deviceutil.hpp"

using std::shared_ptr;

namespace sv {

namespace devices {
namespace properties {
class BaseProperty;
}
}

namespace ui {

namespace devices {
class SelectConfigKeyForm;
}

namespace processing {
namespace dialogs {

class StepBlockDialog : public QDialog
{
	Q_OBJECT

public:
	StepBlockDialog(const Session &session, QWidget *parent = nullptr);

	shared_ptr<sv::devices::properties::BaseProperty> property() const;
	QVariant start_value() const;
	QVariant end_value() const;
	QVariant step_size() const;
	uint delay_ms() const;

private:
	const Session &session_;
	shared_ptr<sv::devices::properties::BaseProperty> property_;

	QFormLayout *form_layout_;
	QLineEdit *name_edit_;
	ui::devices::SelectConfigKeyForm *config_key_form_;
	QWidget *start_value_box_;
	QWidget *end_value_box_;
	QWidget *step_size_box_;
	QSpinBox *delay_ms_box_;
	QDialogButtonBox *button_box_;

	void setup_ui();
	void connect_signals();

public Q_SLOTS:
	void accept() override;

private Q_SLOTS:
	void on_config_key_changed();

};

} // namespace dialogs
} // namespace processing
} // namespace ui
} // namespace sv

#endif // UI_PROCESSING_DIALOGS_STEPBLOCKDIALOG_HPP
