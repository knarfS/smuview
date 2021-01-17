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

#include "scopeverticalcontrolview.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/util.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/ui/datatypes/boolcheckbox.hpp"
#include "src/ui/datatypes/rationalcombobox.hpp"
#include "src/ui/datatypes/stringcombobox.hpp"
#include "src/ui/datatypes/uint64combobox.hpp"

using sv::devices::ConfigKey;

namespace sv {
namespace ui {
namespace views {

ScopeVerticalControlView::ScopeVerticalControlView(Session &session,
		shared_ptr<sv::devices::Configurable> configurable,
		QUuid uuid, QWidget *parent) :
	BaseView(session, uuid, parent),
	configurable_(configurable)
{
	id_ = "scopeverticalcontrol:" + util::format_uuid(uuid_);

	setup_ui();
}

QString ScopeVerticalControlView::title() const
{
	return tr("Vertical Control") + " " + configurable_->display_name();
}

void ScopeVerticalControlView::setup_ui()
{
	QFormLayout *layout = new QFormLayout();

	// Enable channel
	enable_check_ = new ui::datatypes::BoolCheckBox(
		configurable_->get_property(ConfigKey::Enabled), true, true);
	layout->addRow(tr("Enable"), enable_check_);

	// VDiv
	vdiv_box_ = new ui::datatypes::RationalComboBox(
		configurable_->get_property(ConfigKey::VDiv), true, true);
	layout->addRow(tr("VDiv"), vdiv_box_);

	// Coupling
	coupling_box_ = new ui::datatypes::StringComboBox(
		configurable_->get_property(ConfigKey::Coupling), true, true);
	layout->addRow(tr("Coupling"), coupling_box_);

	// Filter
	filter_check_ = new ui::datatypes::BoolCheckBox(
		configurable_->get_property(ConfigKey::Filter), true, true);
	layout->addRow(tr("Filter"), filter_check_);

	// Probe factor
	probe_factor_box_ = new ui::datatypes::UInt64ComboBox(
		configurable_->get_property(ConfigKey::ProbeFactor), true, true);
	layout->addRow(tr("Probe"), probe_factor_box_);

	this->central_widget_->setLayout(layout);
}

void ScopeVerticalControlView::save_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device) const
{
	BaseView::save_settings(settings, origin_device);
	SettingsManager::save_configurable(configurable_, settings, origin_device);
}

void ScopeVerticalControlView::restore_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	BaseView::restore_settings(settings, origin_device);
}

ScopeVerticalControlView *ScopeVerticalControlView::init_from_settings(
	Session &session, QSettings &settings, QUuid uuid,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	auto configurable = SettingsManager::restore_configurable(
		session, settings, origin_device);
	if (configurable)
		return new ScopeVerticalControlView(session, configurable, uuid);
	return nullptr;
}

} // namespace views
} // namespace ui
} // namespace sv
