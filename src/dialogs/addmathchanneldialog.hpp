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

#ifndef DIALOGS_ADDMATHCHANNELDIALOG_HPP
#define DIALOGS_ADDMATHCHANNELDIALOG_HPP

#include <memory>

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QTabWidget>

#include "src/session.hpp"

using std::shared_ptr;
using std::vector;

namespace sv {

namespace devices {
class HardwareDevice;
}

namespace channels {
class BaseChannel;
}

namespace widgets {
class QuantityComboBox;
class QuantityFlagsList;
class SignalTree;
class UnitComboBox;
}

namespace dialogs {

class AddMathChannelDialog : public QDialog
{
	Q_OBJECT

public:
	AddMathChannelDialog(const Session &session,
		const shared_ptr<devices::HardwareDevice> device,
		QWidget *parent = nullptr);

	vector<shared_ptr<channels::BaseChannel>> channels();

private:
	void setup_ui();
	void setup_ui_multiply_tab();
	void setup_ui_divide_tab();
	void setup_ui_integrate_tab();

	const Session &session_;
	const shared_ptr<devices::HardwareDevice> device_;
	vector<shared_ptr<channels::BaseChannel>> channels_;

	QTabWidget *tab_widget_;
	QLineEdit *name_edit_;
	widgets::QuantityComboBox *quantity_box_;
	widgets::QuantityFlagsList *quantity_flags_list_;
	widgets::UnitComboBox *unit_box_;
	widgets::SignalTree *multiply_signal_1_tree_;
	widgets::SignalTree *multiply_signal_2_tree_;
	QDialogButtonBox *button_box_;

public Q_SLOTS:
	void accept() override;

};

} // namespace dialogs
} // namespace sv

#endif // DIALOGS_ADDMATHCHANNELDIALOG_HPP
