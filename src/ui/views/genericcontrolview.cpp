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

#include <QDebug>
#include <QFormLayout>
#include <QSettings>
#include <QString>
#include <QUuid>
#include <QVariant>
#include <QWidget>

#include "genericcontrolview.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/util.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/datatypes/datatypehelper.hpp"
#include "src/ui/views/baseview.hpp"
#include "src/ui/views/viewhelper.hpp"

using std::shared_ptr;

namespace sv {
namespace ui {
namespace views {

GenericControlView::GenericControlView(Session &session,
		shared_ptr<sv::devices::Configurable> configurable,
		QUuid uuid, QWidget *parent) :
	BaseView(session, uuid, parent),
	configurable_(configurable)
{
	id_ = "genericcontrol:" + util::format_uuid(uuid_);

	setup_ui();
	connect_signals();
}

QString GenericControlView::title() const
{
	return tr("Control") + " " + configurable_->display_name();
}

void GenericControlView::setup_ui()
{
	QFormLayout *layout = new QFormLayout();

	for (const auto &prop : configurable_->property_map()) {
		QString text = devices::deviceutil::format_config_key(prop.first);
		QWidget *dt_widget = datatypes::datatypehelper::get_widget_for_property(
			prop.second, true, true);

		layout->addRow(text, dt_widget);
	}

	this->central_widget_->setLayout(layout);
}

void GenericControlView::connect_signals()
{
	// Control elements -> Device

	// Device -> control elements
}

void GenericControlView::save_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device) const
{
	BaseView::save_settings(settings, origin_device);
	SettingsManager::save_configurable(configurable_, settings, origin_device);
}

void GenericControlView::restore_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	BaseView::restore_settings(settings, origin_device);
}

GenericControlView *GenericControlView::init_from_settings(
	Session &session, QSettings &settings, QUuid uuid,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	auto configurable = SettingsManager::restore_configurable(
		session, settings, origin_device);
	if (configurable)
		return new GenericControlView(session, configurable, uuid);
	return nullptr;
}

} // namespace views
} // namespace ui
} // namespace sv
