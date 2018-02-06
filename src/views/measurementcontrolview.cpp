/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <set>
#include <utility>

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVariant>
#include <QVBoxLayout>

#include "measurementcontrolview.hpp"
#include "src/session.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/hardwaredevice.hpp"

Q_DECLARE_METATYPE(sv::data::Quantity)
Q_DECLARE_METATYPE(std::set<sv::data::QuantityFlag>)

using std::make_pair;
using std::set;

namespace sv {
namespace views {

MeasurementControlView::MeasurementControlView(const Session &session,
		shared_ptr<devices::Configurable> configurable, QWidget *parent) :
	BaseView(session, parent),
	configurable_(configurable)
{
	configurable_->list_measured_quantity(measured_quantity_list_);

	setup_ui();
	init_values(); // Must be called before connect_signals()!
	connect_signals();
}

QString MeasurementControlView::title() const
{
	return configurable_->name() + " " + tr("Control");
}

void MeasurementControlView::setup_ui()
{
	QHBoxLayout *layout = new QHBoxLayout();

	quantity_box_ = new QComboBox();
	if (configurable_->is_measured_quantity_listable()) {
		for (auto pair : measured_quantity_list_) {
			data::Quantity qunatity = pair.first;
			quantity_box_->addItem(
				data::quantityutil::format_quantity(qunatity),
				QVariant::fromValue(qunatity));
		}
	}
	layout->addWidget(quantity_box_, 0);

	quantity_flags_box_ = new QComboBox();
	layout->addWidget(quantity_flags_box_, 0);

	set_button_ = new QPushButton();
	set_button_->setText(tr("Set"));
	layout->addWidget(set_button_, 0);

	this->centralWidget_->setLayout(layout);
}

void MeasurementControlView::connect_signals()
{
	// Private
	connect(quantity_box_, SIGNAL(currentIndexChanged(const QString)),
		this, SLOT(on_quantity_changed()));

	// Control elements -> Device
	connect(set_button_, SIGNAL(clicked(bool)), this, SLOT(on_quantity_set()));

	// Device -> control elements
}

void MeasurementControlView::init_values()
{
	if (configurable_->is_measured_quantity_getable()) {
		actual_measured_quantity_ = configurable_->get_measured_quantity();
		for (int i = 0; i < quantity_box_->count(); ++i) {
			QVariant data = quantity_box_->itemData(i);
			auto item_q = data.value<data::Quantity>();
			if (item_q == actual_measured_quantity_.first) {
				quantity_box_->setCurrentIndex(i);
				on_quantity_changed();
				break;
			}
		}
		for (int i = 0; i < quantity_flags_box_->count(); ++i) {
			QVariant data = quantity_flags_box_->itemData(i);
			auto item_qfs = data.value<set<data::QuantityFlag>>();
			if (item_qfs == actual_measured_quantity_.second) {
				quantity_flags_box_->setCurrentIndex(i);
				break;
			}
		}

	}
}

void MeasurementControlView::on_quantity_changed()
{
	quantity_flags_box_->clear();

	QVariant data = quantity_box_->currentData();
	data::Quantity quantity = data.value<data::Quantity>();

	for (auto qf_set : measured_quantity_list_[quantity]) {
		quantity_flags_box_->addItem(
			data::quantityutil::format_quantity_flags(qf_set),
			QVariant::fromValue(qf_set));
	}
}

void MeasurementControlView::on_quantity_set()
{
	QVariant q_data = quantity_box_->currentData();
	data::Quantity quantity = q_data.value<data::Quantity>();
	QVariant qf_data = quantity_flags_box_->currentData();
	set<data::QuantityFlag> quantity_flags =
		qf_data.value<set<data::QuantityFlag>>();

	auto mq_pair = make_pair(quantity, quantity_flags);
	configurable_->set_measured_quantity(mq_pair);
}

} // namespace views
} // namespace sv

