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

#ifndef FLOW_TRIGGERNODEDATA_HPP
#define FLOW_TRIGGERNODEDATA_HPP

#include <QDebug>
#include <QString>

#include <nodes/NodeDataModel>

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace sv {
namespace flow {

class TriggerNodeData : public NodeData
{

public:
	TriggerNodeData() :
		value_(false)
	{}

	TriggerNodeData(bool const value) :
		value_(value)
	{}

	NodeDataType type() const override
	{
		return NodeDataType { "trigger", "Trigger" };
	}

	bool value() const
	{
		qWarning() << "TriggerNodeData.value(): value = " << value_;
		return value_;
	}

	QString to_string() const
	{
		return QString(value_);
	}

private:
	bool value_;

};

} // namespace flow
} // namespace sv

#endif // FLOW_TRIGGERNODEDATA_HPP
