/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_PROCESSING_BLOCKS_SEQUENCESINBLOCK_HPP
#define UI_PROCESSING_BLOCKS_SEQUENCESINBLOCK_HPP

#include <QListWidget>
#include <QListWidgetItem>

namespace sv {
namespace ui {
namespace processing {
namespace blocks {

class SequenceSinBlock : public QListWidgetItem
{

public:
	SequenceSinBlock(QListWidget *parent = nullptr);

private:

};

} // namespace blocks
} // namespace processing
} // namespace ui
} // namespace sv

#endif // UI_PROCESSING_BLOCKS_SEQUENCESINBLOCK_HPP
