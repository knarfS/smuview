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

#include <cassert>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QBoxLayout>
#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QRadioButton>
#include <QVariant>

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

using sv::devices::DeviceType;
using sv::devices::deviceutil::format_device_type;
using sv::devices::deviceutil::get_device_type_map;
using sv::devices::deviceutil::get_device_types;
using sv::devices::deviceutil::is_supported_device;
using sv::devices::deviceutil::is_supported_driver;
using sv::devices::HardwareDevice;

namespace sv {
namespace ui {
namespace dialogs {

ConnectDialog::ConnectDialog(sv::DeviceManager &device_manager,
		QWidget *parent) :
	QDialog(parent),
	device_manager_(device_manager)
{
	qRegisterMetaType<std::map<std::string, std::string>>(
		"std::map<std::string, std::string>");

	check_available_libs();

	connect(this, &ConnectDialog::populate_serials_done,
		this, &ConnectDialog::populate_serials_finish);

	setup_ui();

	unset_connection();

	// Initially populate serials for current selected device
	driver_selected(drivers_->currentIndex());
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
	const QListWidgetItem *const item = device_list_->currentItem();
	if (!item)
		return shared_ptr<HardwareDevice>();

	return item->data(Qt::UserRole).value<shared_ptr<HardwareDevice>>();
}

void ConnectDialog::setup_ui()
{
	QIcon main_icon;
	main_icon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(main_icon);
	this->setWindowTitle(tr("Connect to Device"));

	QVBoxLayout *main_layout = new QVBoxLayout();

	QFormLayout *form_layout = new QFormLayout();

	// Driver selection
	QGroupBox *groupbox_drv = new QGroupBox(tr("Step 1: Choose the driver"));
	QVBoxLayout *vbox_drv = new QVBoxLayout();

	filters_ = new QComboBox();
	populate_filters();
	connect(filters_, QOverload<int>::of(&QComboBox::activated),
		this, &ConnectDialog::filter_selected);
	vbox_drv->addWidget(filters_);
	drivers_ = new QComboBox();
	populate_drivers(DeviceType::Any);
	connect(drivers_, QOverload<int>::of(&QComboBox::activated),
		this, &ConnectDialog::driver_selected);
	vbox_drv->addWidget(drivers_);

	groupbox_drv->setLayout(vbox_drv);
	form_layout->addRow(groupbox_drv);

	// Interface configuration
	QGroupBox *groupbox_if = new QGroupBox(tr("Step 2: Choose the interface"));
	QVBoxLayout *vbox_if = new QVBoxLayout();

	QRadioButton *radiobtn_usb = new QRadioButton(tr("&USB"));
	radiobtn_usb->setChecked(true);
	vbox_if->addWidget(radiobtn_usb);

	radiobtn_serial_ = new QRadioButton(tr("Serial &Port"));
	connect(radiobtn_serial_, &QRadioButton::toggled,
		this, &ConnectDialog::serial_toggled);
	vbox_if->addWidget(radiobtn_serial_);

	serial_config_ = new QWidget();
	QHBoxLayout *serial_config_layout = new QHBoxLayout(serial_config_);
	serial_devices_ = new QComboBox();
	serial_devices_->setEditable(true);
	serial_config_layout->addWidget(serial_devices_,1);
	serial_baudrate_ = new QComboBox();
	serial_baudrate_->setEditable(true);
	serial_baudrate_->addItem("");
	for (const auto &baud : BAUD_RATES_DEFAULT)
		serial_baudrate_->addItem(QString::number(baud), QVariant(baud));
	serial_config_layout->addWidget(serial_baudrate_);
	serial_config_layout->addWidget(new QLabel("baud"));
	serial_config_->setEnabled(false);
	vbox_if->addWidget(serial_config_);

	QRadioButton *radiobtn_tcp = new QRadioButton(tr("&TCP/IP"));
	connect(radiobtn_tcp, &QRadioButton::toggled,
		this, &ConnectDialog::tcp_toggled);
	vbox_if->addWidget(radiobtn_tcp);

	tcp_config_ = new QWidget();
	QHBoxLayout *tcp_config_layout = new QHBoxLayout(tcp_config_);
	tcp_host_ = new QLineEdit;
	tcp_host_->setText(TCP_HOST_DEFAULT);
	tcp_config_layout->addWidget(tcp_host_);
	tcp_config_layout->addWidget(new QLabel(":"));
	tcp_port_ = new QSpinBox;
	tcp_port_->setRange(TCP_PORT_MIN, TCP_PORT_MAX);
	tcp_port_->setValue(TCP_PORT_DEFAULT);
	tcp_config_layout->addWidget(tcp_port_);
	tcp_config_layout->addSpacing(30);
	tcp_config_layout->addWidget(new QLabel(tr("Protocol:")));
	tcp_protocol_ = new QComboBox();
	for (const auto &protocol : TCP_PROTOCOLS)
		tcp_protocol_->addItem(protocol.label, QVariant(protocol.format));
	tcp_config_layout->addWidget(tcp_protocol_);
	tcp_config_->setEnabled(false);
	vbox_if->addWidget(tcp_config_);

	if (gpib_avialable_) {
		QRadioButton *radiobtn_gpib = new QRadioButton(tr("&GPIB"));
		connect(radiobtn_gpib, &QRadioButton::toggled,
			this, &ConnectDialog::gpib_toggled);
		vbox_if->addWidget(radiobtn_gpib);

		/*
		 * TODO: Replace with QComboBox and prefill with available GPIB
		 * connection strings (like the serial box).
		 * Must be implemented in libsigrok.
		 */
		gpib_libgpib_name_ = new QLineEdit;
		gpib_libgpib_name_->setEnabled(false);
		vbox_if->addWidget(gpib_libgpib_name_);
	}

	groupbox_if->setLayout(vbox_if);
	form_layout->addRow(groupbox_if);

	// Scan
	QGroupBox *groupbox_scan = new QGroupBox(tr("Step 3: Scan for devices"));
	QVBoxLayout *vbox_scan = new QVBoxLayout;

	QPushButton *scan_button = new QPushButton(
		tr("&Scan for devices using driver above"));
	connect(scan_button, &QPushButton::pressed,
		this, &ConnectDialog::scan_pressed);
	vbox_scan->addWidget(scan_button);

	groupbox_scan->setLayout(vbox_scan);
	form_layout->addRow(groupbox_scan);

	// Devices
	QGroupBox *groupbox_select = new QGroupBox(tr("Step 4: Select the device"));
	QVBoxLayout *vbox_select = new QVBoxLayout;

	device_list_ = new QListWidget();
	// Let the device list occupy only the minimum space needed
	device_list_->setMaximumHeight(device_list_->minimumSizeHint().height());
	vbox_select->addWidget(device_list_);

	groupbox_select->setLayout(vbox_select);
	form_layout->addRow(groupbox_select);

	main_layout->addLayout(form_layout);

	// Button box
	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, &QDialogButtonBox::accepted,
		this, &ConnectDialog::accept);
	connect(button_box_, &QDialogButtonBox::rejected,
		this, &ConnectDialog::reject);

