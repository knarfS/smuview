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

#include <QIcon>
#include <QPixmap>
#include <QSizePolicy>
#include <QString>
#include <QWidget>

#include "axislocklabel.hpp"
#include "src/ui/widgets/clickablelabel.hpp"

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

AxisLockLabel::AxisLockLabel(
		const int axis_id, const AxisBoundary axis_boundary,
		const QString &text, QWidget *parent) :
	ClickableLabel(text, parent),
	axis_id_(axis_id),
	axis_boundary_(axis_boundary),
	locked_(false)
{
	QIcon unlocked_icon = QIcon::fromTheme("object-unlocked",
		QIcon(":/icons/object-unlocked.png"));
	unlocked_pixmap_ = unlocked_icon.pixmap(QSize(16, 16));

	QIcon locked_icon = QIcon::fromTheme("object-locked",
		QIcon(":/icons/object-locked.png"));
	locked_pixmap_ = locked_icon.pixmap(QSize(16, 16));

	setup_ui();
}

AxisLockLabel::~AxisLockLabel()
{
}

void AxisLockLabel::setup_ui()
{
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	set_locked(locked_);
}

int AxisLockLabel::get_axis_id() const
{
	return axis_id_;
}

AxisBoundary AxisLockLabel::get_axis_boundary() const
{
	return axis_boundary_;
}

bool AxisLockLabel::is_locked() const
{
	return locked_;
}

void AxisLockLabel::set_locked(bool locked)
{
	locked_ = locked;
	if (locked_)
		this->setPixmap(locked_pixmap_);
	else
		this->setPixmap(unlocked_pixmap_);
}

void AxisLockLabel::on_axis_lock_changed(const int axis_id,
	const sv::ui::widgets::plot::AxisBoundary axis_boundary, bool locked)
{
	if (axis_id != axis_id_ || axis_boundary != axis_boundary_)
		return;

	set_locked(locked);
}

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv
