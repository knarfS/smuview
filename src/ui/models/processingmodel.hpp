/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_MODELS_PROCESSINGMODEL_HPP
#define UI_MODELS_PROCESSINGMODEL_HPP

#include <QAbstractItemModel>
#include <QWidget>

namespace sv {
namespace ui {
namespace models {

class ProcessingModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	ProcessingModel(QWidget *parent = 0);

protected:

private:

};

} // namespace models
} // namespace ui
} // namespace sv

#endif // UI_MODELS_PROCESSINGMODEL_HPP

