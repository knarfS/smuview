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

#ifndef FLOW_RAMPSOURCEDATAMODEL_HPP
#define FLOW_RAMPSOURCEDATAMODEL_HPP

#include <iostream>

#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtWidgets/QWidget>

#include <nodes/NodeDataModel>

using std::shared_ptr;
using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace sv {
namespace flow {

class DoubleNodeData;
class TriggerNodeData;

class RampSourceDataModel
	: public NodeDataModel
{
	Q_OBJECT

public:
	RampSourceDataModel();
	virtual ~RampSourceDataModel() {}

	QString caption() const override { return QStringLiteral("Ramp Source"); }
	bool captionVisible() const override { return true; }
	QString name() const override { return QStringLiteral("RampSource"); }
	QString portCaption(PortType, PortIndex) const override;
	bool portCaptionVisible(PortType, PortIndex) const override { return true; }

	QJsonObject save() const override;
	void restore(QJsonObject const &p) override;
	void restore_double(QJsonValue const &v, double &d_val);
	void restore_bool(QJsonValue const &v, bool &b_val);
	void restore_long(QJsonValue const &v, long &l_val);

	unsigned int nPorts(PortType portType) const override;
	NodeDataType dataType(PortType portType, PortIndex portIndex) const override;
	shared_ptr<NodeData> outData(PortIndex port) override;
	void setInData(shared_ptr<NodeData> data, PortIndex portIndex) override;
	QWidget *embeddedWidget() override { return nullptr; }

	void init();
	void set_start_value(double start_value) { start_value_ = start_value; }
	double start_value() { return start_value_; }
	void set_end_value(double end_value) { end_value_ = end_value; }
	double end_value() { return end_value_; }
	void set_step_width(double step_width) { step_width_ = step_width; }
	double step_width() { return step_width_; }
	void set_step_trigger(bool step_trigger) { step_trigger_ = step_trigger; }
	bool step_trigger() { return step_trigger_; }
	void set_step_delay(long step_delay) { step_delay_ = step_delay; }
	double step_delay() { return step_delay_; }

private:
	QTimer *timer_;
	double act_value_;
	double start_value_;
	double end_value_;
	double step_width_;
	bool step_trigger_;
	long step_delay_;
	shared_ptr<DoubleNodeData> ramp_value_;
	shared_ptr<TriggerNodeData> trigger_step_start_;
	shared_ptr<TriggerNodeData> trigger_step_finished_;
	shared_ptr<TriggerNodeData> trigger_period_finished_;

private Q_SLOTS:
	void process();

};

} // namespace flow
} // namespace sv

#endif // FLOW_RAMPSOURCEDATAMODEL_HPP
