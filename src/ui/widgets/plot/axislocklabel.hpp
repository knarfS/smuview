/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_WIDGETS_PLOT_AXISLOCKLABEL_HPP
#define UI_WIDGETS_PLOT_AXISLOCKLABEL_HPP

#include <QPixmap>
#include <QString>
#include <QWidget>

#include "src/ui/widgets/clickablelabel.hpp"
#include "src/ui/widgets/plot/plot.hpp"

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

class AxisLockLabel : public ClickableLabel
{
	Q_OBJECT

public:
	explicit AxisLockLabel(const int axis_id, const AxisBoundary axis_boundary,
		const QString &text = "", QWidget *parent = nullptr);
    ~AxisLockLabel();

	int get_axis_id() const;
	AxisBoundary get_axis_boundary() const;
	bool is_locked() const;
	void set_locked(bool locked);

public Q_SLOTS:
	void on_axis_lock_changed(const int axis_id,
		const sv::ui::widgets::plot::AxisBoundary axis_boundary, bool locked);
private:
	void setup_ui();

	const int axis_id_;
	const AxisBoundary axis_boundary_;
	QPixmap unlocked_pixmap_;
	QPixmap locked_pixmap_;
	bool locked_;

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_AXISLOCKLABEL_HPP
