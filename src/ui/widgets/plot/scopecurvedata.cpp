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

#include <cmath>
#include <limits>
#include <memory>
#include <set>

#include <QDebug>
#include <QPointF>
#include <QRectF>
#include <QSettings>
#include <QString>

#include "scopecurvedata.hpp"
#include "src/session.hpp"
#include "src/settingsmanager.hpp"
#include "src/data/analogscopesignal.hpp"
#include "src/data/analogsegment.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/basedevice.hpp"
#include "src/ui/widgets/plot/basecurvedata.hpp"

using std::dynamic_pointer_cast;
using std::numeric_limits;
using std::set;
using std::shared_ptr;

namespace sv {
namespace ui {
namespace widgets {
namespace plot {

const float ScopeCurveData::EnvelopeThreshold = 64.0f; // TODO: Rename. Für was war das ursprünglich?? Wert zu hoch?

ScopeCurveData::ScopeCurveData(shared_ptr<sv::data::AnalogSegment> segment) :
	BaseCurveData(CurveDataType::ScopeCurveData), // TODO: Do we need the type here? Move to (Scope)Curve?
	segment_(segment),
	actual_segment_id_(0),
	scale_start_sample_(0),
	scale_end_sample_(0),
	samples_per_pixel_(.0)
{
}

bool ScopeCurveData::is_equal(const BaseCurveData *other) const
{
	const ScopeCurveData *scd = dynamic_cast<const ScopeCurveData *>(other);
	if (scd == nullptr)
		return false;

	return segment_ == scd->segment();
}

void ScopeCurveData::setRectOfInterest(const QRectF &rect)
{
	/*
	 * NOTE: Good for zooming in, but NFG for level of detail when having a huge
	 * number of sample points, b/c there is no painting rect
	 */
	if (rect_of_interest_ == rect)
		return;

	//qWarning() << "ScopeCurveData::setRectOfInterest(): ================================================= ";
	//qWarning() << "ScopeCurveData::setRectOfInterest(): rect = " << rect;
	rect_of_interest_ = rect;

	if (rect_of_interest_.left() <= .0)
		scale_start_sample_ = 0;
	else
		scale_start_sample_ = std::floor(rect_of_interest_.left() / segment_->time_stride());

	if (rect_of_interest_.right() <= .0)
		scale_end_sample_ = 0;
	else
		scale_end_sample_ = std::ceil(rect_of_interest_.right() / segment_->time_stride());

	//qWarning() << "ScopeCurveData::setRectOfInterest(): scale_start_sample_ = " << scale_start_sample_;
	//qWarning() << "ScopeCurveData::setRectOfInterest(): scale_end_sample_ = " << scale_end_sample_;
	//qWarning() << "ScopeCurveData::setRectOfInterest(): scale_end_sample_ (ts) = " << scale_end_sample_ * segment_->time_stride();

	//const size_t sample_cnt = segment_->sample_count();
	scale_start_sample_ = scale_start_sample_ <= scale_end_sample_ ? scale_start_sample_ : scale_end_sample_;

// 	qWarning() << "ScopeCurveData::setRectOfInterest(): sample_cnt = " << sample_cnt;
// 	qWarning() << "ScopeCurveData::setRectOfInterest(): scale_start_sample_ = " << scale_start_sample_;
// 	qWarning() << "ScopeCurveData::setRectOfInterest(): scale_end_sample_ = " << scale_end_sample_;
// 	qWarning() << "ScopeCurveData::setRectOfInterest(): scale_end_sample_ (ts) = " << scale_end_sample_ * segment_->time_stride();
//
// 	// Calculate the samples per pixel for the rect
// 	// Number of possible samples for the visible time span of the rect
// 	const size_t rect_sample_cnt =
// 		std::ceil(rect_of_interest_.right() / segment_->time_stride()) -
// 		std::floor(rect_of_interest_.left() / segment_->time_stride());
// 	const size_t rect_pixel_cnt = x_scale_map_.pDist();
// 	samples_per_pixel_ = rect_sample_cnt / (double)rect_pixel_cnt;
//
// 	qWarning() << "ScopeCurveData::setRectOfInterest(): rect_sample_cnt = " << rect_sample_cnt;
// 	qWarning() << "ScopeCurveData::setRectOfInterest(): rect_pixel_cnt = " << rect_pixel_cnt;
// 	qWarning() << "ScopeCurveData::setRectOfInterest(): samples_per_pixel_ = " << samples_per_pixel_;
//
// 	segment_->get_envelope_section(envelope_section_, scale_start_sample_, scale_end_sample_, samples_per_pixel_);
//
// 	qWarning() << "ScopeCurveData::setRectOfInterest(): envelope_section_.scale = " << envelope_section_.scale;
// 	qWarning() << "ScopeCurveData::setRectOfInterest(): envelope_section_.start = " << envelope_section_.start;
// 	qWarning() << "ScopeCurveData::setRectOfInterest(): envelope_section_.length = " << envelope_section_.length;
// 	qWarning() << "ScopeCurveData::setRectOfInterest(): envelope_section_.time_stride = " << envelope_section_.time_stride;
//
// 	// Set invalide bounding rect
// 	//bounding_rect_ = QRectF(0.0, 0.0, -1.0, -1.0);
}

void ScopeCurveData::update_scale_maps(const QwtScaleMap &x_scale_map,
	const QwtScaleMap &y_scale_map)
{
	x_scale_map_ = x_scale_map;
	y_scale_map_ = y_scale_map;

	if (x_scale_map_.s1() <= .0)
		scale_start_sample_ = 0;
	else
		scale_start_sample_ = std::floor(x_scale_map_.s1() / segment_->time_stride());

	if (x_scale_map_.s2() <= .0)
		scale_end_sample_ = 0;
	else
		scale_end_sample_ = std::ceil(x_scale_map_.s2() / segment_->time_stride());

	//qWarning() << "ScopeCurveData::update_scale_maps(): scale_start_sample_ = " << scale_start_sample_;
	//qWarning() << "ScopeCurveData::update_scale_maps(): scale_end_sample_ = " << scale_end_sample_;

	//const size_t sample_cnt = segment_->sample_count();
	scale_start_sample_ = scale_start_sample_ <= scale_end_sample_ ? scale_start_sample_ : scale_end_sample_;

	//qWarning() << "ScopeCurveData::update_scale_maps(): sample_cnt = " << sample_cnt;
	//qWarning() << "ScopeCurveData::update_scale_maps(): scale_start_sample_ = " << scale_start_sample_;

	// Calculate the samples per pixel for the rect
	// Number of possible samples for the visible time span of the rect
	const size_t rect_sample_cnt =
		std::ceil(x_scale_map_.s2() / segment_->time_stride()) -
		std::floor(x_scale_map_.s1() / segment_->time_stride());
	const size_t rect_pixel_cnt = x_scale_map_.pDist();
	samples_per_pixel_ = rect_sample_cnt / (double)rect_pixel_cnt;

	//qWarning() << "ScopeCurveData::update_scale_maps(): rect_sample_cnt = " << rect_sample_cnt;
	//qWarning() << "ScopeCurveData::update_scale_maps(): rect_pixel_cnt = " << rect_pixel_cnt;
	//qWarning() << "ScopeCurveData::update_scale_maps(): samples_per_pixel_ = " << samples_per_pixel_;
	//qWarning() << "ScopeCurveData::update_scale_maps(): rect time span = " << rect_sample_cnt * segment_->time_stride();

	if (scale_end_sample_ > 0)
		segment_->get_envelope(envelope_, samples_per_pixel_);
}

QPointF ScopeCurveData::sample(size_t scale_pos) const
{
	if (samples_per_pixel_ < EnvelopeThreshold)
		return sample_from_signal(scale_pos);

	size_t scale_start_sample_scaled = scale_start_sample_ >> envelope_.scale_power;
	if (scale_pos % 2 == 0) {
		size_t index = (scale_pos / 2) + scale_start_sample_scaled;
		return QPointF(
			envelope_.time_stride * index,
			(envelope_.samples + index)->max);
	}
	else {
		size_t index = ((scale_pos - 1) / 2) + scale_start_sample_scaled;
		return QPointF(
			envelope_.time_stride * index,
			(envelope_.samples + index)->min);
	}
}

size_t ScopeCurveData::size() const
{
	size_t segment_size = size_from_signal();
	if (samples_per_pixel_ < EnvelopeThreshold)
		return segment_size;

	size_t scaled_size = segment_size >> envelope_.scale_power;
	return scaled_size * 2;
}

QRectF ScopeCurveData::boundingRect() const
{
	const size_t segment_size = size_from_signal();
	const double start_ts = sample_from_signal(0).x();
	const double end_ts = sample_from_signal(segment_size).x();

	double max_value = sample(0).y();
	double min_value = sample(1).y();
	const size_t scaled_size = size();
	for (size_t i = 2; i < scaled_size; i=i+2) {
		double max_value_tmp = sample(i).y();
		if (max_value_tmp > max_value)
			max_value = max_value_tmp;
		double min_value_tmp = sample(i+1).y();
		if (min_value_tmp < min_value)
			min_value = min_value_tmp;
	}

	QRectF br = QRectF(
		QPointF(start_ts, max_value),
		QPointF(end_ts, min_value));
	return br;
}

QPointF ScopeCurveData::closest_point(const QPointF &pos, double *dist) const
{
	(void)dist;
	const double x_value = pos.x();
	const int index_max = (int)size() - 1;

	// Corner cases
	if (index_max < 0)
		return QPointF(0, 0);
	if (x_value <= sample(0).x())
		return sample(0);
	if (x_value >= sample(index_max).x())
		return sample(index_max);

	size_t index_min = 0;
	size_t n = index_max;

	while (n > 0) {
		const size_t half = n >> 1;
		const size_t index_mid = index_min + half;

		if (x_value < sample(index_mid).x()) {
			n = half;
		}
		else {
			index_min = index_mid + 1;
			n -= half + 1;
		}
	}

	return sample(index_min);
}

shared_ptr<sv::data::AnalogSegment> ScopeCurveData::segment() const
{
	return segment_;
}

QPointF ScopeCurveData::sample_from_signal(size_t scale_pos) const
{
	size_t new_scale_pos = scale_pos + scale_start_sample_;
	return QPointF(
		segment_->time_stride() * new_scale_pos,
		segment_->get_sample(new_scale_pos));
}

size_t ScopeCurveData::size_from_signal() const
{
	size_t sample_count = segment_->sample_count();
	if (sample_count < scale_start_sample_)
		return 0;
	else if (sample_count > scale_end_sample_)
		return scale_end_sample_ - scale_start_sample_;
	else
		return sample_count - scale_start_sample_;
}

QString ScopeCurveData::name() const
{
	// TODO: Remove
	return "";
}

string ScopeCurveData::id_prefix() const
{
	// TODO: Remove
	return "";
}

sv::data::Quantity ScopeCurveData::x_quantity() const
{
	// TODO: remove
	return sv::data::Quantity::Unknown;
}

set<sv::data::QuantityFlag> ScopeCurveData::x_quantity_flags() const
{
	// TODO: remove
	return set<data::QuantityFlag>();
}

sv::data::Unit ScopeCurveData::x_unit() const
{
	// TODO: remove
	return sv::data::Unit::Unknown;
}

QString ScopeCurveData::x_unit_str() const
{
	// TODO: remove
	return "";
}

QString ScopeCurveData::x_title() const
{
	// TODO: remove
	return "";
}

sv::data::Quantity ScopeCurveData::y_quantity() const
{
	// TODO: remove
	return sv::data::Quantity::Unknown;
}

set<sv::data::QuantityFlag> ScopeCurveData::y_quantity_flags() const
{
	// TODO: remove
	return set<data::QuantityFlag>();
}

sv::data::Unit ScopeCurveData::y_unit() const
{
	// TODO: remove
	return sv::data::Unit::Unknown;
}

QString ScopeCurveData::y_unit_str() const
{
	// TODO: remove
	return "";
}

QString ScopeCurveData::y_title() const
{
	// TODO: remove
	return "";
}

void ScopeCurveData::save_settings(QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device) const
{
	// TODO: remove
	(void)settings;
	(void)origin_device;
}

ScopeCurveData *ScopeCurveData::init_from_settings(
	Session &session, QSettings &settings,
	shared_ptr<sv::devices::BaseDevice> origin_device)
{
	// TODO: remove
	(void)session;
	(void)settings;
	(void)origin_device;
	return nullptr;
}

} // namespace plot
} // namespace widgets
} // namespace ui
} // namespace sv
