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
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpinBox>

#include "src/session.hpp"
#include "src/devices/deviceutil.hpp"

using std::shared_ptr;

namespace sv {

namespace devices {
class Configurable;
}

namespace widgets {
class ConfigKeyComboBox;
class ConfigurableComboBox;
}

namespace ui {
namespace processing {
namespace dialogs {

class StepBlockDialog : public QDialog
{
	Q_OBJECT

public:
	StepBlockDialog(shared_ptr<Session> session,
		shared_ptr<devices::Configurable> configurable,
		QWidget *parent = nullptr);

	shared_ptr<devices::Configurable> configurable() const;
	devices::ConfigKey config_key() const;
	double start_value() const;
	double end_value() const;
	double step_size() const;
	int delay_ms() const;

private:
	shared_ptr<Session> session_;
	shared_ptr<devices::Configurable> configurable_;

	QLineEdit *name_edit_;
	sv::widgets::ConfigurableComboBox *configurable_box_;
	sv::widgets::ConfigKeyComboBox *config_key_box_;
	QDoubleSpinBox *start_value_;
	QDoubleSpinBox *end_value_;
	QDoubleSpinBox *step_size_;
	QSpinBox *delay_ms_;
	QDialogButtonBox *button_box_;

	void setup_ui();
	void connect_signals();

public Q_SLOTS:
	void accept() override;

private Q_SLOTS:
	void configurable_changed();

};

} // namespace dialogs
} // namespace processing
} // namespace ui
} // namespace sv

#endif // UI_PROCESSING_DIALOGS_STEPBLOCKDIALOG_HPP
