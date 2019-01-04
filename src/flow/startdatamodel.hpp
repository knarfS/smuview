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

#ifndef FLOW_STARTDATAMODEL_HPP
#define FLOW_STARTDATAMODEL_HPP

#include <QtCore/QJsonObject>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

#include <nodes/NodeDataModel>

#include <iostream>

using std::shared_ptr;
using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace sv {
namespace flow {

class TriggerNodeData;

class StartDataModel
	: public NodeDataModel
{
	Q_OBJECT

public:
	StartDataModel();
	virtual ~StartDataModel() {}

	QString caption() const override { return QStringLiteral("Start"); }
	bool captionVisible() const override { return true; }
	QString name() const override { return QStringLiteral("Start"); }

	QJsonObject save() const override;
	void restore(QJsonObject const &p) override;

	unsigned int nPorts(PortType portType) const override;
	NodeDataType dataType(PortType portType, PortIndex portIndex) const override;
	shared_ptr<NodeData> outData(PortIndex port) override;
	void setInData(shared_ptr<NodeData>, int) override { }
	QWidget *embeddedWidget() override { return start_button_; }

private:
	void setup_ui();

	// TODO: handel with signal from view tool bar
	QPushButton *start_button_;
	shared_ptr<TriggerNodeData> trigger_;

private Q_SLOTS:
	void on_start_button_pressed();

};

} // namespace flow
} // namespace sv

#endif // FLOW_STARTDATAMODEL_HPP
