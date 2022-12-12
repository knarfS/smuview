/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2012-2013 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2017-2021 Frank Stettner <frank-stettner@gmx.net>
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

#include <cassert>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QDebug>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>

#include "connectdialog.hpp"
#include "src/devicemanager.hpp"
#include "src/devices/deviceutil.hpp"

using std::list;
using std::map;
using std::shared_ptr;
using std::string;

using Glib::ustring;
using Glib::Variant;
using Glib::VariantBase;

using sigrok::ConfigKey;
using sigrok::Driver;

using sv::devices::HardwareDevice;

namespace sv {
namespace ui {
namespace dialogs {

ConnectDialog::ConnectDialog(sv::DeviceManager &device_manager,
		QWidget *parent) :
	QDialog(parent),
	device_manager_(device_manager),
	layout_(this),
	form_(this),
	form_layout_(&form_),
	drivers_(&form_),
	scan_button_(tr("&Scan for devices using driver above"), this),
	device_list_(this),
	button_box_(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
		Qt::Horizontal, this)
{
	qRegisterMetaType<std::map<std::string, std::string>>("std::map<std::string, std::string>");

	setWindowTitle(tr("Connect to Device"));

	connect(&button_box_, &QDialogButtonBox::accepted,
		this, &ConnectDialog::accept);
	connect(&button_box_, &QDialogButtonBox::rejected,
		this, &ConnectDialog::reject);

	connect(this, &ConnectDialog::populate_serials_done,
		this, &ConnectDialog::populate_serials_finish);

	populate_drivers();
	connect(&drivers_, QOverload<int>::of(&QComboBox::activated),
		this, &ConnectDialog::driver_selected);

	form_.setLayout(&form_layout_);

	QVBoxLayout *vbox_drv = new QVBoxLayout;
	vbox_drv->addWidget(&drivers_);
	QGroupBox *groupbox_drv = new QGroupBox(tr("Step 1: Choose the driver"));
	groupbox_drv->setLayout(vbox_drv);
	form_layout_.addRow(groupbox_drv);

	radiobtn_usb_ = new QRadioButton(tr("&USB"), this);
	radiobtn_serial_ = new QRadioButton(tr("Serial &Port"), this);
	radiobtn_tcp_ = new QRadioButton(tr("&TCP/IP"), this);

	radiobtn_usb_->setChecked(true);

	serial_config_ = new QWidget();
	QHBoxLayout *serial_config_layout = new QHBoxLayout(serial_config_);
	serial_devices_.setEditable(true);
	serial_devices_.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	serial_config_layout->addWidget(&serial_devices_);
	serial_baudrate_.setEditable(true);
	serial_baudrate_.addItem("");
	serial_baudrate_.addItem("921600");
	serial_baudrate_.addItem("115200");
	serial_baudrate_.addItem("57600");
	serial_baudrate_.addItem("19200");
	serial_baudrate_.addItem("9600");
	serial_config_layout->addWidget(&serial_baudrate_);
	serial_config_layout->addWidget(new QLabel("baud"));
	serial_config_->setEnabled(false);

	tcp_config_ = new QWidget();
	QHBoxLayout *tcp_config_layout = new QHBoxLayout(tcp_config_);
	tcp_host_ = new QLineEdit;
	tcp_host_->setText("192.168.1.100");
	tcp_config_layout->addWidget(tcp_host_);
	tcp_config_layout->addWidget(new QLabel(":"));
	tcp_port_ = new QSpinBox;
	tcp_port_->setRange(1, 65535);
	tcp_port_->setValue(5555);
	tcp_config_layout->addWidget(tcp_port_);

	tcp_config_layout->addSpacing(30);
	tcp_config_layout->addWidget(new QLabel(tr("Protocol:")));
	tcp_protocol_ = new QComboBox();
	tcp_protocol_->addItem("Raw TCP", QVariant("tcp-raw/%1/%2"));
	tcp_protocol_->addItem("VXI", QVariant("vxi/%1/%2"));
	tcp_config_layout->addWidget(tcp_protocol_);
	tcp_config_layout->setContentsMargins(0, 0, 0, 0);
	tcp_config_->setEnabled(false);

	check_available_libs();

	QVBoxLayout *vbox_if = new QVBoxLayout;
	vbox_if->addWidget(radiobtn_usb_);
	vbox_if->addWidget(radiobtn_serial_);
	vbox_if->addWidget(serial_config_);
	vbox_if->addWidget(radiobtn_tcp_);
	vbox_if->addWidget(tcp_config_);

	QGroupBox *groupbox_if = new QGroupBox(tr("Step 2: Choose the interface"));
	groupbox_if->setLayout(vbox_if);
	form_layout_.addRow(groupbox_if);

	QVBoxLayout *vbox_scan = new QVBoxLayout;
	vbox_scan->addWidget(&scan_button_);
	QGroupBox *groupbox_scan = new QGroupBox(tr("Step 3: Scan for devices"));
	groupbox_scan->setLayout(vbox_scan);
	form_layout_.addRow(groupbox_scan);

	QVBoxLayout *vbox_select = new QVBoxLayout;
	// Let the device list occupy only the minimum space needed
	device_list_.setMaximumHeight(device_list_.minimumSizeHint().height());
	vbox_select->addWidget(&device_list_);
	QGroupBox *groupbox_select = new QGroupBox(tr("Step 4: Select the device"));
	groupbox_select->setLayout(vbox_select);
	form_layout_.addRow(groupbox_select);

	unset_connection();

	connect(radiobtn_serial_, &QRadioButton::toggled,
		this, &ConnectDialog::serial_toggled);
	connect(radiobtn_tcp_, &QRadioButton::toggled,
		this, &ConnectDialog::tcp_toggled);
	connect(&scan_button_, &QPushButton::pressed,
		this, &ConnectDialog::scan_pressed);

	if (gpib_avialable_) {
		radiobtn_gpib_ = new QRadioButton(tr("&GPIB"), this);
		/*
		 * TODO: Replace with QComboBox and prefill with available GPIB
		 * connection strings (like the serial box).
		 * Must be implemented in libsigrok.
		 */
		gpib_libgpib_name_ = new QLineEdit;
		gpib_libgpib_name_->setEnabled(false);
		vbox_if->addWidget(radiobtn_gpib_);
		vbox_if->addWidget(gpib_libgpib_name_);

		connect(radiobtn_gpib_, &QRadioButton::toggled,
			this, &ConnectDialog::gpib_toggled);
	}

	setLayout(&layout_);

	layout_.addWidget(&form_);
	layout_.addWidget(&button_box_);

	// Initially populate serials for current selected device
	driver_selected(drivers_.currentIndex());
}

ConnectDialog::~ConnectDialog() {
	/*
	 * NOTE: Wait until a potentially running populate_serials_thread_ thread
	 *       has finished, otherwise sv will crash.
	 *       Waiting for the lock/mutex isn't strictly needed (empty d'tor is
	 *       sufficient), but better safe than sorry. :)
	 */
	std::lock_guard<std::mutex> lock(populate_serials_mtx_);
}

shared_ptr<HardwareDevice> ConnectDialog::get_selected_device() const
{
	const QListWidgetItem *const item = device_list_.currentItem();
	if (!item)
		return shared_ptr<HardwareDevice>();

	return item->data(Qt::UserRole).value<shared_ptr<HardwareDevice>>();
}

void ConnectDialog::populate_drivers()
{
	for (const auto &entry : device_manager_.context()->drivers()) {
		auto name = entry.first;
		auto sr_driver = entry.second;

		if (sv::devices::deviceutil::is_supported_driver(sr_driver)) {
			drivers_.addItem(QString("%1 (%2)").arg(
				sr_driver->long_name().c_str(), name.c_str()),
				QVariant::fromValue(sr_driver));
		}
	}
}

void ConnectDialog::check_available_libs()
{
	gpib_avialable_ = false;
	QString libgpib("libgpib");

	GSList *libs_orig = sr_buildinfo_libs_get();
	for (GSList *lib = libs_orig; lib; lib = lib->next) {
		GSList *lib_data = static_cast<GSList *>(lib->data);
		QString name(static_cast<const char *>(lib_data->data));

		if (QString::compare(name, libgpib, Qt::CaseInsensitive) == 0) {
			gpib_avialable_ = true;
			g_slist_free_full(lib_data, g_free);
			break;
		}
		g_slist_free_full(lib_data, g_free);
	}
	g_slist_free(libs_orig);
}

void ConnectDialog::populate_serials_start(shared_ptr<Driver> driver)
{
	serial_devices_.clear();
	serial_devices_.addItem(tr("Loading..."));
	serial_config_->setDisabled(true);

	populate_serials_thread_ =
		std::thread(&ConnectDialog::populate_serials_thread_proc, this, driver);
	populate_serials_thread_.detach();
}

void ConnectDialog::populate_serials_finish(
	const std::map<std::string, std::string> &serials)
{
	std::lock_guard<std::mutex> lock(populate_serials_mtx_);

	serial_devices_.clear();
	for (const auto &serial : serials) {
		serial_devices_.addItem(QString("%1 (%2)").arg(
			serial.first.c_str(), serial.second.c_str()),
			QString::fromStdString(serial.first));
	}
	if (radiobtn_serial_->isChecked())
		serial_config_->setDisabled(false);
}

void ConnectDialog::populate_serials_thread_proc(shared_ptr<Driver> driver)
{
	std::unique_lock<std::mutex> lock(populate_serials_mtx_, std::try_to_lock);
	if (lock.owns_lock()) {
		map<string, string> serials = device_manager_.context()->serials(driver);
		Q_EMIT populate_serials_done(serials);
	}
}

void ConnectDialog::unset_connection()
{
	device_list_.clear();
	button_box_.button(QDialogButtonBox::Ok)->setDisabled(true);
}

void ConnectDialog::serial_toggled(bool checked)
{
	std::unique_lock<std::mutex> lock(populate_serials_mtx_, std::try_to_lock);
	if (lock.owns_lock())
		serial_config_->setEnabled(checked);
}

void ConnectDialog::tcp_toggled(bool checked)
{
	tcp_config_->setEnabled(checked);
}

void ConnectDialog::gpib_toggled(bool checked)
{
	gpib_libgpib_name_->setEnabled(checked);
}

void ConnectDialog::scan_pressed()
{
	device_list_.clear();

	const int d_index = drivers_.currentIndex();
	if (d_index == -1)
		return;

	shared_ptr<Driver> driver =
		drivers_.itemData(d_index).value<shared_ptr<Driver>>();

	assert(driver);

	map<const ConfigKey *, VariantBase> drvopts;

	if (serial_config_->isEnabled()) {
		QString serial;
		const int s_index = serial_devices_.currentIndex();
		if (s_index >= 0 && s_index < serial_devices_.count() &&
				serial_devices_.currentText() == serial_devices_.itemText(s_index))
			serial = serial_devices_.itemData(s_index).value<QString>();
		else
			serial = serial_devices_.currentText();
		drvopts[ConfigKey::CONN] = Variant<ustring>::create(
			serial.toUtf8().constData());

		// Set baud rate if specified
		if (serial_baudrate_.currentText().length() > 0)
			drvopts[ConfigKey::SERIALCOMM] = Variant<ustring>::create(
				QString("%1/8n1").arg(serial_baudrate_.currentText()).toUtf8().constData());
	}

	if (tcp_config_->isEnabled()) {
		QString host = tcp_host_->text();
		QString port = tcp_port_->text();
		if (!host.isEmpty()) {
			QString conn = tcp_protocol_->
				itemData(tcp_protocol_->currentIndex()).toString();
			conn = conn.arg(host, port);

			drvopts[ConfigKey::CONN] = Variant<ustring>::create(
				conn.toUtf8().constData());
		}
	}

	if (gpib_avialable_ && gpib_libgpib_name_->isEnabled()) {
		QString name = gpib_libgpib_name_->text();
		QString conn = QString("libgpib/%1").arg(name);

		drvopts[ConfigKey::CONN] = Variant<ustring>::create(
			conn.toUtf8().constData());
	}

	const list<shared_ptr<HardwareDevice>> devices =
		device_manager_.driver_scan(driver, drvopts);

	for (const auto &device : devices) {
		assert(device);

		QString text = device->display_name(device_manager_);
		text += QString(" with %1 channels").arg(
			device->sr_device()->channels().size());

		QListWidgetItem *const item = new QListWidgetItem(text,
			&device_list_);
		item->setData(Qt::UserRole, QVariant::fromValue(device));
		device_list_.addItem(item);
	}

	device_list_.setCurrentRow(0);
	button_box_.button(QDialogButtonBox::Ok)->setDisabled(device_list_.count() == 0);
}

void ConnectDialog::driver_selected(int index)
{
	shared_ptr<Driver> driver =
		drivers_.itemData(index).value<shared_ptr<Driver>>();

	unset_connection();

	populate_serials_start(driver);
}

} // namespace dialogs
} // namespace ui
} // namespace sv
