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

#include "scopetriggercontrolview.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/util.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/ui/datatypes/doublespinbox.hpp"
#include "src/ui/datatypes/stringcombobox.hpp"

using sv::devices::ConfigKey;

namespace sv {
namespace ui {
namespace views {

ScopeTriggerControlView::ScopeTriggerControlView(Session &session,
		shared_ptr<sv::devices::Configurable> configurable,
		QUuid uuid, QWidget *parent) :
	BaseView(session, uuid, parent),
	configurable_(configurable)
{
	id_ = "scopetriggercontrol:" + util::format_uuid(uuid_);

	setup_ui();
}

QString ScopeTriggerControlView::title() const
{
	return tr("Trigger Control") + " " + configurable_->display_name();
}

void ScopeTriggerControlView::setup_ui()
{
	QFormLayout *layout = new QFormLayout();

	// Source
	source_box_ = new ui::datatypes::StringComboBox(
		configurable_->get_property(ConfigKey::TriggerSource), true, true);
	layout->addRow(tr("Source"), source_box_);

	// Slope
	slope_box_ = new ui::datatypes::StringComboBox(
		configurable_->get_property(ConfigKey::TriggerSlope), true, true);
	layout->addRow(tr("Slope"), slope_box_);

	// Level
	level_spin_ = new ui::datatypes::DoubleSpinBox(
		configurable_->get_property(ConfigKey::TriggerLevel), true, true);
	layout->addRow(tr("Level"), level_spin_);

	this->central_widget_->setLayout(layout);
}

void ScopeTriggerControlView::save_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device) const
{
	BaseView::save_settings(settings, origin_device);
	SettingsManager::save_configurable(configurable_, settings, origin_device);
}

void ScopeTriggerControlView::restore_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	BaseView::restore_settings(settings, origin_device);
}

ScopeTriggerControlView *ScopeTriggerControlView::init_from_settings(
	Session &session, QSettings &settings, QUuid uuid,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	auto configurable = SettingsManager::restore_configurable(
		session, settings, origin_device);
	if (configurable)
		return new ScopeTriggerControlView(session, configurable, uuid);
	return nullptr;
}

} // namespace views
} // namespace ui
} // namespace sv
