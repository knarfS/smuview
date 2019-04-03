/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DATATYPES_DOUBLEDISPLAY_HPP
#define UI_DATATYPES_DOUBLEDISPLAY_HPP

#include <memory>

#include <QVariant>

#include "src/ui/datatypes/basewidget.hpp"
#include "src/ui/widgets/monofontdisplay.hpp"

using std::shared_ptr;

namespace sv {

namespace devices {
namespace properties {
class BaseProperty;
}
}

namespace ui {
namespace datatypes {

class DoubleDisplay : public widgets::MonoFontDisplay, public BaseWidget
{
	Q_OBJECT

public:
	DoubleDisplay(
		shared_ptr<sv::devices::properties::BaseProperty> property,
		const bool auto_update, QWidget *parent = nullptr);

	QVariant variant_value() const;

private:
	void setup_ui();
	void connect_signals();

private Q_SLOTS:
	/**
	 * Signal handling for Widget -> Property. Nothing to do here.
	 */
	void value_changed(const double);
	/**
	 * Signal handling for Property -> Widget
	 */
	void on_value_changed(const QVariant);

};

} // namespace datatypes
} // namespace ui
} // namespece sv

#endif // UI_DATATYPES_DOUBLEDISPLAY_HPP
