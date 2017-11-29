/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef WIDGETS_OPTIONALVALUECONTROL_HPP
#define WIDGETS_OPTIONALVALUECONTROL_HPP

#include <memory>

#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QString>
#include <QWidget>

using std::shared_ptr;

namespace sv {

namespace devices {
class HardwareDevice;
}

namespace widgets {

class ControlButton;

class OptionalValueControl : public QGroupBox
{
    Q_OBJECT

public:
	OptionalValueControl(
		const bool is_state_getable, const bool is_state_setable,
		const bool is_value_getable, const bool is_value_setable,
		const QString title, const QString unit,
		const double min, const double max, const double steps,
		QWidget *parent = 0);

private:
	bool state_;
	double value_;
	bool is_state_enabled_;
	bool is_state_getable_;
	bool is_state_setable_;
	bool is_value_getable_;
	bool is_value_setable_;
	QString title_;
	QString unit_;
	double min_;
	double max_;
	double steps_;

	widgets::ControlButton *controlButton;
	QDoubleSpinBox *doubleSpinBox;

	void setup_ui();
	void connect_signals();

public Q_SLOTS:
	void change_state(const bool state);
	void change_value(const double value);

private Q_SLOTS:
	void on_state_changed(const bool state);
	void on_value_changed(const double value);

Q_SIGNALS:
	void state_changed(const bool state);
	void value_changed(const double value);

};

} // namespace widgets
} // namespace sv

#endif // WIDGETS_OPTIONALVALUECONTROL_HPP
