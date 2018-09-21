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

#ifndef UI_DIALOGS_SELECTSIGNALDIALOG_HPP
#define UI_DIALOGS_SELECTSIGNALDIALOG_HPP

#include <memory>
#include <vector>

#include <QDialog>
#include <QDialogButtonBox>

#include "src/session.hpp"

using std::shared_ptr;
using std::vector;

namespace sv {

namespace data {
class BaseSignal;
}
namespace devices {
class BaseDevice;
}

namespace ui {

namespace devices {
class SignalTree;
}

namespace dialogs {

class SelectSignalDialog : public QDialog
{
	Q_OBJECT

public:
	SelectSignalDialog(const Session &session,
		const shared_ptr<sv::devices::BaseDevice> device,
		QWidget *parent = nullptr);

	vector<shared_ptr<sv::data::BaseSignal>> signals();

private:
	void setup_ui();

	const Session &session_;
	const shared_ptr<sv::devices::BaseDevice> device_;
	vector<shared_ptr<sv::data::BaseSignal>> signals_;

	ui::devices::SignalTree *signal_tree_;
	QDialogButtonBox *button_box_;

public Q_SLOTS:
	void accept() override;

};

} // namespace dialogs
} // namespace ui
} // namespace sv

#endif // UI_DIALOGS_SELECTSIGNALDIALOG_HPP
