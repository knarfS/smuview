/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef TABS_DEVICETAB_HPP
#define TABS_DEVICETAB_HPP

#include <memory>
#include <unordered_set>
#include <vector>

#include <QMainWindow>

#include "src/util.hpp"
#include "src/data/signalbase.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/tabs/basetab.hpp"

using std::shared_ptr;
using std::unordered_set;

namespace sv {

class Session;

namespace tabs {

class DeviceTab : public BaseTab
{
	Q_OBJECT

private:

public:
	DeviceTab(Session &session,
		shared_ptr<devices::HardwareDevice> device, QMainWindow *parent);

	virtual void clear_signals();

	/**
	 * Returns the signal bases contained in this tab.
	 */
	unordered_set< shared_ptr<data::SignalBase> > signalbases() const;

	virtual void clear_signalbases();

	virtual void add_signalbase(const shared_ptr<data::SignalBase> signalbase);

protected:
	shared_ptr<devices::HardwareDevice> device_;
	util::TimeUnit time_unit_;
	unordered_set< shared_ptr<data::SignalBase> > signalbases_;

public Q_SLOTS:

private Q_SLOTS:

};

} // namespace tabs
} // namespace sv

#endif // TABS_DEVICETAB_HPP
