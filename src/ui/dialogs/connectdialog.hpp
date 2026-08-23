/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012-2013 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2017-2026 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_DIALOGS_CONNECTDIALOG_HPP
#define UI_DIALOGS_CONNECTDIALOG_HPP

#include <array>
#include <map>
#include <memory>
#include <mutex>
#include <qhashfunctions.h>
#include <string>
#include <thread>

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "src/devices/deviceutil.hpp"
#include "src/devices/hardwaredevice.hpp"

using std::array;
using std::shared_ptr;
using sv::devices::DeviceType;

namespace sigrok {
class Driver;
}

Q_DECLARE_METATYPE(shared_ptr<sigrok::Driver>)
Q_DECLARE_METATYPE(shared_ptr<sv::devices::HardwareDevice>)
Q_DECLARE_METATYPE(sv::devices::DeviceType)

namespace sv {

class DeviceManager;

namespace devices {
class HardwareDevice;
}

namespace ui {
namespace dialogs {

class ConnectDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ConnectDialog(sv::DeviceManager &device_manager,
		QWidget *parent = nullptr);
	~ConnectDialog();

	shared_ptr<sv::devices::HardwareDevice> get_selected_device() const;

private:
	// Constants for serial
	static constexpr array<uint, 5> BAUD_RATES_DEFAULT = {
		921600, 115200, 57600, 19200, 9600
	};

	// Constants for TCP
	inline static const QString TCP_HOST_DEFAULT =
		QStringLiteral("192.168.1.100");
	static constexpr uint TCP_PORT_MIN = 1;
	static constexpr uint TCP_PORT_MAX = 65535;
	static constexpr uint TCP_PORT_DEFAULT = 5555;
	struct ProtocolInfo { QString label; QString format; };
	inline static const array<ProtocolInfo, 2> TCP_PROTOCOLS = {{
		{ QStringLiteral("Raw TCP"), QStringLiteral("tcp-raw/%1/%2")},
		{ QStringLiteral("VXI"), QStringLiteral("vxi/%1/%2")}
	}};

	void setup_ui();
	void populate_filters();
	void populate_drivers(DeviceType type_filter);
	void populate_serials_start(shared_ptr<sigrok::Driver> driver);
	void populate_serials_thread_proc(shared_ptr<sigrok::Driver> driver);
	void check_available_libs();
	void unset_connection();

	sv::DeviceManager &device_manager_;

	bool gpib_avialable_;

	std::thread populate_serials_thread_;
	std::mutex populate_serials_mtx_;

	QComboBox *filters_;
	QComboBox *drivers_;
	QRadioButton *radiobtn_serial_;
	QWidget *serial_config_;
	QComboBox *serial_devices_;
	QComboBox *serial_baudrate_;
	QWidget *tcp_config_;
	QLineEdit *tcp_host_;
	QSpinBox *tcp_port_;
	QComboBox *tcp_protocol_;
	QLineEdit *gpib_libgpib_name_;
	QListWidget *device_list_;
	QDialogButtonBox *button_box_;

private Q_SLOTS:
	void filter_selected(int index);
	void driver_selected(int index);
	void serial_toggled(bool checked);
	void tcp_toggled(bool checked);
	void gpib_toggled(bool checked);
	void scan_pressed();
	void populate_serials_finish(
		const std::map<std::string, std::string> &serials);

Q_SIGNALS:
	void populate_serials_done(std::map<std::string, std::string> serials);

};

} // namespace dialogs
} // namespace ui
} // namespace sv

#endif // UI_DIALOGS_CONNECTDIALOG_HPP
