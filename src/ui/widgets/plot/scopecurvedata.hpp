/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef UI_WIDGETS_PLOT_SCOPECURVEDATA_HPP
#define UI_WIDGETS_PLOT_SCOPECURVEDATA_HPP

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <QPointF>
#include <QRectF>
#include <QSettings>
#include <QString>
#include <qwt_scale_map.h>

#include "src/data/analogsegment.hpp"
#include "src/data/datautil.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"

using std::set;
using std::shared_ptr;
using std::string;
using std::vector;

namespace sv {

class Session;

namespace data {
class AnalogSegment;
}
namespace devices {
class BaseDevice;
}

namespace ui {
namespace widgets {
namespace plot {

class ScopeCurveData : public BaseCurveData
{
	Q_OBJECT

public:
	explicit ScopeCurveData(shared_ptr<sv::data::AnalogSegment> segment);

	bool is_equal(const BaseCurveData *other) const override;

	void setRectOfInterest(const QRectF &rect) override;

	/**
	 * @brief Return a sample (x and y) of the curve.
	 * The index starts at the visible part of the curve with 0 and ends at the
	 * visible part with ScopeCurveData::size().
	 *
	 * @param[in] i The position of the sample to return.
	 * @return The sample at position i.
	 */
	QPointF sample(size_t i) const override;

	/**
	 * @brief Return the size of the curve.
	 * Only the size of the visible part of the curve is returned.
	 *
	 * @return The size.
	 */
	size_t size() const override;

	/**
	 * @brief Return the bounding rectangle of the curve.
	 * The returned bounding rectangle only contains the visible part of the
	 * curve.
	 *
	 * @return The bounding rectangle.
	 */
	QRectF boundingRect() const override;

	size_t size_from_signal() const;
	QPointF closest_point(const QPointF &pos, double *dist) const override;
	/* TODO: Remove and move to (Scope)Curve! */
	QString name() const override;
	string id_prefix() const override;
	/* TODO end */
	void set_x_scale_map(const QwtScaleMap &x_scale_map);
	void set_y_scale_map(const QwtScaleMap &y_scale_map);
	void set_scale_maps(const QwtScaleMap &x_scale_map,
		const QwtScaleMap &y_scale_map);

	/* TODO: Remove and move to (Scope)Curve! */
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
	/* TODO end */

	shared_ptr<sv::data::AnalogSegment> segment() const;

	/* TODO: Remove and move to (Scope)Curve! */
	void save_settings(QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device) const override;
	static ScopeCurveData *init_from_settings(
		Session &session, QSettings &settings,
		shared_ptr<sv::devices::BaseDevice> origin_device);
	/* TODO end */

private:
	static const float EnvelopeThreshold;

	shared_ptr<sv::data::AnalogSegment> segment_;
	uint32_t actual_segment_id_;
	QRectF rect_of_interest_;
	size_t start_sample_;
	size_t end_sample_;
	mutable QRectF bounding_rect_; // Needs to be butable, b/c of "boundingRect() const"
	QwtScaleMap x_scale_map_;
	QwtScaleMap y_scale_map_;
	sv::data::AnalogSegment::EnvelopeSection envelope_section_;

	QPointF sample_from_signal(size_t i) const;

};

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv

#endif // UI_WIDGETS_PLOT_SCOPECURVEDATA_HPP
