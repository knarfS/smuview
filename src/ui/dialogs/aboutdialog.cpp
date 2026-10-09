/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Soeren Apel <soeren@apelpie.net>
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

#include <memory>

#include <glib.h>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSize>
#include <QTabWidget>
#include <QTextBrowser>
#include <QUrl>

#include "aboutdialog.hpp"
#include "src/data/properties/baseproperty.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/utils/apputil.hpp"

using std::dynamic_pointer_cast;

namespace sv {
namespace ui {
namespace dialogs {

AboutDialog::AboutDialog(DeviceManager &device_manager,
		shared_ptr<sv::devices::BaseDevice> device,
		QWidget *parent) :
	QDialog(parent),
	device_manager_(device_manager),
	device_(device)
{
	setup_ui();
}

void AboutDialog::setup_ui()
{
	QIcon main_icon;
	main_icon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(main_icon);
	this->setWindowTitle(tr("About SmuView"));
	this->resize(600, 400);

	QVBoxLayout *main_layout = new QVBoxLayout();

	QHBoxLayout *header_layout = new QHBoxLayout();

	QLabel *smuview_icon = new QLabel();
	smuview_icon->setPixmap(QIcon(":/icons/smuview.svg").pixmap(64, 64));
	header_layout->addWidget(smuview_icon, 0, Qt::AlignHCenter);

	header_layout->addSpacing(20);

	QString header_text = QStringLiteral(
		"<center>"
		"  <big><b>%1</b></big><br>"
		"  %2<br>"
		"  %3"
		"</center>").arg(
			QApplication::applicationName().toHtmlEscaped(),
			QApplication::applicationVersion().toHtmlEscaped(),
			tr("GNU GPL, version 3 or later").toHtmlEscaped());

	QLabel *header_label = new QLabel();
	/* Object name for selftest functionality */
	header_label->setObjectName("about_header_text");
	header_label->setText(header_text);
	header_layout->addWidget(header_label);

	header_layout->addStretch(5);

	QVBoxLayout *header_buttons = new QVBoxLayout();
	QPushButton *version_button = new QPushButton();
	version_button->setText(tr("Copy version info"));
	connect(version_button, &QPushButton::clicked,
		this, &AboutDialog::copy_version_info);
	header_buttons->addWidget(version_button);
	QPushButton *manual_button = new QPushButton();
	manual_button->setText(tr("SmuView manual"));
	connect(manual_button, &QPushButton::clicked,
		this, &AboutDialog::open_manual);
	header_buttons->addWidget(manual_button);
	header_layout->addLayout(header_buttons);

	main_layout->addLayout(header_layout);

	QTabWidget *tab_widget = new QTabWidget();
	tab_widget->addTab(get_about_page(), tr("About"));
	if (device_)
		tab_widget->addTab(get_device_page(), tr("Device"));
	tab_widget->addTab(get_versions_page(), tr("Versions"));
	tab_widget->addTab(get_license_page(), tr("Licenses"));
	main_layout->addWidget(tab_widget);

	QDialogButtonBox *button_box = new QDialogButtonBox(QDialogButtonBox::Ok);
	connect(button_box, &QDialogButtonBox::accepted,
		this, &AboutDialog::accept);
	main_layout->addWidget(button_box);

	this->setLayout(main_layout);
}

QTextBrowser *AboutDialog::get_about_page() const
{
	QString about_html = QStringLiteral(
		"<h3>%1</h3>"
		"<p>%2</p>"
		"<h3>%3</h3>"
		"<ul>"
		"  <li>%4: <a href=\"%5\">%5</a></li>"
		"  <li>%6: <a href=\"%7\">%7</a></li>"
		"  <li>%8: <a href=\"%9\">%9</a></li>"
		"</ul>").arg(
			tr("About").toHtmlEscaped(),
			tr("SmuView is a GUI for sigrok that supports power supplies, "
				"electronic loads and all sorts of measurement devices like "
				"multimeters, LCR meters and so on.").toHtmlEscaped(),
			tr("Links").toHtmlEscaped(),
			tr("Homepage").toHtmlEscaped(),
			"https://github.com/knarfS/SmuView",
			tr("Manual").toHtmlEscaped(),
			"https://knarfs.github.io/doc/smuview/continuous/manual.html",
			tr("sigrok Wiki").toHtmlEscaped(),
			"https://sigrok.org");

	// Max. of 9 arguments for Qt < 5.14
	about_html += QStringLiteral(
		"<h3>%1</h3>"
		"<ul>"
		"  <li>%2: <a href=\"%3\">%3</a></li>"
		"</ul>").arg(
			tr("Bugtracker").toHtmlEscaped(),
			tr("Report bugs here").toHtmlEscaped(),
			"https://github.com/knarfS/SmuView/issues");

	QTextBrowser *about_widget = new QTextBrowser();
	about_widget->setHtml(about_html);
	about_widget->setOpenExternalLinks(true);

	return about_widget;
}

QTextBrowser *AboutDialog::get_versions_page() const
{
	QTextBrowser *versions_widget = new QTextBrowser();
	QString version_md = utils::apputil::get_versions_md();
	version_md += utils::apputil::get_supported_drivers_md(device_manager_);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
	versions_widget->setMarkdown(version_md);
#else
	versions_widget->setPlainText(version_md);
#endif

	return versions_widget;
}

QTextBrowser *AboutDialog::get_device_page() const
{
	// Device info
	auto sr_device = device_->sr_device();
	auto hw_device = dynamic_pointer_cast<devices::HardwareDevice>(device_);
	shared_ptr<sigrok::HardwareDevice> sr_hw_device = nullptr;
	if (hw_device)
		sr_hw_device = hw_device->sr_hardware_device();

	QString device_name;
	if (sr_device->vendor().length() > 0) {
		device_name += QString("%1 ").arg(
			QString::fromStdString(sr_device->vendor()));
	}
	device_name += QString::fromStdString(sr_device->model());
	if (sr_device->version().length() > 0) {
		device_name += QString(" (%1)").arg(
			QString::fromStdString(sr_device->version()));
	}

	QString serial_nr("-");
	if (sr_device->serial_number().length() > 0)
		serial_nr = QString::fromStdString(sr_device->serial_number());

	QString conn_id("-");
	if (sr_device->connection_id().length() > 0)
		conn_id = QString::fromStdString(sr_device->connection_id());

	QString dev_id("-");
	if (device_->id().length() > 0)
		dev_id = QString::fromStdString(device_->id());

	QString dev_types_sigrok;
	if (sr_hw_device) {
		const auto sr_keys = sr_hw_device->driver()->config_keys();
		QString sep("");
		for (const auto &sr_key : sr_keys) {
			dev_types_sigrok.append(sep).append( // TODO join
				QString::fromStdString(sr_key->description()));
			sep = QString(", ");
		}
	}
	else
		dev_types_sigrok = "-";

	QString device_html = QStringLiteral(
		"<h2>%1</h2>"
		"<ul>"
		"  <li><b>%2:</b> %3</li>"
		"  <li><b>%4:</b> %5</li>"
		"  <li><b>%6:</b> %7</li>"
		"  <li><b>%8:</b> %9</li>").arg(
			device_name.toHtmlEscaped(),
			tr("Serial Number").toHtmlEscaped(), serial_nr.toHtmlEscaped(),
			tr("Connection").toHtmlEscaped(), conn_id.toHtmlEscaped(),
			tr("Device ID").toHtmlEscaped(), dev_id.toHtmlEscaped(),
			tr("Device types (sigrok)").toHtmlEscaped(),
			dev_types_sigrok.toHtmlEscaped());

	// Max. of 9 arguments for Qt < 5.14
	device_html += QStringLiteral(
		"  <li><b>%1:</b> %2</li>"
		"</ul>").arg(
			tr("Device types (SmuView)").toHtmlEscaped(),
			devices::deviceutil::format_device_type(device_->type())
				.toHtmlEscaped());

	if (hw_device) {
		for (const auto &[_, configurable] : hw_device->configurable_map()) {
			device_html += QStringLiteral(
				"<h3>%1 %2</h3>"
				"<p></p>"
				"<table border=\"1\" cellspacing=\"0\" cellpadding=\"4\">"
				"  <thead>"
				"    <tr>"
				"      <th></th>"
				"      <th align=\"center\">GET</th>"
				"      <th align=\"center\">SET</th>"
				"      <th align=\"center\">LIST</th>"
				"    </tr>"
				"  </thead>").arg(
					tr("Configurable").toHtmlEscaped(),
					configurable->display_name().toHtmlEscaped());

			for (const auto &[ck, property] : configurable->property_map()) {
				device_html += QStringLiteral(
					"<tr>"
					"  <td align=\"left\"><b>%1</b></td>"
					"  <td align=\"center\">%2</td>"
					"  <td align=\"center\">%3</td>"
					"  <td align=\"center\">%4</td>"
					"</tr>").arg(
						devices::deviceutil::format_config_key(ck),
						property->is_getable() ? tr("yes") : tr("no"),
						property->is_setable() ? tr("yes") : tr("no"),
						property->is_listable() ? tr("yes") : tr("no"));
			}

			device_html += "</table>";
		}
	}

	QTextBrowser *device_widget = new QTextBrowser();
	device_widget->setHtml(device_html);
	device_widget->setOpenExternalLinks(true);

	return device_widget;
}

QTextBrowser *AboutDialog::get_license_page() const
{
	QString license_html = QStringLiteral(
		"<p>%1</p>"
		"<p><a href=\"https://www.gnu.org/licenses/gpl-3.0\">%2</a></p>"
		"<p>%3</p>").arg(
			tr("SmuView is licensed under the").toHtmlEscaped(),
			tr("GNU General Public License, version 3 or later (GPLv3+)")
				.toHtmlEscaped(),
			tr("Some individual source files are licensed under GPLv2+ or "
				"GPLv3+ specifically, but the project as a whole is "
				"distributed under GPLv3+ terms.").toHtmlEscaped());

	QTextBrowser *license_widget = new QTextBrowser();
	license_widget->setHtml(license_html);
	license_widget->setOpenExternalLinks(true);

	return license_widget;
}

void AboutDialog::copy_version_info()
{
	QString versions = utils::apputil::get_versions_md();
	versions += utils::apputil::get_supported_drivers_md(device_manager_);

	QClipboard *clipboard = QGuiApplication::clipboard();
	clipboard->setText(versions);
}

void AboutDialog::open_manual()
{
	QUrl manual_url(
		"https://knarfs.github.io/doc/smuview/continuous/manual.html");
	QDesktopServices::openUrl(manual_url);
}

} // namespace dialogs
} // namespace ui
} // namespace sv
