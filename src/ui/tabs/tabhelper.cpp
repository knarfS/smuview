/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018-2026 Frank Stettner <frank-stettner@gmx.net>
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

#include <QWidget>

#include "tabhelper.hpp"
#include "src/session.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/devices/measurementdevice.hpp"
#include "src/devices/oscilloscopedevice.hpp"
#include "src/devices/sourcesinkdevice.hpp"
#include "src/devices/userdevice.hpp"
#include "src/ui/tabs/devicetab.hpp"
#include "src/ui/tabs/measurementtab.hpp"
#include "src/ui/tabs/oscilloscopetab.hpp"
#include "src/ui/tabs/sourcesinktab.hpp"
#include "src/ui/tabs/usertab.hpp"

using std::shared_ptr;
using std::static_pointer_cast;

namespace sv {
namespace ui {
namespace tabs {
namespace tabhelper {

DeviceTab *get_tab_for_device(Session &session,
	shared_ptr<devices::BaseDevice> device, QWidget *parent)
{
	if (!device)
		return nullptr;

	// Power supplies or electronic loads
	if (devices::deviceutil::is_source_sink_device(device->type())) {
		return new SourceSinkTab(session,
			static_pointer_cast<devices::SourceSinkDevice>(device), parent);
	}

	// Oscilloscopes
	if (devices::deviceutil::is_oscilloscope_device(device->type())) {
		return new OscilloscopeTab(session,
			static_pointer_cast<devices::OscilloscopeDevice>(device), parent);
	}

	// Measurement devices like DMMs, scales, LCR meters, etc., but also
	// the demo device(s)
	if (devices::deviceutil::is_measurement_device(device->type())) {
		return new MeasurementTab(session,
			static_pointer_cast<devices::MeasurementDevice>(device), parent);
	}

	// User device tab
	if (devices::deviceutil::is_user_device(device->type())) {
		return new UserTab(session,
			static_pointer_cast<devices::UserDevice>(device), parent);
	}

	return nullptr;
}

} // namespace tabhelper
} // namespace tabs
} // namespace ui
} // namespace sv
