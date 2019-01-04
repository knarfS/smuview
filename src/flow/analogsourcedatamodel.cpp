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

#include "analogsourcedatamodel.hpp"
#include "src/flow/doublenodedata.hpp"

#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>

namespace sv {
namespace flow {

AnalogSourceDataModel::AnalogSourceDataModel() :
	line_edit_(new QLineEdit())
{
  line_edit_->setValidator(new QDoubleValidator());
  line_edit_->setMaximumSize(line_edit_->sizeHint());

  connect(line_edit_, &QLineEdit::textChanged,
          this, &AnalogSourceDataModel::onTextEdited);

  line_edit_->setText("0.0");
}


QJsonObject AnalogSourceDataModel::save() const
{
  QJsonObject modelJson = NodeDataModel::save();

  if (number_)
    modelJson["number"] = QString::number(number_->value());

  return modelJson;
}


void AnalogSourceDataModel::restore(QJsonObject const &p)
{
  QJsonValue v = p["number"];

  if (!v.isUndefined())
  {
    QString strNum = v.toString();

    bool   ok;
    double d = strNum.toDouble(&ok);
    if (ok)
    {
      number_ = std::make_shared<DoubleNodeData>(d);
      line_edit_->setText(strNum);
    }
  }
}


unsigned int AnalogSourceDataModel::nPorts(PortType portType) const
{
  unsigned int result = 1;

  switch (portType)
  {
    case PortType::In:
      result = 0;
      break;

    case PortType::Out:
      result = 1;

    default:
      break;
  }

  return result;
}


void AnalogSourceDataModel::onTextEdited(QString const &string)
{
  Q_UNUSED(string);

  bool ok = false;
  double number = line_edit_->text().toDouble(&ok);

  if (ok) {
    number_ = std::make_shared<DoubleNodeData>(number);

    Q_EMIT dataUpdated(0);
  }
  else {
    Q_EMIT dataInvalidated(0);
  }
}


NodeDataType AnalogSourceDataModel::dataType(PortType, PortIndex) const
{
  return DoubleNodeData().type();
}


std::shared_ptr<NodeData> AnalogSourceDataModel::outData(PortIndex)
{
  return number_;
}

} // namespace flow
} // namespace sv
