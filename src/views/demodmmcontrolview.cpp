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

#include <vector>

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "demodmmcontrolview.hpp"
#include "src/session.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/widgets/quantitycombobox.hpp"
#include "src/widgets/quantityflagslist.hpp"

using std::vector;

namespace sv {
namespace views {

DemoDMMControlView::DemoDMMControlView(const Session &session,
		shared_ptr<devices::Configurable> configurable, QWidget *parent) :
	BaseView(session, parent),
	configurable_(configurable)
{
	configurable_->list_measured_quantity(measured_quantity_list_);
	setup_ui();
	connect_signals();
	init_values();
}

QString DemoDMMControlView::title() const
{
	return configurable_->name() + " " + tr("Control");
}

void DemoDMMControlView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	QStringList quantity_list;
	QStringList quantityflags_list;
	if (configurable_->is_measured_quantity_getable()) {
		for (auto pair : measured_quantity_list_) {
			quantity_list.append(
				data::quantityutil::format_quantity(pair.first));
		}
	}

	quantity_box_ = new widgets::QuantityComboBox();
	layout->addWidget( quantity_box_);

	quantity_flags_list_ = new widgets::QuantityFlagsList();
	layout->addWidget(quantity_flags_list_);

	this->centralWidget_->setLayout(layout);
}

void DemoDMMControlView::connect_signals()
{
	connect(quantity_box_, SIGNAL(currentIndexChanged(int)),
		this, SLOT(on_quantity_changed(int)));
}

void DemoDMMControlView::init_values()
{
}

void DemoDMMControlView::on_quantity_changed(int index)
{
	(void)index;

	/*
	data::Quantity quantity = quantity_box_->selected_sr_quantity();
	vector<const sigrok::QuantityFlag *> sr_qfs =
		quantity_flags_list_->selected_sr_quantity_flags();
	configurable_->set_measured_quantity(sr_q, sr_qfs);
	*/
}

void DemoDMMControlView::on_quantity_flags_changed()
{
}

} // namespace views
} // namespace sv

