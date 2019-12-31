/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DATATYPES_UINT64LABEL_HPP
#define UI_DATATYPES_UINT64LABEL_HPP

#include <memory>

#include <QLabel>
#include <QString>
#include <QVariant>

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

class UInt64Label : public QLabel, public BaseWidget
{
	Q_OBJECT

public:
	UInt64Label(
		shared_ptr<sv::data::properties::BaseProperty> property,
		const bool auto_update, QWidget *parent = nullptr);

	QVariant variant_value() const override;

private:
	void setup_ui();
	void connect_signals();

	QString text_;

private Q_SLOTS:
	/** Signal handling for Widget -> Property. Nothing to do here. */
	void value_changed(const QString);
	/** Signal handling for Property -> Widget */
	void on_value_changed(const QVariant) override;
	/** Signal handling for Property -> Widget. Nothing to do here. */
	void on_list_changed() override;

};

} // namespace datatypes
} // namespace ui
} // namespece sv

#endif // UI_DATATYPES_UINT64LABEL_HPP
