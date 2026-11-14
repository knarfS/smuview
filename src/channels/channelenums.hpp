/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2026 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CHANNELS_CHANNELENUMS_HPP
#define CHANNELS_CHANNELENUMS_HPP

namespace sv {
namespace channels {

enum class ChannelType
{
	/**
	 * Channels with analog data (Power supplies, loads, DMMs)
	 */
	AnalogChannel,
	/**
	 * Virtual channel for calculated data
	 */
	MathChannel,
	/**
	 * Virtual channel for user generated data (e.g. scripts)
	 */
	UserChannel
};

} // namespace channels
} // namespace sv

#endif // CHANNELS_CHANNELENUMS_HPP
