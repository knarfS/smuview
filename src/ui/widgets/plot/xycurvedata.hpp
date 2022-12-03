/*
 * This file is part of the SmuView project.
 *
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

#ifndef UI_WIDGETS_PLOT_XYCURVEDATA_HPP
#define UI_WIDGETS_PLOT_XYCURVEDATA_HPP

#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include <QPointF>
#include <QRectF>
#include <QSettings>
#include <QString>

#include "src/data/datautil.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"

using std::mutex;
using std::set;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sv {

class Session;

namespace data {
class AnalogTimeSignal;
}
namespace devices {
class BaseDevice;
}

namespace ui {
namespace widgets {
namespace plot {

/*
 * NOTE: XYCurveData must also inherit QObject (Important: first QObject,
 *       then BaseCurvedata), to get signals/slots working!
 */
class XYCurveData : public BaseCurveData
{
	Q_OBJECT

public:
	XYCurveData(shared_ptr<sv::data::AnalogTimeSignal> x_t_signal,
		shared_ptr<sv::data::AnalogTimeSignal> y_t_signal);

	bool is_equal(const BaseCurveData *other) const override;

	QPointF sample(size_t index) const override;
	size_t size() const override;
	QRectF boundingRect() const override;

	QPointF closest_point(const QPointF &pos, double *dist) const override;
	QString name() const override;
	string id_prefix() const override;
	sv::data::Quantity x_quantity() const override;
	set<sv::data::QuantityFlag> x_quantity_flags() const override;
	sv::data::Unit x_unit() const override;
	QString x_unit_str() const override;
	QString x_title() const override;
	sv::data::Quantity y_quantity() const override;
	set<sv::data::QuantityFlag> y_quantity_flags() const override;
	sv::data::Unit y_unit() const override;
	QString y_unit_str() const override;
	QString y_title() const override;

	shared_ptr<sv::data::AnalogTimeSignal> x_t_signal() const;
	shared_ptr<sv::data::AnalogTimeSignal> y_t_signal() const;

	void save_settings(QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device) const override;
	static XYCurveData *init_from_settings(
		Session &session, QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device);

private:
	shared_ptr<sv::data::AnalogTimeSignal> x_t_signal_;
	shared_ptr<sv::data::AnalogTimeSignal> y_t_signal_;
	size_t x_t_signal_pos_;
	size_t y_t_signal_pos_;
	// TODO: use some sort of AnalogSignal instead of 2 vectors?
	shared_ptr<vector<double>> x_data_;
	shared_ptr<vector<double>> y_data_;
	mutex sample_append_mutex_;

private Q_SLOTS:
	void on_sample_appended();

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_XYCURVEDATA_HPP
