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

#include <QtCore/QJsonValue>
#include <QLabel>
#include <QVariant>

#include "doubleconfigsinkdatamodel.hpp"
#include "src/flow/doublenodedata.hpp"

using std::dynamic_pointer_cast;
using std::make_shared;
using std::shared_ptr;

namespace sv {
namespace flow {

DoubleConfigSinkDataModel::DoubleConfigSinkDataModel() :
	NodeDataModel()
{
	setup_ui();
}


void DoubleConfigSinkDataModel::setup_ui()
{
	label_ = new QLabel();
}

QJsonObject DoubleConfigSinkDataModel::save() const
{
	QJsonObject modelJson = NodeDataModel::save();
	if (value_)
		modelJson["value"] = value_->to_string();

	return modelJson;
}

void DoubleConfigSinkDataModel::restore(QJsonObject const &p)
{
	QJsonValue v = p["value"];

	if (!v.isUndefined())
	{
		/*
		QString str_num = v.toString();
		bool ok;
		double d = str_num.toDouble(&ok);
		if (ok) {
			number_ = make_shared<DoubleNodeData>(d);
			//line_edit_->setText(strNum);
		}
		*/
	}
}

unsigned int DoubleConfigSinkDataModel::nPorts(PortType portType) const
{
	unsigned int result = 1;
	switch (portType) {
	case PortType::In:
		result = 1;
		break;
	case PortType::Out:
		result = 0;
		break;
	default:
		break;
	}

	return result;
}

NodeDataType DoubleConfigSinkDataModel::dataType(PortType, PortIndex) const
{
	return DoubleNodeData().type();
}

shared_ptr<NodeData> DoubleConfigSinkDataModel::outData(PortIndex)
{
	shared_ptr<NodeData> ptr;
	return ptr;
}

void DoubleConfigSinkDataModel::setInData(shared_ptr<NodeData> data, PortIndex)
{
	if (!property_)
		return;

	auto double_data = dynamic_pointer_cast<DoubleNodeData>(data);
	if (double_data) {
		label_->setText(double_data->to_string());
		property_->change_value(QVariant(double_data->value()));
	}
	else {
		label_->clear();
	}

	label_->adjustSize();
}

} // namespace flow
} // namespace sv
