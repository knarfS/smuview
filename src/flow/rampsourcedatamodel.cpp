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

#include <QtCore/QDebug>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QString>

#include "rampsourcedatamodel.hpp"
#include "src/flow/doublenodedata.hpp"
#include "src/flow/triggernodedata.hpp"

using std::dynamic_pointer_cast;
using std::make_shared;
using std::shared_ptr;

namespace sv {
namespace flow {

RampSourceDataModel::RampSourceDataModel() :
	start_value_(0.0),
	end_value_(3.5),
	step_width_(0.1),
	step_delay_(250)
{
	init();

	timer_ = new QTimer();
	connect(timer_, SIGNAL(timeout()), this, SLOT(process()));
}


void RampSourceDataModel::init()
{
	if (start_value_ > end_value_ && step_width_ > 0) {
		step_width_ *= -1;
	}
	act_value_ = start_value_;
}

QJsonObject RampSourceDataModel::save() const
{
	QJsonObject modelJson = NodeDataModel::save();
	modelJson["act_value"] = QString::number(start_value_);
	modelJson["start_value"] = QString::number(start_value_);
	modelJson["end_value"] = QString::number(end_value_);
	modelJson["step_width"] = QString::number(step_width_);
	modelJson["step_trigger"] = QString::number(step_trigger_);
	modelJson["step_delay"] = QString::number(step_delay_);

	return modelJson;
}

void RampSourceDataModel::restore(QJsonObject const &p)
{
	restore_double(p["act_value"], act_value_);
	restore_double(p["start_value"], start_value_);
	restore_double(p["end_value"], end_value_);
	restore_double(p["step_width"], step_width_);
	restore_bool(p["step_trigger"], step_trigger_);
	restore_long(p["step_delay"], step_delay_);
}

void RampSourceDataModel::restore_double(QJsonValue const &v, double &d_val)
{
	if (!v.isUndefined()) {
		QString strNum = v.toString();
		bool ok;
		double d = strNum.toDouble(&ok);
		if (ok)
			d_val = d;
	}
}

void RampSourceDataModel::restore_bool(QJsonValue const &v, bool &b_val)
{
	if (!v.isUndefined()) {
		QString strBool = v.toString();
		bool ok;
		int i = strBool.toInt(&ok);
		if (ok)
			b_val = (bool)i;
	}
}

void RampSourceDataModel::restore_long(QJsonValue const &v, long &l_val)
{
	if (!v.isUndefined()) {
		QString strNum = v.toString();
		bool ok;
		long l = strNum.toLong(&ok);
		if (ok)
			l_val = l;
	}
}

QString RampSourceDataModel::portCaption(
	PortType port_type, PortIndex port_index) const
{
	QString caption;

	switch (port_type) {
	case PortType::In:
		switch (port_index) {
		case 0:
			caption = tr("Start trigger");
			break;
		case 1:
			caption = tr("Step trigger");
			break;
		default:
			break;
		}
		break;
	case PortType::Out:
		switch (port_index) {
		case 0:
			caption = tr("Ramp Data");
			break;
		case 1:
			caption = tr("Step finished");
			break;
		case 2:
			caption = tr("Period finished");
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return caption;
}

unsigned int RampSourceDataModel::nPorts(PortType port_type) const
{
	unsigned int result = 1;

	switch (port_type) {
	case PortType::In:
		result = 2;
		break;
	case PortType::Out:
		result = 3;
		break;
	default:
		break;
	}

	return result;
}

NodeDataType RampSourceDataModel::dataType(
	PortType port_type, PortIndex port_index) const
{
	NodeDataType type;

	switch (port_type) {
	case PortType::In:
		type = TriggerNodeData().type();
		break;
	case PortType::Out:
		switch (port_index) {
		case 0:
			type = DoubleNodeData().type();
			break;
		case 1:
		case 2:
			type = TriggerNodeData().type();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return type;
}

shared_ptr<NodeData> RampSourceDataModel::outData(PortIndex port_index)
{
	switch (port_index) {
	case 0:
		return ramp_value_;
	case 1:
		return trigger_step_finished_;
	case 2:
		return trigger_period_finished_;
	default:
		break;
	}
	return nullptr;
}

void RampSourceDataModel::setInData(
	shared_ptr<NodeData> data, PortIndex port_index)
{
	auto trigger_data = dynamic_pointer_cast<TriggerNodeData>(data);

	if (port_index == 0) {
		// Start Trigger

		init();

		if (trigger_data && trigger_data->value()) {
			if (step_trigger_) {
				qWarning() << "RampSourceDataModel::setInData(): StartTrigger -> process";
				process();
			}
			else {
				timer_->start(step_delay_);
			}
		}
		/*
		 * TODO: how to implement stop?
		else {
			if (!step_trigger_) {
				timer_->stop();
				act_value_ = start_value_;
			}
		}
		*/
	}
	else if (port_index == 1) {
		// Step Trigger

		if (trigger_data && trigger_data->value() && step_trigger_) {
			qWarning() << "RampSourceDataModel::setInData(): StepTrigger -> process";
			process();
		}
	}
}

void RampSourceDataModel::process()
{
	bool stop = false;
	if (start_value_ > end_value_) {
		if (act_value_ < end_value_)
			stop = true;
	}
	else {
		if (act_value_ > end_value_)
			stop = true;
	}

	if (!stop) {
		// TODO: new object or change value in existing object?
		ramp_value_ = make_shared<DoubleNodeData>(act_value_);
		Q_EMIT dataUpdated(0);

		// TODO: utility
		trigger_step_finished_ = make_shared<TriggerNodeData>(true);
		Q_EMIT dataUpdated(1);
		trigger_step_finished_ = make_shared<TriggerNodeData>(false);
		Q_EMIT dataUpdated(1);

		act_value_ = act_value_ + step_width_;
	}
	else {
		timer_->stop();

		// TODO: utility
		trigger_period_finished_ = make_shared<TriggerNodeData>(true);
		Q_EMIT dataUpdated(2);
		trigger_period_finished_ = make_shared<TriggerNodeData>(false);
		Q_EMIT dataUpdated(2);
	}
}

} // namespace flow
} // namespace sv
