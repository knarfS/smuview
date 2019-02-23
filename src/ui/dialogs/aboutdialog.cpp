/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2017 Soeren Apel <soeren@apelpie.net>
 * Copyright (C) 2017-2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <memory>

#include <glib.h>
#include <boost/version.hpp>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QApplication>
#include <QDebug>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QTextDocument>

#include "aboutdialog.hpp"
#include "config.h"
#include "src/util.hpp"
#include "src/channels/basechannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/devices/basedevice.hpp"
#include "src/devices/configurable.hpp"
#include "src/devices/deviceutil.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/devices/properties/baseproperty.hpp"

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
	const int icon_size = 64;

	resize(600, 400);

	page_list = new QListWidget;
	page_list->setViewMode(QListView::IconMode);
	page_list->setIconSize(QSize(icon_size, icon_size));
	page_list->setMovement(QListView::Static);
	page_list->setMaximumWidth(icon_size + (icon_size / 2) + 2);
	page_list->setSpacing(12);

	pages = new QStackedWidget;
	create_pages();
	page_list->setCurrentIndex(page_list->model()->index(0, 0));

	QHBoxLayout *tab_layout = new QHBoxLayout;
	tab_layout->addWidget(page_list);
	tab_layout->addWidget(pages, Qt::AlignLeft);

	QDialogButtonBox *button_box = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QVBoxLayout* root_layout = new QVBoxLayout(this);
	root_layout->addLayout(tab_layout);
	root_layout->addWidget(button_box);

	connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));
	connect(page_list, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
		this, SLOT(on_page_changed(QListWidgetItem*, QListWidgetItem*)));
}

