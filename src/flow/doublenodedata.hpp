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

#ifndef FLOW_DOUBLENODEDATA_HPP
#define FLOW_DOUBLENODEDATA_HPP

#include <QString>

#include <nodes/NodeDataModel>

using QtNodes::NodeDataType;
using QtNodes::NodeData;

namespace sv {
namespace flow {

class DoubleNodeData : public NodeData
{

public:
	DoubleNodeData() :
		value_(0.0)
	{}

	DoubleNodeData(double const value) :
		value_(value)
	{}

	NodeDataType type() const override
	{
		return NodeDataType { "double", "Double" };
	}

	double value() const
	{
		return value_;
	}

	QString to_string() const
	{
		return QString::number(value_, 'f');
	}

private:
	double value_;

};

} // namespace flow
} // namespace sv

#endif // FLOW_DOUBLENODEDATA_HPP
