/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2019-2020 Frank Stettner <frank-stettner@gmx.net>
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

#include <string>

#include <QDebug>
#include <QFormLayout>
#include <QSettings>
#include <QString>
#include <QVariant>
#include <QWidget>

#include "genericcontrolview.hpp"
#include "src/session.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/ui/datatypes/datatypehelper.hpp"

namespace sv {
namespace ui {
namespace views {

GenericControlView::GenericControlView(Session &session,
		shared_ptr<sv::devices::Configurable> configurable, QWidget *parent) :
	BaseView(session, parent),
	configurable_(configurable)
{
	id_ = "genericcontrol:" + std::to_string(BaseView::id_counter++);

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

	for (const auto &prop : configurable_->properties()) {
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

void GenericControlView::save_settings(QSettings &settings) const
{
	qWarning() << "GenericControlView::save_settings(): settings.group = " << settings.group();

	settings.setValue("id", QVariant(QString::fromStdString(id_)));
	settings.setValue("device", QVariant(QString::fromStdString(configurable_->device_id())));
	settings.setValue("configurable", QVariant(QString::fromStdString(configurable_->name())));
}

void GenericControlView::restore_settings(QSettings &settings)
{
	(void)settings;
}

} // namespace views
} // namespace ui
} // namespace sv
