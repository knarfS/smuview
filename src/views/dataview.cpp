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

#include <QDebug>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include "dataview.hpp"
#include "src/session.hpp"
#include "src/data/analogsignal.hpp"

namespace sv {
namespace views {

DataView::DataView(const Session &session,
		shared_ptr<data::AnalogSignal> signal,
		QWidget *parent) :
	BaseView(session, parent),
	signal_(signal),
	next_signal_pos_(0)
{
	setup_ui();
	populate_table();
	connect_signals();
}

QString DataView::title() const
{
	return signal_->name() + " " + tr("Data");
}

void DataView::setup_ui()
{
	QVBoxLayout *layout = new QVBoxLayout();

	data_table_ = new QTableWidget();
	data_table_->setColumnCount(2);
	QTableWidgetItem *time_header_item = new QTableWidgetItem(tr("Time"));
    time_header_item->setTextAlignment(Qt::AlignVCenter);
	data_table_->setHorizontalHeaderItem(0, time_header_item);
	QTableWidgetItem *value_header_item = new QTableWidgetItem(tr("Value"));
    value_header_item->setTextAlignment(Qt::AlignVCenter);
	data_table_->setHorizontalHeaderItem(1, value_header_item);
	layout->addWidget(data_table_);

	this->centralWidget_->setLayout(layout);
}

void DataView::populate_table()
{
	size_t signal_size = signal_->get_sample_count();
	while (next_signal_pos_ < signal_size) {
		int row  = data_table_->rowCount();
		data_table_->insertRow(row);

		data::sample_t sample = signal_->get_sample(next_signal_pos_, true);
		QTableWidgetItem *time_item =
			new QTableWidgetItem(tr("%1").arg(sample.first));
		data_table_->setItem(row, 0, time_item);
		QTableWidgetItem *value_item =
			new QTableWidgetItem(tr("%1").arg(sample.second));
		data_table_->setItem(row, 1, value_item);

		++next_signal_pos_;
	}
	// TODO: Implement free scrolling. when at the bottom, then fix
	data_table_->scrollToBottom();
}

void DataView::connect_signals()
{
	connect(signal_.get(), SIGNAL(sample_added()),
		this, SLOT(populate_table()));
}

} // namespace views
} // namespace sv

