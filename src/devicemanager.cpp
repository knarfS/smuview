/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2013 Joel Holdsworth <joel@airwebreathe.org.uk>
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

#include <cassert>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/filesystem.hpp>
#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QApplication>
#include <QObject>
#include <QProgressDialog>

#include "devicemanager.hpp"
#include "src/util.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/devices/measurementdevice.hpp"
#include "src/devices/sourcesinkdevice.hpp"

using std::bind;
using std::list;
using std::map;
using std::multimap;
using std::pair;
using std::placeholders::_1;
using std::placeholders::_2;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

using Glib::VariantBase;

namespace sv {

DeviceManager::DeviceManager(shared_ptr<sigrok::Context> context,
	vector<string> drivers) :
	context_(context)
{
	unique_ptr<QProgressDialog> progress(new QProgressDialog("",
		QObject::tr("Cancel"), 0, context->drivers().size() + 1));
	progress->setWindowModality(Qt::WindowModal);
	progress->setMinimumDuration(1);  // To show the dialog immediately

	int entry_num = 1;

	/*
	 * Check the presence of optional user specs for device scans.
	 * Determine the driver names and options (in generic format) when
	 * applicable.
	 */
	multimap<string, vector<string> > user_drvs_name_opts;
	if (!drivers.empty()) {
		for (string driver : drivers) {
			vector<string> user_drv_opts = sv::util::split_string(driver, ":");
			string user_drv_name = user_drv_opts.front();
			user_drv_opts.erase(user_drv_opts.begin());

			user_drvs_name_opts.insert(
				pair< string, vector<string> >(user_drv_name, user_drv_opts));
		}
	}

	/*
	 * Scan for devices. No specific options apply here, this is
	 * best effort auto detection.
	 */
	for (auto entry : context->drivers()) {
		progress->setLabelText(QObject::tr("Scanning for %1...")
			.arg(QString::fromStdString(entry.first)));

		if (user_drvs_name_opts.count(entry.first) > 0)
			continue;
		driver_scan(entry.second, map<const sigrok::ConfigKey *, VariantBase>());

		progress->setValue(entry_num++);
		QApplication::processEvents();
		if (progress->wasCanceled())
			break;
	}

	/*
	 * Optionally run another scan with potentially more specific
	 * options when requested by the user. This is motivated by
	 * several different uses: It can find devices that are not
	 * covered by the above auto detection (UART, TCP). It can
	 * prefer one out of multiple found devices, and have this
	 * device pre-selected for new sessions upon user's request.
	 */
	user_spec_devices_.clear();
 	if (!drivers.empty() && !user_drvs_name_opts.empty()) {
		shared_ptr<sigrok::Driver> scan_drv;
		map<const sigrok::ConfigKey *, VariantBase> scan_opts;

		/*
		 * Lookup the device driver names.
		 */
		map<string, shared_ptr<sigrok::Driver>> drivers = context->drivers();
		for( auto it = user_drvs_name_opts.begin(), end = user_drvs_name_opts.end();
			it != end;
 			it = user_drvs_name_opts.upper_bound(it->first)) {

			auto user_drv_name = it->first;
			auto user_drv_opts = it->second;

			auto entry = drivers.find(user_drv_name);
			scan_drv = (entry != drivers.end()) ? entry->second : nullptr;

			/*
			* Convert generic string representation of options
			* to the driver specific data types.
			*/
			if (scan_drv && !user_drv_opts.empty()) {
				auto drv_opts = scan_drv->scan_options();
				scan_opts = driver_scan_options(user_drv_opts, drv_opts);
			}

			/*
			* Run another scan for the specified driver, passing
			* user provided scan options this time.
			*/
			list< shared_ptr<devices::HardwareDevice> > found;
			if (scan_drv) {
				found = driver_scan(scan_drv, scan_opts);
				if (!found.empty())
					user_spec_devices_.push_back(found.front());
			}
		}
	}
	progress->setValue(entry_num++);
}

const shared_ptr<sigrok::Context>& DeviceManager::context() const
{
	return context_;
}

shared_ptr<sigrok::Context> DeviceManager::context()
{
	return context_;
}

const list< shared_ptr<devices::HardwareDevice> >&
DeviceManager::devices() const
{
	return devices_;
}

/**
 * Get the device that was detected with user provided scan options.
 */
list< shared_ptr<devices::HardwareDevice> >
DeviceManager::user_spec_devices() const
{
	return user_spec_devices_;
}

/**
 * Convert generic options to data types that are specific to Driver::scan().
 *
 * @param[in] user_spec Vector of tokenized words, string format.
 * @param[in] driver_opts Driver's scan options, result of Driver::scan_options().
 *
 * @return Map of options suitable for Driver::scan().
 */
map<const sigrok::ConfigKey *, Glib::VariantBase>
DeviceManager::driver_scan_options(vector<string> user_spec,
	set<const sigrok::ConfigKey *> driver_opts)
{
	map<const sigrok::ConfigKey *, Glib::VariantBase> result;

	for (auto entry : user_spec) {
		/*
		 * Split key=value specs. Accept entries without separator
		 * (for simplified boolean specifications).
		 */
		string key, val;
		size_t pos = entry.find("=");
		if (pos == std::string::npos) {
			key = entry;
			val = "";
		} else {
			key = entry.substr(0, pos);
			val = entry.substr(pos + 1);
		}

		/*
		 * Skip user specifications that are not a member of the
		 * driver's set of supported options. Have the text format
		 * input spec converted to the required driver specific type.
		 */
		const sigrok::ConfigKey *cfg;
		try {
			cfg = sigrok::ConfigKey::get_by_identifier(key);
			if (!cfg)
				continue;
			if (driver_opts.find(cfg) == driver_opts.end())
				continue;
		} catch (...) {
			continue;
		}
		result[cfg] = cfg->parse_string(val);
	}

	return result;
}

list< shared_ptr<devices::HardwareDevice> >
DeviceManager::driver_scan(
	shared_ptr<sigrok::Driver> sr_driver,
	map<const sigrok::ConfigKey *, VariantBase> drvopts)
{
	list< shared_ptr<devices::HardwareDevice> > driver_devices;

	assert(sr_driver);

	const auto keys = sr_driver->config_keys();
	bool supported_device = keys.count(sigrok::ConfigKey::POWER_SUPPLY)
		| keys.count(sigrok::ConfigKey::ELECTRONIC_LOAD)
		//| keys.count(sigrok::ConfigKey::MULTIMETER) // TODO
		//| keys.count(sigrok::ConfigKey::DEMO_DEV) // TODO
		;
	if (!supported_device)
		return driver_devices;

	// Remove any device instances from this driver from the device
	// list. They will not be valid after the scan.
	devices_.remove_if([&](shared_ptr<devices::HardwareDevice> device) {
		return device->sr_hardware_device()->driver() == sr_driver; });

	// Do the scan
	auto sr_devices = sr_driver->scan(drvopts);

	// Add the scanned devices to the main list, set display names and sort.
	for (shared_ptr<sigrok::HardwareDevice> sr_device : sr_devices) {
		if (keys.count(sigrok::ConfigKey::POWER_SUPPLY) |
				keys.count(sigrok::ConfigKey::ELECTRONIC_LOAD)) {
			const shared_ptr<devices::SourceSinkDevice> d(
				new devices::SourceSinkDevice(context_, sr_device));
			driver_devices.push_back(d);
		}
		else if (keys.count(sigrok::ConfigKey::MULTIMETER)) {
			const shared_ptr<devices::MeasurementDevice> d(
				new devices::MeasurementDevice(context_, sr_device));
			driver_devices.push_back(d);
		}
	}

	devices_.insert(devices_.end(), driver_devices.begin(),
		driver_devices.end());
	devices_.sort(bind(&DeviceManager::compare_devices, this, _1, _2));
	driver_devices.sort(bind(&DeviceManager::compare_devices, this, _1, _2));

	return driver_devices;
}

const map<string, string> DeviceManager::get_device_info(
	shared_ptr<devices::Device> device)
{
	map<string, string> result;

	assert(device);

	const shared_ptr<sigrok::Device> sr_dev = device->sr_device();
	if (sr_dev->vendor().length() > 0)
		result["vendor"] = sr_dev->vendor();
	if (sr_dev->model().length() > 0)
		result["model"] = sr_dev->model();
	if (sr_dev->version().length() > 0)
		result["version"] = sr_dev->version();
	if (sr_dev->serial_number().length() > 0)
		result["serial_num"] = sr_dev->serial_number();
	if (sr_dev->connection_id().length() > 0)
		result["connection_id"] = sr_dev->connection_id();

	return result;
}

const shared_ptr<devices::HardwareDevice> DeviceManager::find_device_from_info(
	const map<string, string> search_info)
{
	shared_ptr<devices::HardwareDevice> last_resort_dev;
	map<string, string> dev_info;

	for (shared_ptr<devices::HardwareDevice> dev : devices_) {
		assert(dev);
		dev_info = get_device_info(dev);

		// If present, vendor and model always have to match.
		if (dev_info.count("vendor") > 0 && search_info.count("vendor") > 0)
			if (dev_info.at("vendor") != search_info.at("vendor"))
				continue;

		if (dev_info.count("model") > 0 && search_info.count("model") > 0)
			if (dev_info.at("model") != search_info.at("model"))
				continue;

		// Most unique match: vendor/model/serial_num (but don't match a S/N of 0)
		if ((dev_info.count("serial_num") > 0) && (dev_info.at("serial_num") != "0")
				&& search_info.count("serial_num") > 0)
			if (dev_info.at("serial_num") == search_info.at("serial_num") &&
					dev_info.at("serial_num") != "0")
				return dev;

		// Second best match: vendor/model/connection_id
		if (dev_info.count("connection_id") > 0 &&
			search_info.count("connection_id") > 0)
			if (dev_info.at("connection_id") == search_info.at("connection_id"))
				return dev;

		// Last resort: vendor/model/version
		if (dev_info.count("version") > 0 &&
			search_info.count("version") > 0)
			if (dev_info.at("version") == search_info.at("version") &&
					dev_info.at("version") != "0")
				return dev;

		// For this device, we merely have a vendor/model match.
		last_resort_dev = dev;
	}

	// If there wasn't even a vendor/model/version match, we end up here.
	// This is usually the case for devices with only vendor/model data.
	// The selected device may be wrong with multiple such devices attached
	// but it is the best we can do at this point. After all, there may be
	// only one such device and we do want to select it in this case.
	return last_resort_dev;
}

bool DeviceManager::compare_devices(shared_ptr<devices::Device> a,
	shared_ptr<devices::Device> b)
{
	assert(a);
	assert(b);
	return a->display_name(*this).compare(b->display_name(*this)) < 0;
}

} // namespace sv
