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

#include "analogsignalsinkpopup.hpp"
#include "src/session.hpp"
#include "src/data/analogsignal.hpp"
#include "src/devices/properties/doubleproperty.hpp"
#include "src/flow/nodes/analogsignalsinkdatamodel.hpp"
#include "src/ui/devices/selectsignalwidget.hpp"
#include "src/ui/widgets/popup.hpp"

using std::static_pointer_cast;

namespace sv {
namespace ui {
namespace widgets {
namespace flow {

AnalogSignalSinkPopup::AnalogSignalSinkPopup(const Session &session,
		QtNodes::Node &node, QWidget *parent) :
	Popup(parent),
	session_(session),
	node_(node)
{
	setup_ui();
}


void AnalogSignalSinkPopup::setup_ui()
{
	sv::flow::nodes::AnalogSignalSinkDataModel *data_model =
		static_cast<sv::flow::nodes::AnalogSignalSinkDataModel *>(
			node_.nodeDataModel());

	QVBoxLayout *main_layout = new QVBoxLayout();

	signal_widget_ = new devices::SelectSignalWidget(session_);
	if (data_model && data_model->signal()) {
		// TODO:
		/*
		signal_widget_->select_device(
			data_model->property()->configurable());
		signal_widget_->select_configurable(
			data_model->property()->configurable());
		signal_widget_->select_signal(
			data_model->property()->signal());
		*/

	}
	main_layout->addWidget(signal_widget_);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(on_accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(close()));

	this->setLayout(main_layout);
}

void AnalogSignalSinkPopup::showEvent(QShowEvent *event)
{
	widgets::Popup::showEvent(event);
}

void AnalogSignalSinkPopup::on_accept()
{
	sv::flow::nodes::AnalogSignalSinkDataModel *data_model =
		static_cast<sv::flow::nodes::AnalogSignalSinkDataModel *>(
			node_.nodeDataModel());

	shared_ptr<sv::data::AnalogSignal> signal =
		static_pointer_cast<sv::data::AnalogSignal>(
			signal_widget_->selected_signal());

	if (signal) {
		data_model->set_signal(signal);
		this->close();
	}
}

} // namespace flow
} // namespace widgets
} // namespace ui
} // namespace sv
