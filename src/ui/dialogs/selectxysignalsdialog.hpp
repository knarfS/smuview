/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2020 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DIALOGS_SELECTXYSIGNALSDIALOG_HPP
#define UI_DIALOGS_SELECTXYSIGNALSDIALOG_HPP

#include <memory>

#include <QDialog>
#include <QDialogButtonBox>

#include "src/session.hpp"

using std::shared_ptr;

namespace sv {

namespace data {
class BaseSignal;
}
namespace devices {
class BaseDevice;
}

namespace ui {

namespace devices {
class SelectSignalWidget;
}

namespace dialogs {

class SelectXYSignalsDialog : public QDialog
{
	Q_OBJECT

public:
	SelectXYSignalsDialog(const Session &session,
		const shared_ptr<sv::devices::BaseDevice> selected_device,
		QWidget *parent = nullptr);

	shared_ptr<sv::data::BaseSignal> x_signal();
	shared_ptr<sv::data::BaseSignal> y_signal();

private:
	void setup_ui();

	const Session &session_;
	const shared_ptr<sv::devices::BaseDevice> selected_device_;

	ui::devices::SelectSignalWidget *x_signal_widget_;
	ui::devices::SelectSignalWidget *y_signal_widget_;
	QDialogButtonBox *button_box_;

};

} // namespace dialogs
} // namespace ui
} // namespace sv

#endif // UI_DIALOGS_SELECTXYSIGNALSDIALOG_HPP
