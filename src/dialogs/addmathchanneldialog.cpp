/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2018 Frank Stettner <frank-stettner@gmx.net>
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

#include <QComboBox>
#include <QDebug>
#include <QFormLayout>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "addmathchanneldialog.hpp"
#include "src/channels/basechannel.hpp"
#include "src/channels/mathchannel.hpp"
#include "src/channels/multiplychannel.hpp"
#include "src/data/analogsignal.hpp"
#include "src/devices/device.hpp"
#include "src/devices/hardwaredevice.hpp"
#include "src/widgets/quantitycombobox.hpp"
#include "src/widgets/quantityflagslist.hpp"
#include "src/widgets/signaltree.hpp"
#include "src/widgets/unitcombobox.hpp"

using std::static_pointer_cast;

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)

namespace sv {
namespace dialogs {

AddMathChannelDialog::AddMathChannelDialog(const Session &session,
		const shared_ptr<devices::HardwareDevice> device,
		QWidget *parent) :
	QDialog(parent),
	session_(session),
	device_(device)
{
	setup_ui();
}

void AddMathChannelDialog::setup_ui()
{
	QIcon mainIcon;
	mainIcon.addFile(QStringLiteral(":/icons/smuview.ico"),
		QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(mainIcon);
	this->setWindowTitle(tr("Add Math Channel"));

	QFormLayout *form_layout = new QFormLayout();

	name_edit_ = new QLineEdit();
	form_layout->addRow(tr("Name"), name_edit_);

	quantity_box_ = new widgets::QuantityComboBox();
	form_layout->addRow(tr("Quantity"), quantity_box_);

	quantity_flags_list_ = new widgets::QuantityFlagsList();
	form_layout->addRow(tr("Quantity Flags"), quantity_flags_list_);

	unit_box_ = new widgets::UnitComboBox();
	form_layout->addRow(tr("Unit"), unit_box_);

	multiply_signal_1_tree_ = new widgets::SignalTree(
		session_, true, true, false, device_);
	form_layout->addRow(tr("Signal 1"), multiply_signal_1_tree_);

	multiply_signal_2_tree_ = new widgets::SignalTree(
		session_, true, true, false, device_);
	form_layout->addRow(tr("Signal 2"), multiply_signal_2_tree_);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	form_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(form_layout);


	/*
	QVBoxLayout *main_layout = new QVBoxLayout;



	tab_widget_ = new QTabWidget();
	this->setup_ui_multiply_tab();
	this->setup_ui_divide_tab();
	this->setup_ui_integrate_tab();
	//tab_widget_->setCurrentIndex(selected_view_type_);
	main_layout->addWidget(tab_widget_);

	button_box_ = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
	main_layout->addWidget(button_box_);
	connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	this->setLayout(main_layout);
	*/
}

void AddMathChannelDialog::setup_ui_multiply_tab()
{
	QString title(tr("Multiply"));
	QWidget *multiply_widget = new QWidget();

	QVBoxLayout *main_layout = new QVBoxLayout;

	multiply_signal_1_tree_ = new widgets::SignalTree(
		session_, true, true, false, device_);
	main_layout->addWidget(multiply_signal_1_tree_);

	multiply_signal_2_tree_ = new widgets::SignalTree(
		session_, true, true, false, device_);
	main_layout->addWidget(multiply_signal_2_tree_);
	multiply_widget->setLayout(main_layout);

	QFormLayout *form_layout = new QFormLayout();

	widgets::QuantityComboBox *quantity_box_ =
		new widgets::QuantityComboBox();
	form_layout->addRow(tr("Quantity"), quantity_box_);

	widgets::QuantityFlagsList *quantity_flags_list_ =
		new widgets::QuantityFlagsList();
	form_layout->addRow(tr("Quantity Flags"), quantity_flags_list_);

	widgets::UnitComboBox *unit_box_ = new widgets::UnitComboBox();
	form_layout->addRow(tr("Unit"), unit_box_);

	main_layout->addLayout(form_layout);

	tab_widget_->addTab(multiply_widget, title);
}

void AddMathChannelDialog::setup_ui_divide_tab()
{
	QString title(tr("Divide"));
	QWidget *divide_widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();
	divide_widget->setLayout(layout);

	tab_widget_->addTab(divide_widget, title);
}

void AddMathChannelDialog::setup_ui_integrate_tab()
{
	QString title(tr("Integrate"));
	QWidget *integrate_widget = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();
	integrate_widget->setLayout(layout);

	tab_widget_->addTab(integrate_widget, title);
}

vector<shared_ptr<channels::BaseChannel>> AddMathChannelDialog::channels()
{
	return channels_;
}

void AddMathChannelDialog::accept()
{
	if (name_edit_->text().size() == 0)
		return;

	shared_ptr<data::AnalogSignal> signal_1;
	auto signals_1 = multiply_signal_1_tree_->selected_signals();
	if (signals_1.size() != 1)
		return;
	signal_1 = static_pointer_cast<data::AnalogSignal>(signals_1[0]);

	shared_ptr<data::AnalogSignal> signal_2;
	auto signals_2 = multiply_signal_2_tree_->selected_signals();
	if (signals_2.size() != 1)
		return;
	signal_2 = static_pointer_cast<data::AnalogSignal>(signals_2[0]);

	auto channel = make_shared<channels::MultiplyChannel>(
		quantity_box_->selected_sr_quantity(),
		quantity_flags_list_->selected_sr_quantity_flags(),
		unit_box_->selected_sr_unit(),
		signal_1, signal_2,
		device_, tr("Math User"), name_edit_->text(),
		signal_1->signal_start_timestamp());

	channels_.push_back(channel);

	QDialog::accept();
}

} // namespace dialogs
} // namespace sv
