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

const float ScopeCurveData::EnvelopeThreshold = 64.0f;

ScopeCurveData::ScopeCurveData(shared_ptr<sv::data::AnalogSegment> segment) :
	BaseCurveData(CurveDataType::ScopeCurveData), // TODO: Do we need the type here? Move to (Scope)Curve?
	segment_(segment),
	actual_segment_id_(0),
	start_sample_(0),
	end_sample_(0),
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

	qWarning() << "ScopeCurveData::setRectOfInterest(): ================================================= ";
	qWarning() << "ScopeCurveData::setRectOfInterest(): rect = " << rect;
	rect_of_interest_ = rect;

	if (rect_of_interest_.left() <= .0) {
		start_sample_ = 0;
	}
	else
		start_sample_ = std::floor(rect_of_interest_.left() / segment_->time_stride());

	if (rect_of_interest_.right() <= .0)
		end_sample_ = 0;
	else
		end_sample_ = std::ceil(rect_of_interest_.right() / segment_->time_stride());

	qWarning() << "ScopeCurveData::setRectOfInterest(): start_sample_ = " << start_sample_;
	qWarning() << "ScopeCurveData::setRectOfInterest(): end_sample_ = " << end_sample_;
	qWarning() << "ScopeCurveData::setRectOfInterest(): end_sample_ (ts) = " << end_sample_ * segment_->time_stride();


	const size_t sample_cnt = segment_->sample_count();
	//const size_t end_sample = end_sample_ <= sample_cnt ? end_sample_ : sample_cnt;
	//const size_t start_sample = start_sample_ <= end_sample ? start_sample_ : end_sample;
	end_sample_ = end_sample_ <= sample_cnt ? end_sample_ : sample_cnt;
	start_sample_ = start_sample_ <= end_sample_ ? start_sample_ : end_sample_;

	qWarning() << "ScopeCurveData::setRectOfInterest(): sample_cnt = " << sample_cnt;
	qWarning() << "ScopeCurveData::setRectOfInterest(): start_sample_ = " << start_sample_;
	qWarning() << "ScopeCurveData::setRectOfInterest(): end_sample_ = " << end_sample_;
	qWarning() << "ScopeCurveData::setRectOfInterest(): end_sample_ (ts) = " << end_sample_ * segment_->time_stride();

	// Calculate the samples per pixel for the rect
	// Number of possible samples for the visible time span of the rect
	const size_t rect_sample_cnt =
		std::ceil(rect_of_interest_.right() / segment_->time_stride()) -
		std::floor(rect_of_interest_.left() / segment_->time_stride());
	const size_t rect_pixel_cnt = x_scale_map_.pDist();
	samples_per_pixel_ = rect_sample_cnt / (double)rect_pixel_cnt;

	qWarning() << "ScopeCurveData::setRectOfInterest(): rect_sample_cnt = " << rect_sample_cnt;
	qWarning() << "ScopeCurveData::setRectOfInterest(): rect_pixel_cnt = " << rect_pixel_cnt;
	qWarning() << "ScopeCurveData::setRectOfInterest(): samples_per_pixel_ = " << samples_per_pixel_;

	segment_->get_envelope_section(envelope_section_, start_sample_, end_sample_, samples_per_pixel_);

	qWarning() << "ScopeCurveData::setRectOfInterest(): envelope_section_.scale = " << envelope_section_.scale;
	qWarning() << "ScopeCurveData::setRectOfInterest(): envelope_section_.start = " << envelope_section_.start;
	qWarning() << "ScopeCurveData::setRectOfInterest(): envelope_section_.length = " << envelope_section_.length;
	qWarning() << "ScopeCurveData::setRectOfInterest(): envelope_section_.time_stride = " << envelope_section_.time_stride;

	// Set invalide bounding rect
	//bounding_rect_ = QRectF(0.0, 0.0, -1.0, -1.0);
}

QPointF ScopeCurveData::sample(size_t i) const
{
	/*
	if (resampled_data_.empty()) {
		//qWarning() << "ScopeCurveData::sample(): sample_from_signal(" << i << ")";
		return sample_from_signal(i);
	}
	else {
		//qWarning() << "ScopeCurveData::sample(): resampled_data_.at(" << i << ")";
		if (i < resampled_data_.size())
			return resampled_data_.at(i);
		else {
			qWarning() << "ScopeCurveData::sample(): resampled_data_.at(" << i << ")";
			return QPointF(1, 1);
		}
	}
	*/

	QPointF sample;

	if (samples_per_pixel_ < EnvelopeThreshold)
		sample = sample_from_signal(i);
	else {
		if (i % 2 == 0) {
			size_t index = i / 2;
			sample = QPointF(
				envelope_section_.time_stride * (index + envelope_section_.start),
				(envelope_section_.samples+index)->max);
		}
		else {
			size_t index = (i - 1) / 2;
			sample = QPointF(
				envelope_section_.time_stride * (index + envelope_section_.start),
				(envelope_section_.samples+index)->min);
		}
	}

	//qWarning() << "ScopeCurveData::sample(): " << i << " -> "  << sample;

	return sample;
}

