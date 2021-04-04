/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>
#include <string>

#include <QFormLayout>
#include <QSettings>
#include <QUuid>

#include "scopehorizontalcontrolview.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/util.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/ui/datatypes/boolcheckbox.hpp"
#include "src/ui/datatypes/rationalcombobox.hpp"
#include "src/ui/datatypes/uint64combobox.hpp"
#include "src/ui/datatypes/uint64label.hpp"
#include "src/ui/datatypes/uint64spinbox.hpp"

using sv::devices::ConfigKey;

namespace sv {
namespace ui {
namespace views {

ScopeHorizontalControlView::ScopeHorizontalControlView(Session &session,
		shared_ptr<sv::devices::Configurable> configurable,
		QUuid uuid, QWidget *parent) :
	BaseView(session, uuid, parent),
	configurable_(configurable)
{
	id_ = "scopehorizontalcontrol:" + util::format_uuid(uuid_);

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

	// Buffer
	buffer_size_box_ = new ui::datatypes::UInt64ComboBox(
		configurable_->get_property(ConfigKey::BufferSize), true, true);
	layout->addRow(tr("Buffer size"), buffer_size_box_);

	// Average mode
	average_check_ = new ui::datatypes::BoolCheckBox(
		configurable_->get_property(ConfigKey::Averaging), true, true);
	layout->addRow(tr("Averaging"), average_check_);

	// Average count
	auto avg_samples_prop = configurable_->get_property(ConfigKey::AvgSamples);
	if (avg_samples_prop->is_listable()) {
		average_count_box_ = new ui::datatypes::UInt64ComboBox(
			avg_samples_prop, true, true);
		layout->addRow(tr("Average count"), average_count_box_);
	}
	else {
		average_count_spin_ = new ui::datatypes::UInt64SpinBox(
			avg_samples_prop, true, true);
		layout->addRow(tr("Average count"), average_count_spin_);
	}

	this->central_widget_->setLayout(layout);
}

void ScopeHorizontalControlView::save_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device) const
{
	BaseView::save_settings(settings, origin_device);
	SettingsManager::save_configurable(configurable_, settings, origin_device);
}

void ScopeHorizontalControlView::restore_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	BaseView::restore_settings(settings, origin_device);
}

ScopeHorizontalControlView *ScopeHorizontalControlView::init_from_settings(
	Session &session, QSettings &settings, QUuid uuid,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	auto configurable = SettingsManager::restore_configurable(
		session, settings, origin_device);
	if (configurable)
		return new ScopeHorizontalControlView(session, configurable, uuid);
	return nullptr;
}

} // namespace views
} // namespace ui
} // namespace sv
