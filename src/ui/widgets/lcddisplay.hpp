/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_WIDGETS_LCDDISPLAY_HPP
#define UI_WIDGETS_LCDDISPLAY_HPP

#include <QLabel>
#include <QLCDNumber>
#include <QString>

#include "src/ui/widgets/valuedisplay.hpp"

namespace sv {
namespace ui {
namespace widgets {

class LcdDisplay : public ValueDisplay
{
	Q_OBJECT

public:
	LcdDisplay(
		int digits, int decimal_places, const bool auto_range,
		const QString &unit, const QString &unit_suffix,
		const QString &extra_text, const bool small, QWidget *parent = nullptr);

private:
	int height_;
	uint width_scale_factor_;

	QLCDNumber *value_lcd_;
	QLabel *extra_label_;
	QLabel *unit_label_;

	void setup_ui() override;
	void update_value_widget_dimensions() override;
	void update_extra_widget_dimensions() override;
	void update_unit_widget_dimensions() override;
	void show_value(const QString &value) override;
	void show_extra_text(const QString &extra_text) override;
	void show_unit(const QString &unit) override;

};

} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_LCDDISPLAY_HPP
