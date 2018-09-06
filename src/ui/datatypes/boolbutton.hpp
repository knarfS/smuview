/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DATATYPES_BOOLBUTTON_HPP
#define UI_DATATYPES_BOOLBUTTON_HPP

#include <memory>

#include <QPushButton>

#include "src/devices/deviceutil.hpp"

using std::shared_ptr;

namespace sv {

namespace devices {
class Configurable;
}

namespace ui {
namespace datatypes {

class BoolButton : public QPushButton
{
    Q_OBJECT

public:
	BoolButton(shared_ptr<devices::Configurable> configurable,
		devices::ConfigKey config_key, bool auto_commit,
		QWidget *parent = nullptr);

private:
	const QIcon on_icon_;
	const QIcon off_icon_;
	const QIcon dis_icon_;
	shared_ptr<devices::Configurable> configurable_;
	devices::ConfigKey config_key_;
	bool auto_commit_;

	void setup_ui();
	void connect_signals();

public Q_SLOTS:
	void change_state(const bool);

private Q_SLOTS:
	void on_state_changed(const bool);

Q_SIGNALS:
	void state_changed(const bool);

};

} // namespace datatypes
} // namespace ui
} // namespace sv

#endif // UI_DATATYPES_BOOLBUTTON_HPP
