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

#ifndef FLOW_ANALOGSOURCEDATAMODEL_HPP
#define FLOW_ANALOGSOURCEDATAMODEL_HPP

#include <QtCore/QJsonObject>
#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>

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

class DoubleNodeData;

/// The model dictates the number of inputs and outputs for the Node.
/// In this example it has no logic.
class AnalogSourceDataModel
  : public NodeDataModel
{
  Q_OBJECT

public:
  AnalogSourceDataModel();
  virtual ~AnalogSourceDataModel() {}

public:
  QString caption() const override { return QStringLiteral("Number Source"); }
  bool captionVisible() const override { return false; }
  QString name() const override { return QStringLiteral("NumberSource"); }

public:
  QJsonObject save() const override;
  void restore(QJsonObject const &p) override;

public:
  unsigned int nPorts(PortType portType) const override;
  NodeDataType dataType(PortType portType, PortIndex portIndex) const override;
  shared_ptr<NodeData> outData(PortIndex port) override;
  void setInData(shared_ptr<NodeData>, int) override { }
  QWidget *embeddedWidget() override { return line_edit_; }

private Q_SLOTS:
  void onTextEdited(QString const &string);

private:
  shared_ptr<DoubleNodeData> number_;
  QLineEdit *line_edit_;

};


} // namespace flow
} // namespace sv

#endif // DEVICES_CONFIGURABLE_HPP
