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

#ifndef FLOW_DOUBLECONFIGSINKDATAMODEL_HPP
#define FLOW_DOUBLECONFIGSINKDATAMODEL_HPP

#include <iostream>

#include <QtCore/QJsonObject>
#include <QtCore/QObject>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

#include <nodes/NodeDataModel>

#include "src/devices/properties/doubleproperty.hpp"

using std::shared_ptr;
using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;
using sv::devices::properties::DoubleProperty;

namespace sv {
namespace flow {

class DoubleNodeData;

class DoubleConfigSinkDataModel
	: public NodeDataModel
{
	Q_OBJECT

public:
	DoubleConfigSinkDataModel();
	virtual ~DoubleConfigSinkDataModel() {}

	QString caption() const override { return QStringLiteral("Double ConfigKey Sink"); }
	bool captionVisible() const override { return true; }
	QString name() const override { return QStringLiteral("DoubleConfigKeySink"); }

	QJsonObject save() const override;
	void restore(QJsonObject const &p) override;

	unsigned int nPorts(PortType portType) const override;
	NodeDataType dataType(PortType portType, PortIndex portIndex) const override;
	shared_ptr<NodeData> outData(PortIndex port) override;
	void setInData(shared_ptr<NodeData>, PortIndex) override;
	QWidget *embeddedWidget() override { return label_; }

	void set_property(shared_ptr<DoubleProperty> property) { property_ = property; }
	shared_ptr<DoubleProperty> property() { return property_; }

private:
	void setup_ui();

	shared_ptr<DoubleProperty> property_;
	shared_ptr<DoubleNodeData> value_;
	QWidget *widget_;
	QLabel *label_;

};

} // namespace flow
} // namespace sv

#endif // FLOW_DOUBLECONFIGSINKDATAMODEL_HPP
