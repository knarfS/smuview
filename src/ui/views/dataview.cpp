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

#include <QAction>
#include <QDebug>
#include <QScrollBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolBar>
#include <QVBoxLayout>

#include "dataview.hpp"
#include "src/session.hpp"
#include "src/data/analogsignal.hpp"

namespace sv {
namespace ui {
namespace views {

DataView::DataView(Session &session,
		shared_ptr<sv::data::AnalogSignal> signal,
		QWidget *parent) :
	BaseView(session, parent),
	signal_(signal),
	next_signal_pos_(0),
	auto_scroll_(true),
	action_auto_scroll_(new QAction(this)),
	action_add_signal_(new QAction(this))
{
	setup_ui();
	setup_toolbar();
	connect_signals();
}

QString DataView::title() const
{
	return signal_->display_name() + " " + tr("Data");
}

void DataView::setup_ui()
{
	QString value_text = QString("%1 [%3").
		arg(signal_->quantity_name()).arg(signal_->unit_name());
	if (!signal_->quantity_flags_name().isEmpty()) {
		value_text.append(" ").append(signal_->quantity_flags_name());
	}
	value_text.append("]");

	QVBoxLayout *layout = new QVBoxLayout();

	data_table_ = new QTableWidget();
	data_table_->setColumnCount(2);
	QTableWidgetItem *time_header_item = new QTableWidgetItem(tr("Time [s]"));
	time_header_item->setTextAlignment(Qt::AlignVCenter);
	data_table_->setHorizontalHeaderItem(0, time_header_item);
	QTableWidgetItem *value_header_item = new QTableWidgetItem(value_text);
	value_header_item->setTextAlignment(Qt::AlignVCenter);
	data_table_->setHorizontalHeaderItem(1, value_header_item);
	data_table_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	layout->addWidget(data_table_);

	this->central_widget_->setLayout(layout);

	this->populate_table();
}

void DataView::setup_toolbar()
{
	action_auto_scroll_->setText(tr("Auto scroll"));
	action_auto_scroll_->setIcon(
		QIcon::fromTheme("go-bottom",
		QIcon(":/icons/go-bottom.png")));
	action_auto_scroll_->setCheckable(true);
	action_auto_scroll_->setChecked(auto_scroll_);
	connect(action_auto_scroll_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_auto_scroll_triggered()));

	/* TODO: Implement
	action_add_signal_->setText(tr("Add signal"));
	action_add_signal_->setIcon(
		QIcon::fromTheme("office-chart-line",
		QIcon(":/icons/office-chart-line.png")));
	connect(action_add_signal_, SIGNAL(triggered(bool)),
		this, SLOT(on_action_add_signal_triggered()));
	*/

	toolbar_ = new QToolBar("Data View Toolbar");
	toolbar_->addAction(action_auto_scroll_);
	//toolbar_->addSeparator();
	// TODO: Implement:
	//toolbar_->addAction(action_add_signal_);
	this->addToolBar(Qt::TopToolBarArea, toolbar_);
}

void DataView::connect_signals()
{
	connect(signal_.get(), SIGNAL(sample_appended()),
		this, SLOT(populate_table()));
}

void DataView::populate_table()
{
	size_t signal_size = signal_->get_sample_count();
	while (next_signal_pos_ < signal_size) {
		int row  = data_table_->rowCount();
		data_table_->insertRow(row);

		sv::data::sample_t sample = signal_->get_sample(next_signal_pos_, true);
		QTableWidgetItem *time_item =
			new QTableWidgetItem(tr("%1").arg(sample.first));
		data_table_->setItem(row, 0, time_item);
		QTableWidgetItem *value_item =
			new QTableWidgetItem(tr("%1").arg(sample.second));
		data_table_->setItem(row, 1, value_item);

		++next_signal_pos_;
	}

	if (auto_scroll_)
		data_table_->scrollToBottom();
}

void DataView::on_action_auto_scroll_triggered()
{
	auto_scroll_ = !auto_scroll_;
	action_auto_scroll_->setChecked(auto_scroll_);
}

void DataView::on_action_add_signal_triggered()
{
}

} // namespace views
} // namespace ui
} // namespace sv

