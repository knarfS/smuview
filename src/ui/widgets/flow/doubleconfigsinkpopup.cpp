/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>

#include <QDebug>
#include <QDialogButtonBox>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QWidget>
#include <Node>

#include "doubleconfigsinkpopup.hpp"
#include "src/session.hpp"
#include "src/devices/properties/doubleproperty.hpp"
#include "src/flow/doubleconfigsinkdatamodel.hpp"
#include "src/ui/devices/selectpropertyform.hpp"
#include "src/ui/widgets/popup.hpp"

using std::static_pointer_cast;

namespace sv {
namespace ui {
namespace widgets {
namespace flow {

DoubleConfigSinkPopup::DoubleConfigSinkPopup(const Session &session,
		QtNodes::Node &node, QWidget *parent) :
	Popup(parent),
	session_(session),
	node_(node)
{
	setup_ui();
}


void DoubleConfigSinkPopup::setup_ui()
{
	sv::flow::DoubleConfigSinkDataModel *data_model =
		static_cast<sv::flow::DoubleConfigSinkDataModel *>(
			node_.nodeDataModel());

	QVBoxLayout *main_layout = new QVBoxLayout();

	property_form_ = new devices::SelectPropertyForm(session_);
	if (data_model && data_model->property()) {
		// TODO: select device
		property_form_->select_configurable(
			data_model->property()->configurable());
		property_form_->select_config_key(
			data_model->property()->config_key());
	}
	main_layout->addLayout(property_form_);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(on_accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(close()));

	this->setLayout(main_layout);
}

void DoubleConfigSinkPopup::showEvent(QShowEvent *event)
{
	widgets::Popup::showEvent(event);
}

void DoubleConfigSinkPopup::on_accept()
{
	sv::flow::DoubleConfigSinkDataModel *data_model =
		static_cast<sv::flow::DoubleConfigSinkDataModel *>(
			node_.nodeDataModel());

	shared_ptr<sv::devices::properties::DoubleProperty> property =
		static_pointer_cast<sv::devices::properties::DoubleProperty>(
			property_form_->selected_property());
	if (property) {
		data_model->set_property(property);
		this->close();
	}
}

} // namespace flow
} // namespace widgets
} // namespace ui
} // namespace sv
