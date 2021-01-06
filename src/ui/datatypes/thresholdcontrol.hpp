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

#ifndef UI_DATATYPES_THRESHOLDCONTROL_HPP
#define UI_DATATYPES_THRESHOLDCONTROL_HPP

#include <memory>

#include <QGroupBox>

#include "src/ui/datatypes/basewidget.hpp"

using std::shared_ptr;

namespace sv {

namespace data {
namespace properties {
class BaseProperty;
}
}

namespace ui {
namespace datatypes {

class BoolButton;
class DoubleSpinBox;

class ThresholdControl : public QGroupBox, public BaseWidget
{
	Q_OBJECT

public:
	ThresholdControl(
		shared_ptr<sv::data::properties::BaseProperty> property,
		shared_ptr<sv::data::properties::BaseProperty> bool_prop,
		const bool auto_commit, const bool auto_update,
		const QString &title, QWidget *parent = nullptr);

	QVariant variant_value() const override;

private:
	shared_ptr<sv::data::properties::BaseProperty> bool_prop_;
	QString title_;

	BoolButton *button_;
	DoubleSpinBox *spin_box_;

	void setup_ui();

private Q_SLOTS:
	/** Signal handling for Property -> Widget. Nothing to do here. */
	void on_value_changed(const QVariant &qvar);
	/** Signal handling for Property -> Widget. Nothing to do here. */
	void on_list_changed();

};

} // namespace datatypes
} // namespace ui
} // namespace sv

#endif // UI_DATATYPES_THRESHOLDCONTROL_HPP