void AboutDialog::create_pages()
{
	// Device page
	if (device_) {
		pages->addWidget(get_device_page(pages));

		QListWidgetItem *device_button = new QListWidgetItem(page_list);
		device_button->setIcon(QIcon(":/icons/smuview.svg"));
		device_button->setText(tr("Device"));
		device_button->setTextAlignment(Qt::AlignHCenter);
		device_button->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}

	// About page
	pages->addWidget(get_about_page(pages));

	QListWidgetItem *about_button = new QListWidgetItem(page_list);
	about_button->setIcon(QIcon(":/icons/information.svg"));
	about_button->setText(tr("About"));
	about_button->setTextAlignment(Qt::AlignHCenter);
	about_button->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

QWidget *AboutDialog::get_about_page(QWidget *parent) const
{
	QLabel *icon = new QLabel();
	icon->setPixmap(QPixmap(QString::fromUtf8(":/icons/smuview.svg")));

	/* Setup the version field */
	QLabel *version_info = new QLabel();
	version_info->setText(tr("%1 %2<br />%3<br /><a href=\"http://%4\">%4</a>")
		.arg(QApplication::applicationName(),
		QApplication::applicationVersion(),
		tr("GNU GPL, version 3 or later"),
		QApplication::organizationDomain()));
	version_info->setOpenExternalLinks(true);

	shared_ptr<sigrok::Context> context = device_manager_.context();

	QString s;

	s.append("<style type=\"text/css\"> tr .id { white-space: pre; padding-right: 5px; } </style>");

	s.append("<table>");

	/* Library info */
	s.append("<tr><td colspan=\"2\"><b>" +
		tr("Libraries and features:") + "</b></td></tr>");

	s.append(QString("<tr><td><i>%1</i></td><td>%2</td></tr>")
		.arg(QString("Qt"), qVersion()));
	s.append(QString("<tr><td><i>%1</i></td><td>%2</td></tr>")
		.arg(QString("glibmm"), SV_GLIBMM_VERSION));
	s.append(QString("<tr><td><i>%1</i></td><td>%2</td></tr>")
		.arg(QString("Boost"), BOOST_LIB_VERSION));

	s.append(QString("<tr><td><i>%1</i></td><td>%2/%3 (rt: %4/%5)</td></tr>")
		.arg(QString("libsigrok"), SR_PACKAGE_VERSION_STRING,
		SR_LIB_VERSION_STRING, sr_package_version_string_get(),
		sr_lib_version_string_get()));

	GSList *l_orig = sr_buildinfo_libs_get();
	for (GSList *l = l_orig; l; l = l->next) {
		GSList *m = (GSList *)l->data;
		const char *lib = (const char *)m->data;
		const char *version = (const char *)m->next->data;
		s.append(QString("<tr><td><i>- %1</i></td><td>%2</td></tr>")
			.arg(QString(lib), QString(version)));
		g_slist_free_full(m, g_free);
	}
	g_slist_free(l_orig);

	char *host = sr_buildinfo_host_get();
	s.append(QString("<tr><td><i>- Host</i></td><td>%1</td></tr>")
		.arg(QString(host)));
	g_free(host);

	char *scpi_backends = sr_buildinfo_scpi_backends_get();
	s.append(QString("<tr><td><i>- SCPI backends</i></td><td>%1</td></tr>")
		.arg(QString(scpi_backends)));
	g_free(scpi_backends);

	/* Set up the supported field */
	s.append("<tr><td colspan=\"2\"></td></tr>");
	s.append("<tr><td colspan=\"2\"><b>" +
		tr("Supported hardware drivers:") + "</b></td></tr>");
	for (const auto &entry : context->drivers()) {
		s.append(QString("<tr><td class=\"id\"><i>%1</i></td><td>%2</td></tr>")
			.arg(QString::fromUtf8(entry.first.c_str()),
				QString::fromUtf8(entry.second->long_name().c_str())));
	}

	// No need for input formats
	/*
	s.append("<tr><td colspan=\"2\"></td></tr>");
	s.append("<tr><td colspan=\"2\"><b>" +
		tr("Supported input formats:") + "</b></td></tr>");
	for (const auto &entry : context->input_formats()) {
		s.append(QString("<tr><td class=\"id\"><i>%1</i></td><td>%2</td></tr>")
			.arg(QString::fromUtf8(entry.first.c_str()),
				QString::fromUtf8(entry.second->description().c_str())));
	}
	*/

	// No need for output formats
	/*
	s.append("<tr><td colspan=\"2\"></td></tr>");
	s.append("<tr><td colspan=\"2\"><b>" +
		tr("Supported output formats:") + "</b></td></tr>");
	for (const auto &entry : context->output_formats()) {
		s.append(QString("<tr><td class=\"id\"><i>%1</i></td><td>%2</td></tr>")
			.arg(QString::fromUtf8(entry.first.c_str()),
				QString::fromUtf8(entry.second->description().c_str())));
	}
	*/

	s.append("</table>");

	QTextDocument *supported_doc = new QTextDocument();
	supported_doc->setHtml(s);

	QTextBrowser *support_list = new QTextBrowser();
	support_list->setDocument(supported_doc);

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(icon, 0, 0, 1, 1);
	layout->addWidget(version_info, 0, 1, 1, 1);
	layout->addWidget(support_list, 1, 1, 1, 1);

	QWidget *page = new QWidget(parent);
	page->setLayout(layout);

	return page;
}

QWidget *AboutDialog::get_device_page(QWidget *parent) const
{
	QLabel *icon = new QLabel();
	icon->setPixmap(QPixmap(QString::fromUtf8(":/icons/smuview.svg")));

	// Device info
	auto sr_device = device_->sr_device();
	auto hw_device = dynamic_pointer_cast<devices::HardwareDevice>(device_);
	shared_ptr<sigrok::HardwareDevice> sr_hw_device = nullptr;
	if (hw_device)
		sr_hw_device = hw_device->sr_hardware_device();

	QString device_info_text("<b>");

	if (sr_device->version().length() > 0) {
		device_info_text.append(QString("%1 ").arg(
			QString::fromStdString(sr_device->vendor())));
	}
	device_info_text.append(QString("%1</b>").arg(
		QString::fromStdString(sr_device->model())));
	if (sr_device->version().length() > 0) {
		device_info_text.append(QString(" (%1)").arg(
			QString::fromStdString(sr_device->version())));
	}

	if (sr_device->serial_number().length() > 0) {
		device_info_text.append(QString("<br /><b>" + tr("Serial Number") + ":</b> %1").arg(
			QString::fromStdString(sr_device->serial_number())));
	}
	if (sr_device->connection_id().length() > 0) {
		device_info_text.append(QString("<br /><b>" + tr("Connection") + ":</b> %1").arg(
			QString::fromStdString(sr_device->connection_id())));
	}

	QLabel *device_info = new QLabel();
	device_info->setText(device_info_text);

	QString s;
	s.append("<style type=\"text/css\"> tr .id { white-space: pre; padding-right: 5px; } </style>");
	s.append("<table width=\"100%\" border=\"1\">");

	/* Device functions */
	s.append("<tr><td colspan=\"7\"><b>" +
		tr("Sigrok device functions:") + "</b></td></tr>");
	s.append(QString("<tr><td>&nbsp;</td><td colspan=\"6\">"));
	if (sr_hw_device) {
		const auto sr_keys = sr_hw_device->driver()->config_keys();
		QString sep("");
		for (const auto &sr_key : sr_keys) {
				s.append(sep).append(
					QString::fromStdString(sr_key->description()));
				sep = QString(", ");
		}
	}
	s.append(QString("</td></tr>"));
	s.append("<tr><td colspan=\"7\"><b>" +
		tr("SmuView device functions") + "</b></td></tr>");
	s.append(QString("<tr><td>&nbsp;</td><td colspan=\"6\">%1</td></tr>").arg(
		devices::deviceutil::format_device_type(device_->type())));
	s.append("<tr><td colspan=\"7\">&nbsp;</td></tr>");

	/* SmuView device configurables and config keys */
	if (hw_device) {
		s.append("<tr><td colspan=\"7\"><b>" +
			tr("SmuView device configurables and properties:") +
			"</b></td></tr>");
		const auto configurables = hw_device->configurables();
		for (const auto &cnf : configurables) {
			s.append(QString("<tr><td>&nbsp;</td><td>%1</td>").
				arg(cnf->name()));
			s.append(QString("<td>GET</td><td>Value</td><td>SET</td>"));
			s.append(QString("<td>LIST</td><td>Values</td></tr>"));
			auto props = cnf->properties();
			for (const auto &prop : props) {
				s.append(QString("<tr><td>&nbsp;</td>"));
				s.append(QString("<td><i>%1</i></td>").arg(
					devices::deviceutil::format_config_key(prop.first)));
				if (prop.second->is_getable()) {
					s.append(QString("<td>X</td>"));
					//if (prop.second->value().canConvert<QString>())
					//	s.append(QString("<td>%1</td>").arg(
					//		prop.second->value().toString()));
					//else
						s.append(QString("<td>?</td>"));
				}
				else
					s.append(QString("<td>&nbsp;</td><td>&nbsp;</td>"));
				if (prop.second->is_setable())
					s.append(QString("<td>X</td>"));
				else
					s.append(QString("<td>&nbsp;</td>"));
				if (prop.second->is_listable())
					s.append(QString("<td>X</td><td>&nbsp;</td>"));
				else
					s.append(QString("<td>&nbsp;</td><td>&nbsp;</td>"));

				s.append(QString("</tr>"));
			}
		}
		s.append("<tr><td colspan=\"7\">&nbsp;</td></tr>");
	}

	// SmuView all device signals
	// vector<shared_ptr<data::AnalogSignal>> all_signals()
	s.append("<tr><td colspan=\"2\"><b>" +
		tr("SmuView device signals (device->all_signals()):") +
		"</b></td></tr>");
	const auto signals = device_->all_signals();
	for (const auto &signal : signals) {
		s.append(QString("<tr><td>%1</td><td>%2</td></tr>").arg(
			signal->name(), signal->name()));
	}

	// SmuView channel name + channel
	// map<QString, shared_ptr<devices::Channel>> channel_name_map();
	s.append("<tr><td colspan=\"2\"><b>" +
		tr("SmuView channel name and channel (device->channel_name_map()):") +
		"</b></td></tr>");
	const auto ch_name_signal_map = device_->channel_name_map();
	for (const auto &ch_name_signal_pair : ch_name_signal_map) {
		s.append(QString("<tr><td>%1</td><td>%2</td></tr>").
			arg(ch_name_signal_pair.first).
			arg(ch_name_signal_pair.second->name()));
	}

	//map<shared_ptr<sigrok::Channel>, shared_ptr<devices::Channel>> sr_channel_map();

	// SmuView device channel group names + channels
	// map<QString, vector<shared_ptr<devices::Channel>>> channel_group_name_map();
	s.append("<tr><td colspan=\"2\"><b>" +
		tr("SmuView channel group name and channels (device->channel_group_name_map()):") +
		"</b></td></tr>");
	const auto cg_name_channel_map = device_->channel_group_name_map();
	for (const auto &cg_name_channel_pair : cg_name_channel_map) {
		s.append(QString("<tr><td>%1</td><td></td></tr>").arg(
			cg_name_channel_pair.first));
		for (const auto &channel : cg_name_channel_pair.second) {
			s.append(QString("<tr><td></td><td>%1</td></tr>").arg(
				channel->name()));
		}
	}

	/* Sigrok Device channel groups + channels */
	s.append("<tr><td colspan=\"2\"><b>" +
		tr("Sigrok channel groups and channels:") + "</b></td></tr>");
	const auto sr_cgs = sr_device->channel_groups();
	for (const auto &sr_cg_pair : sr_cgs) {
		QString channel_names("");
		QString sep("");
		shared_ptr<sigrok::ChannelGroup> sr_cg = sr_cg_pair.second;
		for (const auto &sr_ch : sr_cg->channels()) {
			channel_names.append(sep);
			channel_names.append(QString::fromStdString(sr_ch->name()));
			sep = QString(" ");
		}
		s.append(QString("<tr><td><i>%1</i></td><td>%2</td></tr>").arg(
			QString::fromStdString(sr_cg_pair.first), channel_names));
	}

	/* Sigrok Device channel */
	s.append("<tr><td colspan=\"2\"><b>" +
		tr("Sigrok device channels:") + "</b></td></tr>");
	const auto sr_channels = sr_device->channels();
	for (const auto &sr_channel : sr_channels) {
		s.append(QString("<tr><td><i>%1</i></td><td></td></tr>").arg(
			QString::fromStdString(sr_channel->name())));
	}

	s.append("</table>");

	QTextDocument *device_doc = new QTextDocument();
	device_doc->setHtml(s);

	QTextBrowser *device_list = new QTextBrowser();
	device_list->setDocument(device_doc);

	QGridLayout *layout = new QGridLayout();
	layout->addWidget(icon, 0, 0, 1, 1);
	layout->addWidget(device_info, 0, 1, 1, 1);
	layout->addWidget(device_list, 1, 1, 1, 1);

	QWidget *page = new QWidget(parent);
	page->setLayout(layout);

	return page;
}

void AboutDialog::on_page_changed(
	QListWidgetItem *current, QListWidgetItem *previous)
{
	if (!current)
		current = previous;

	pages->setCurrentIndex(page_list->row(current));
}

} // namespace dialogs
} // namespace ui
} // namespace sv
