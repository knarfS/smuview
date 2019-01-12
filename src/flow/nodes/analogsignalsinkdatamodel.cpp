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
#include <QDateTime>
#include <QLabel>
#include <QVariant>

#include "analogsignalsinkdatamodel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/devices/properties/doubleproperty.hpp"
#include "src/flow/doublenodedata.hpp"

using std::dynamic_pointer_cast;
using std::make_shared;
using std::shared_ptr;

namespace sv {
namespace flow {
namespace nodes {

AnalogSignalSinkDataModel::AnalogSignalSinkDataModel() :
	NodeDataModel()
{
	setup_ui();
}


void AnalogSignalSinkDataModel::setup_ui()
{
	label_ = new QLabel();
}

QJsonObject AnalogSignalSinkDataModel::save() const
{
	QJsonObject modelJson = NodeDataModel::save();
	if (value_)
		modelJson["signal"] = signal_->name();

	return modelJson;
}

void AnalogSignalSinkDataModel::restore(QJsonObject const &p)
{
	QJsonValue v = p["signal"];

	if (!v.isUndefined())
	{
		// TODO: How to save a signal (also: property/configKey/device/configurable/...)
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

unsigned int AnalogSignalSinkDataModel::nPorts(PortType portType) const
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

NodeDataType AnalogSignalSinkDataModel::dataType(PortType, PortIndex) const
{
	return DoubleNodeData().type();
}

shared_ptr<NodeData> AnalogSignalSinkDataModel::outData(PortIndex)
{
	shared_ptr<NodeData> ptr;
	return ptr;
}

void AnalogSignalSinkDataModel::setInData(shared_ptr<NodeData> data, PortIndex)
{
	if (!signal_)
		return;

	auto double_data = dynamic_pointer_cast<DoubleNodeData>(data);
	if (double_data) {
		label_->setText(double_data->to_string());
		double sample = double_data->value();
		// TODO: use std::chrono / std::time
		double timestamp = QDateTime::currentMSecsSinceEpoch() / (double)1000;
		// TODO: digits, decimals, new push_sample() with double property
		signal_->push_sample(&sample, timestamp, sizeof(double), 7, 7);
	}
	else {
		label_->clear();
	}

	label_->adjustSize();
}

} // namespace nodes
} // namespace flow
} // namespace sv
