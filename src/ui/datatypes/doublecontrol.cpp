/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017-2019 Frank Stettner <frank-stettner@gmx.net>
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

#include <QDebug>
#include <QVBoxLayout>

#include "doublecontrol.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/ui/datatypes/doubleslider.hpp"
#include "src/ui/datatypes/doubledisplay.hpp"
#include "src/ui/datatypes/doublespinbox.hpp"

namespace sv {
namespace ui {
namespace datatypes {

DoubleControl::DoubleControl(
		shared_ptr<sv::data::properties::BaseProperty> property,
		const bool auto_commit, const bool auto_update,
		QString title, QWidget *parent) :
	QGroupBox(parent),
	BaseWidget(property, auto_commit, auto_update),
	title_(title)
{
	setup_ui();
}

void DoubleControl::setup_ui()
{
	this->setTitle(title_);

	QSizePolicy size_policy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	size_policy.setHorizontalStretch(0);
	size_policy.setVerticalStretch(0);
	this->setSizePolicy(size_policy);

	QVBoxLayout *layout = new QVBoxLayout();

	display_ = new DoubleDisplay(property_, true);
	layout->addWidget(display_, 0, Qt::AlignCenter);

	spin_box_ = new DoubleSpinBox(property_, true, true);
	layout->addWidget(spin_box_);

	slider_ = new DoubleSlider(property_, true, true);
	layout->addWidget(slider_);

	this->setLayout(layout);
}

QVariant DoubleControl::variant_value() const
{
	return QVariant(spin_box_->value());
}

} // namespace widgets
} // namespace ui
} // namespace sv
