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

#include <QFormLayout>

#include "scopehorizontalcontrolview.hpp"
#include "src/session.hpp"
#include "src/devices/configurable.hpp"
#include "src/ui/datatypes/rationalcombobox.hpp"
#include "src/ui/datatypes/uint64label.hpp"

using sv::devices::ConfigKey;

namespace sv {
namespace ui {
namespace views {

ScopeHorizontalControlView::ScopeHorizontalControlView(Session &session,
		shared_ptr<sv::devices::Configurable> configurable, QWidget *parent) :
	BaseView(session, parent),
	configurable_(configurable)
{
	setup_ui();
}

QString ScopeHorizontalControlView::title() const
{
	return tr("Horizontal Control") + " " + configurable_->display_name();
}

void ScopeHorizontalControlView::setup_ui()
{
	QFormLayout *layout = new QFormLayout();

	// Samplerate
	samplerate_label_ = new ui::datatypes::UInt64Label(
		configurable_->get_property(ConfigKey::Samplerate), true);
	layout->addRow(tr("Samplerate"), samplerate_label_);

	// Timebase
	timebase_box_ = new ui::datatypes::RationalComboBox(
		configurable_->get_property(ConfigKey::TimeBase), true, true);
	layout->addRow(tr("Timebase"), timebase_box_);

	this->central_widget_->setLayout(layout);
}

} // namespace views
} // namespace ui
} // namespace sv