size_t ScopeCurveData::size() const
{
	/*
	if (resampled_data_.empty()) {
		size_t size = size_from_signal();
		qWarning() << "ScopeCurveData::size(): size_from_signal = " << size;
		//return size_from_signal();
		return size;
	}
	else {
		size_t size = resampled_data_.size();
		qWarning() << "ScopeCurveData::size(): resampled_data_.size() = " << size;
		//return resampled_data_.size();
		return size;
	}
	*/

	if (samples_per_pixel_ < EnvelopeThreshold)
		return size_from_signal();
	else
		return envelope_section_.length * 2;
}

QRectF ScopeCurveData::boundingRect() const
{
	const size_t total_sample_cnt = end_sample_ - start_sample_;
	// NOTE: Mapping from 0/total_sample_cnt to index is done in sample_from_signal()
	double start_sample_ts = sample_from_signal(0).x();
	double end_sample_ts = sample_from_signal(total_sample_cnt).x();

	// TODO: min/max for specific span
	QRectF br = QRectF(
		QPointF(start_sample_ts, segment_->max_value()),
		QPointF(end_sample_ts, segment_->min_value()));

	return br;

	/* FAST min/max
	// Check for a valid rect
	//qWarning() << "ScopeCurveData::boundingRect(): bounding_rect_.isValid() = " << bounding_rect_.isValid();
	//if (bounding_rect_.isValid())
	//	return bounding_rect_;
	// Not enough sample to create a rect
	if (size() <= 1)
		return QRectF(1.0, 1.0, -2.0, -2.0);

	// Calculate bounding rect

	double min = .0;
	double max = .0;

	if (sample(0).y() < sample(1).y()) {
		min = sample(0).y();
		max = sample(1).y();
	}
	else {
		min = sample(1).y();
		max = sample(0).y();
	}

	//compare pairs
	size_t size = this->size();
	for (size_t i=2; i<size-2; i+=2) {
		if (sample(i).y() > sample(i+1).y()) {
			if (sample(i).y() > max)
				max = sample(i).y();
			if (sample(i+1).y() < min)
				min = sample(i+1).y();
		}
		else {
			if (sample(i+1).y() > max)
				max = sample(i+1).y();
			if (sample(i).y() < min)
				min = sample(i).y();
		}
	}

	bounding_rect_ = QRectF(0.0, min, size * dt(), max - min);
	qWarning() << "ScopeCurveData::boundingRect(): bounding_rect_ = " << bounding_rect_;
	return bounding_rect_;
	*/



	/* LAST, WORKING
	// Not enough sample to create a rect
	if (size_from_signal() <= 1)
		return QRectF(0.0, 0.0, -1.0, -1.0);

	size_t end_sample = start_sample_ + size_from_signal();
	//qWarning() << "ScopeCurveData::boundingRect(): end_sample = " << end_sample;
	double min = sample_from_signal(start_sample_).y();
	double max = sample_from_signal(start_sample_).y();
	for (size_t i=start_sample_; i<end_sample; i++) {
		if (sample_from_signal(i).y() > max)
			max = sample_from_signal(i).y();
		if (sample_from_signal(i).y() < min)
			min = sample_from_signal(i).y();
	}

	bounding_rect_ = QRectF(
		QPointF(sample_from_signal(start_sample_).x(), max),
		QPointF(sample_from_signal(end_sample).x(), min));

	//qWarning() << "ScopeCurveData::boundingRect(): bounding_rect_ = " << bounding_rect_;
	return bounding_rect_;
	*/
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

void ScopeCurveData::set_x_scale_map(const QwtScaleMap &x_scale_map)
{
	x_scale_map_ = x_scale_map;
}

void ScopeCurveData::set_y_scale_map(const QwtScaleMap &y_scale_map)
{
	y_scale_map_ = y_scale_map;
}

void ScopeCurveData::set_scale_maps(const QwtScaleMap &x_scale_map,
	const QwtScaleMap &y_scale_map)
{
	x_scale_map_ = x_scale_map;
	y_scale_map_ = y_scale_map;
	//qWarning() << "ScopeCurveData::set_scale_maps(): x_scale_map = " << x_scale_map_.p1() << ", " << x_scale_map_.p2();
	//qWarning() << "ScopeCurveData::set_scale_maps(): x_scale_map = " << x_scale_map_.s1() << ", " << x_scale_map_.s2();
	//qWarning() << "ScopeCurveData::set_scale_maps(): x_scale_map = " << x_scale_map_.pDist() << ", " << x_scale_map_.sDist();
	//qWarning() << "ScopeCurveData::set_scale_maps(): y_scale_map = " << y_scale_map_.p1() << ", " << y_scale_map_.p2();
	//qWarning() << "ScopeCurveData::set_scale_maps(): y_scale_map = " << y_scale_map_.s1() << ", " << y_scale_map_.s2();
	//qWarning() << "ScopeCurveData::set_scale_maps(): y_scale_map = " << y_scale_map.pDist() << ", " << y_scale_map.sDist();

	//resample();

	if (x_scale_map_.s1() <= .0) {
		start_sample_ = 0;
	}
	else
		start_sample_ = std::floor(x_scale_map_.s1() / segment_->time_stride());

	if (x_scale_map_.s2() <= .0)
		end_sample_ = 0;
	else
		end_sample_ = std::ceil(x_scale_map_.s2() / segment_->time_stride());

	qWarning() << "ScopeCurveData::set_scale_maps(): start_sample_ = " << start_sample_;
	qWarning() << "ScopeCurveData::set_scale_maps(): end_sample_ = " << end_sample_;

	const size_t sample_cnt = segment_->sample_count();
	end_sample_ = end_sample_ <= sample_cnt ? end_sample_ : sample_cnt;
	start_sample_ = start_sample_ <= end_sample_ ? start_sample_ : end_sample_;

	qWarning() << "ScopeCurveData::set_scale_maps(): sample_cnt = " << sample_cnt;
	qWarning() << "ScopeCurveData::set_scale_maps(): start_sample_ = " << start_sample_;
	qWarning() << "ScopeCurveData::set_scale_maps(): end_sample_ = " << end_sample_;

	// Calculate the samples per pixel for the rect
	// Number of possible samples for the visible time span of the rect
	const size_t rect_sample_cnt =
		std::ceil(x_scale_map_.s2() / segment_->time_stride()) -
		std::floor(x_scale_map_.s1() / segment_->time_stride());
	const size_t rect_pixel_cnt = x_scale_map_.pDist();
	samples_per_pixel_ = rect_sample_cnt / (double)rect_pixel_cnt;

	qWarning() << "ScopeCurveData::set_scale_maps(): rect_sample_cnt = " << rect_sample_cnt;
	qWarning() << "ScopeCurveData::set_scale_maps(): rect_pixel_cnt = " << rect_pixel_cnt;
	qWarning() << "ScopeCurveData::set_scale_maps(): samples_per_pixel_ = " << samples_per_pixel_;

	if (end_sample_ > 0)
		segment_->get_envelope_section(envelope_section_, start_sample_, end_sample_, samples_per_pixel_);

	qWarning() << "ScopeCurveData::set_scale_maps(): envelope_section_.scale = " << envelope_section_.scale;
	qWarning() << "ScopeCurveData::set_scale_maps(): envelope_section_.start = " << envelope_section_.start;
	qWarning() << "ScopeCurveData::set_scale_maps(): envelope_section_.length = " << envelope_section_.length;
	qWarning() << "ScopeCurveData::set_scale_maps(): envelope_section_.time_stride = " << envelope_section_.time_stride;
}

QPointF ScopeCurveData::sample_from_signal(size_t i) const
{
	//signal_data_->lock();

	size_t index = start_sample_ + i;

	//qWarning() << "ScopeCurveData::sample_from_signal(): i = " << i;
	//qWarning() << "ScopeCurveData::sample_from_signal(): index = " << index;

	QPointF sample_point(segment_->time_stride() * index, segment_->get_sample(index));

	//qWarning() << "ScopeCurveData::sample_from_signal(): " << i << " = " << sample_point
	//	<< " (time_stride = " << QString("%1").arg(segment_->time_stride(), 10, 'f', -1, '0') << ")";

	//signal_data_->.unlock();

	return sample_point;
}

size_t ScopeCurveData::size_from_signal() const
{
	// TODO: Synchronize x/y sample data
	// TODO: MinMax-Resample

	size_t sample_count = segment_->sample_count();

	size_t size = 0;
	if (sample_count < start_sample_)
		size = 0;
	else if (sample_count > end_sample_)
		size = end_sample_ - start_sample_;
	else
		size = sample_count - start_sample_;

	qWarning() << "ScopeCurveData::size_from_signal(): size = " << size;
	return size;
}

/*
void ScopeCurveData::resample()
{
	unsigned int prevIndex = 0;
	unsigned int currIndex = 0;
	unsigned int xIndex = 0;
	//unsigned int sIndex = 0;
	float min, max;

	//qWarning() << "ScopeCurveData::resample(): points.size() = " << size();

	// Get a pointer to the raw point data.
	//const QPointF *p = points.data();

	// Determine the number of plot pixels. We do this by looking to see what
	// the last x coordinate is in the points vector. As this has been
	// transformed from sample coordinates to screen coordinates this is also
	// the number of plot pixels, though there may be a better way to do this.
	//size_t numPixels = (size_t)((sample_from_signal(size() - 1).x() - sample_from_signal(0).x()) + 0.5F);
	//qWarning() << "ScopeCurveData::resample(): p[size() - 1].x() = " << sample_from_signal(size() - 1).x();
	//qWarning() << "ScopeCurveData::resample(): p[0].x() = " << sample_from_signal(0).x();
	//qWarning() << "ScopeCurveData::resample(): numPixels = " << numPixels;
	size_t sample_cnt = size_from_signal();
	size_t pixels_cnt = x_scale_map_.pDist();

	// Calculate the number of samples per pixel. This assumes equidistant
	// data points along the x-axis.
	double samples_per_pixel = sample_cnt / (double)pixels_cnt;
	//qWarning() << "ScopeCurveData::resample(): samples_per_pixel = " << samples_per_pixel;

	// Set the previous transformed sample index.
	prevIndex = 0;

	double samples_per_pixel_threshold = 2;

	// Only weed out points when we exceed the points per pixel threshold.
	if (samples_per_pixel > samples_per_pixel_threshold) {
		// Construct a vector to hold the stripped point values. As we only plot
		// one vertical line per plot pixel we only need twice the number of pixels.
		//QPolygonF stripped(pixels_cnt * 2);
		resampled_data_.clear();
		//resampled_data_.resize(pixels_cnt * 2);

		// Loop over each plot pixel and create the polygon to plot for that
		// pixel.
		for (xIndex = 1; xIndex <= pixels_cnt; xIndex++) {
			// Calculate the current sample index for the plot pixel.
			currIndex = (unsigned int) (((double) xIndex * samples_per_pixel) + 0.5);
			if (currIndex > (unsigned int) (sample_cnt - 1))
				currIndex = sample_cnt - 1;

			// Initialise the min and max values.
			min = numeric_limits<float>::max();
			max = numeric_limits<float>::min();

			// Loop over the range of samples represented by one plot pixel and
			// calculate the min/max y value.
			//qWarning() << "ScopeCurveData::resample(): Loop Range: prevIndex < currIndex = " << prevIndex << " < " << currIndex ;
			for (unsigned int x = prevIndex; x < currIndex; x++) {
				QPointF sample = sample_from_signal(x);
				//qWarning() << "ScopeCurveData::resample():     x < currIndex = " << x << " < " << currIndex;
				//qWarning() << "ScopeCurveData::resample():     sample(x).y() = " << sample(x).y();
				min = ((min > sample.y()) ? sample.y() : min);
				max = ((max < sample.y()) ? sample.y() : max);
			}

			// Draw the vertical line for the min/max value at this plot pixel.
			QPointF prev_sample = sample_from_signal(prevIndex);
			//qWarning() << "ScopeCurveData::resample(): MIN " << min << " @sIndex " << sIndex << ", @x " << prev_sample.x();
			resampled_data_.push_back(QPointF(prev_sample.x(), min));
			//qWarning() << "ScopeCurveData::resample(): MAX " << max << " @sIndex " << sIndex << ", @x " << prev_sample.x();
			resampled_data_.push_back(QPointF(prev_sample.x(), max));

			// Move the range we min/max over for next time.
			prevIndex = currIndex;
		}

		// Adjust the stripped vector to the correct size if its not already.
		//stripped.resize(sIndex);

		//qWarning() << "ScopeCurveData::resample(): resampled_data_.size() = " << resampled_data_.size();

		// Return the stripped points.
		//return stripped;
	}
	else {
		resampled_data_.clear();
		resampled_data_.resize(0);
	}

	// No stripping to be done, return original points.
	//return points;
}
*/

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

shared_ptr<sv::data::AnalogSegment> ScopeCurveData::segment() const
{
	return segment_;
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