	this->setLayout(main_layout);
}

void ConnectDialog::populate_filters()
{
	// Add all know device types and sort by name
	for (const auto &type : get_device_type_map()) {
		if (is_supported_device(type.device_type))
			filters_->addItem(type.name, QVariant::fromValue(type.device_type));
	}
	filters_->model()->sort(0);

	// Add the 'Any' type as first item and select
	filters_->insertItem(0,
		format_device_type(DeviceType::Any),
		QVariant::fromValue(DeviceType::Any));
	filters_->setCurrentIndex(0);
}

void ConnectDialog::populate_drivers(DeviceType type_filter)
{
	drivers_->clear();

	for (const auto &entry : device_manager_.context()->drivers()) {
		const auto &name = entry.first;
		const auto &sr_driver = entry.second;

		bool match = false;
		if (type_filter == DeviceType::Any)
			// No filter -> accept any devices
			match = true;
		else if (is_supported_driver(sr_driver) &&
				get_device_types(sr_driver).count(type_filter) > 0)
			match = true;

		if (match) {
			drivers_->addItem(
				QString("%1 (%2)").arg(
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
	serial_devices_->clear();
	serial_devices_->addItem(tr("Loading..."));
	serial_config_->setDisabled(true);

	populate_serials_thread_ =
		std::thread(&ConnectDialog::populate_serials_thread_proc, this, driver);
	populate_serials_thread_.detach();
}

void ConnectDialog::populate_serials_finish(
	const std::map<std::string, std::string> &serials)
{
	std::lock_guard<std::mutex> lock(populate_serials_mtx_);

	serial_devices_->clear();
	for (const auto &serial : serials) {
		serial_devices_->addItem(QString("%1 (%2)").arg(
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
	device_list_->clear();
	button_box_->button(QDialogButtonBox::Ok)->setDisabled(true);
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
	device_list_->clear();

	const int d_index = drivers_->currentIndex();
	if (d_index == -1)
		return;

	shared_ptr<Driver> driver =
		drivers_->itemData(d_index).value<shared_ptr<Driver>>();

	assert(driver);

	map<const ConfigKey *, VariantBase> drvopts;

	if (serial_config_->isEnabled()) {
		QString serial;
		const int s_index = serial_devices_->currentIndex();
		if (s_index >= 0 && s_index < serial_devices_->count() &&
				serial_devices_->currentText() == serial_devices_->itemText(s_index))
			serial = serial_devices_->itemData(s_index).value<QString>();
		else
			serial = serial_devices_->currentText();
		drvopts[ConfigKey::CONN] = Variant<ustring>::create(
			serial.toUtf8().constData());

		// Set baud rate if specified (TODO: use value)
		if (serial_baudrate_->currentText().length() > 0) {
			drvopts[ConfigKey::SERIALCOMM] = Variant<ustring>::create(
				QString("%1/8n1").arg(
					serial_baudrate_->currentText()).toUtf8().constData());
		}
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

		QListWidgetItem *const item = new QListWidgetItem(text);
		item->setData(Qt::UserRole, QVariant::fromValue(device));
		device_list_->addItem(item);
	}

	device_list_->setCurrentRow(0);
	button_box_->button(QDialogButtonBox::Ok)->
		setDisabled(device_list_->count() == 0);
}

void ConnectDialog::filter_selected(int index)
{
	unset_connection();

	populate_drivers(filters_->itemData(index).value<DeviceType>());
}

void ConnectDialog::driver_selected(int index)
{
	unset_connection();

	populate_serials_start(
		drivers_->itemData(index).value<shared_ptr<Driver>>());
}

} // namespace dialogs
} // namespace ui
} // namespace sv
