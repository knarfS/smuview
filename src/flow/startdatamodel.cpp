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

#include <QtCore/QJsonValue>
#include <NodeDataModel>

#include "startdatamodel.hpp"
#include "src/flow/triggernodedata.hpp"

namespace sv {
namespace flow {

StartDataModel::StartDataModel() :
	QtNodes::NodeDataModel()
{
	setup_ui();
}


void StartDataModel::setup_ui()
{
	start_button_ = new QPushButton();
	start_button_->setText(tr("Start"));
	start_button_->setCheckable(true);
	start_button_->setChecked(false);
	connect(start_button_, SIGNAL(clicked()),
		this, SLOT(on_start_button_pressed()));
}

QJsonObject StartDataModel::save() const
{
	QJsonObject modelJson = NodeDataModel::save();

	if (trigger_)
		modelJson["trigger"] = QString(trigger_->value());

	return modelJson;
}

void StartDataModel::restore(QJsonObject const &p)
{
	QJsonValue v = p["trigger"];

	if (!v.isUndefined()) {
		QString str = v.toString();

		bool ok;
		bool b = str.toInt(&ok);
		if (ok)
			trigger_ = std::make_shared<TriggerNodeData>(b);
	}
}

unsigned int StartDataModel::nPorts(PortType port_type) const
{
	unsigned int result = 1;

	switch (port_type) {
	case PortType::In:
		result = 0;
		break;
	case PortType::Out:
		result = 1;
		break;
	default:
		break;
	}

	return result;
}

NodeDataType StartDataModel::dataType(PortType, PortIndex) const
{
  return TriggerNodeData().type();
}

std::shared_ptr<NodeData> StartDataModel::outData(PortIndex)
{
  return trigger_;
}

void StartDataModel::on_start_button_pressed()
{
	bool b = false;
	if (start_button_->isChecked()) {
		b = true;
		start_button_->setText(tr("Running..."));
	}
	else {
		start_button_->setText(tr("Start"));
	}

    trigger_ = std::make_shared<TriggerNodeData>(b);
    Q_EMIT dataUpdated(0);
}

} // namespace flow
} // namespace sv
