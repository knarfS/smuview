/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2026 Frank Stettner <frank-stettner@gmx.net>
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

#include <boost/version.hpp>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QGuiApplication>
#include <QString>
#include <qwt_global.h>

#include "src/config.h"
#include "src/devicemanager.hpp"

using std::shared_ptr;

namespace sv {
namespace utils {
namespace apputil {

QString get_versions_md()
{
	char *sr_host = sr_buildinfo_host_get();
	QString host = QString(sr_host);
	g_free(sr_host);

	char *sr_scpi_backends = sr_buildinfo_scpi_backends_get();
	QString scpi_backends = QString(sr_scpi_backends);
	g_free(sr_scpi_backends);

	QString versions = QStringLiteral(
		"### SmuView information\n\n"
		"- **SmuView:** %1\n"
		"- **OS:** %2 (%3 / %4)\n\n"
		"### SmuView libraries\n\n"
		"- **Qt:** %5 (%6)\n"
		"- **Qwt:** %7\n"
		"- **glibmm:** %8\n"
		"- **Boost:** %9\n").arg(
			SV_VERSION_STRING,
			QSysInfo::prettyProductName(),
			QSysInfo::kernelVersion(), QSysInfo::productVersion(),
			qVersion(), QGuiApplication::platformName(),
			QWT_VERSION_STR,
			SV_GLIBMM_VERSION,
			BOOST_LIB_VERSION);

	// Max. of 9 arguments for Qt < 5.14
	versions += QStringLiteral(
		"- **Python:** %1\n"
		"- **pybind11:** %2\n\n"
		"### libsigrok information\n\n"
		"- **libsigrok:** %3/%4 (rt: %5/%6)\n"
		"- **Host:** %7\n"
		"- **SCPI backends:** %8\n\n"
		"### libsigrok libraries\n\n").arg(
			SV_PYTHON_VERSION,
			SV_PYBIND11_VERSION,
			SR_PACKAGE_VERSION_STRING, SR_LIB_VERSION_STRING,
			sr_package_version_string_get(), sr_lib_version_string_get(),
			host,
			scpi_backends);

	GSList *libs_orig = sr_buildinfo_libs_get();
	for (GSList *lib = libs_orig; lib; lib = lib->next) {
		GSList *lib_data = static_cast<GSList *>(lib->data);
		const char *name = static_cast<const char *>(lib_data->data);
		const char *version = static_cast<const char *>(lib_data->next->data);
		versions += QStringLiteral("- **%1:** %2\n").arg(
			QString(name), QString(version));
		g_slist_free_full(lib_data, g_free);
	}
	g_slist_free(libs_orig);
	versions += "\n";

	return versions;
}

QString get_supported_drivers_md(DeviceManager &device_manager)
{
	shared_ptr<sigrok::Context> context = device_manager.context();

	QString drivers = "### libsigrok supported hardware drivers\n\n";
	for (const auto &[name, driver] : context->drivers()) {
		drivers += QStringLiteral("- **%1:** %2\n").arg(
			QString::fromUtf8(name.c_str()),
			QString::fromUtf8(driver->long_name().c_str()));
	}
	drivers += "\n";

	return drivers;
}

QString get_supported_input_formats_md(DeviceManager &device_manager)
{
	shared_ptr<sigrok::Context> context = device_manager.context();

	QString inputs = "### libsigrok supported input formats\n\n";
	for (const auto &[name, input] : context->input_formats()) {
		inputs += QStringLiteral("- **%1:** %2\n").arg(
			QString::fromUtf8(name.c_str()),
			QString::fromUtf8(input->description().c_str()));
	}
	inputs += "\n";

	return inputs;
}

QString get_supported_output_formats_md(DeviceManager &device_manager)
{
	shared_ptr<sigrok::Context> context = device_manager.context();

	QString outputs = "### libsigrok supported output formats\n\n";
	for (const auto &[name, output] : context->output_formats()) {
		outputs += QStringLiteral("- **%1:** %2\n").arg(
			QString::fromUtf8(name.c_str()),
			QString::fromUtf8(output->description().c_str()));
	}
	outputs += "\n";

	return outputs;
}

} // namespace apputil
} // namespace utils
} // namespace sv
