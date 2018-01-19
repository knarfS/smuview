/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef VIEWS_DEMODMMCONTROLVIEW_HPP
#define VIEWS_DEMODMMCONTROLVIEW_HPP

#include <memory>
#include <set>

#include <QString>

#include "src/devices/configurable.hpp"
#include "src/views/baseview.hpp"

using std::map;
using std::set;
using std::shared_ptr;

namespace sigrok {
class Quantity;
class QuantityFlag;
}

namespace sv {

class Session;

namespace widgets {
class QuantityComboBox;
class QuantityFlagsList;
}

namespace views {

class DemoDMMControlView : public BaseView
{
	Q_OBJECT

public:
	DemoDMMControlView(const Session& session,
		shared_ptr<devices::Configurable> configurable,
		QWidget* parent = nullptr);

	QString title() const;

private:
	shared_ptr<devices::Configurable> configurable_;
	devices::Configurable::sr_mq_flags_list_t sr_mq_flags_list_;
	devices::Configurable::mq_flags_list_t mq_flags_list_;

	widgets::QuantityComboBox *quantity_box_;
	widgets::QuantityFlagsList *quantity_flags_list_;

	void setup_ui();
	void connect_signals();
	void init_values();

protected:

public Q_SLOTS:

private Q_SLOTS:
	void on_quantity_changed(int index);
	void on_quantity_flags_changed();

};

} // namespace views
} // namespace sv

#endif // VIEWS_DEMODMMCONTROLVIEW_HPP

