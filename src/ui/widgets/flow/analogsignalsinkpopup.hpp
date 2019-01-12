/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_WIDGETS_FLOW_ANALOGSIGNALSINKPOPUP_HPP
#define UI_WIDGETS_FLOW_ANALOGSIGNALSINKPOPUP_HPP

#include <QDialogButtonBox>
#include <QShowEvent>
#include <QWidget>

#include <Node>

#include "src/ui/widgets/popup.hpp"

namespace sv {

class Session;

namespace ui {

namespace devices {
class SelectSignalWidget;
}

namespace widgets {
namespace flow {

class AnalogSignalSinkPopup : public widgets::Popup
{
	Q_OBJECT

public:
	AnalogSignalSinkPopup(const Session &session, QtNodes::Node &node,
		QWidget *parent);

private:
	void setup_ui();
	void showEvent(QShowEvent *event);

	const Session &session_;
	QtNodes::Node &node_;

	devices::SelectSignalWidget *signal_widget_;
	QDialogButtonBox *button_box_;

private Q_SLOTS:
	void on_accept();

};

} // namespace flow
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_FLOW_ANALOGSIGNALSINKPOPUP_HPP
