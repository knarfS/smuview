/*
 * This file is part of the SmuView project.
 *
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

#ifndef DATA_ANALOGDATA_HPP
#define DATA_ANALOGDATA_HPP

#include <memory>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QObject>

#include "basedata.hpp"

using std::shared_ptr;
using std::vector;

namespace sv {
namespace data {

class AnalogData : public BaseData
{
	Q_OBJECT

public:
	AnalogData();

	void clear();

	size_t get_sample_count() const;
	vector<double> get_samples(size_t start_sample, size_t end_sample) const;
	double get_sample(size_t pos) const;

	void push_sample(void *sample);
	void push_sample(void *sample,
		const sigrok::Quantity *sr_quantity, const sigrok::Unit *sr_unit);
	void push_interleaved_samples(float *samples,
		size_t sample_count, size_t stride,
		const sigrok::Quantity *sr_quantity, const sigrok::Unit *sr_unit);

	void set_fixed_quantity(bool fixed);
	void set_quantity(const sigrok::Quantity *sr_quantity);
	void set_unit(const sigrok::Unit *sr_unit);

	const QString quantity() const;
	const QString unit() const;
	double last_value() const;
	double min_value() const;
	double max_value() const;

private:
	shared_ptr<vector<double>> data_;
	size_t sample_count_;

	bool quantity_fixed_;
	const sigrok::Quantity *sr_quantity_;
	const sigrok::Unit *sr_unit_;
	QString quantity_;
	QString unit_;

	double last_value_;
	double min_value_;
	double max_value_;

Q_SIGNALS:
	void quantity_changed(QString);
	void unit_changed(QString);
	void samples_cleared();

};

} // namespace data
} // namespace sv

#endif // DATA_ANALOGDATA_HPP
