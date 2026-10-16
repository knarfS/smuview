/*
 * This file is part of the SmuView project.
 *
 * Copyright (C) 2026 Frank Stettner <frank-stettner@gmx.net>
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

#include <string>
#include <boost/test/unit_test.hpp>

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

#include "src/python/bindingsenums.hpp"
#include "src/data/datautil.hpp"
#include "src/devices/deviceutil.hpp"

namespace py = pybind11;

namespace {

py::module_ *test_module_ = nullptr;

/**
 * Only one Python interpreter can be initialized per process, this starts the
 * embedded interpreter and binds the python enums as a global fixture.
 */
struct PythonFixture
{
	py::scoped_interpreter interpreter;
	py::module_ test_module;

	PythonFixture() : test_module(py::module_::import("types")
		.attr("ModuleType")("test_smuview").cast<py::module_>())
	{
		test_module.attr("__pdoc__") = py::dict();
		init_Enums(test_module);
		test_module_ = &test_module;
	}

	~PythonFixture()
	{
		test_module_ = nullptr;
	}
};

} // namespace

BOOST_GLOBAL_FIXTURE(PythonFixture);


BOOST_AUTO_TEST_SUITE(PythonBindingsTest)

BOOST_AUTO_TEST_CASE(data_types_are_bound_test)
{
	BOOST_REQUIRE(test_module_ != nullptr);

	const py::object py_data_type = test_module_->attr("DataType");
	const py::dict members = py_data_type.attr("__members__");

	for (const auto &entry : sv::data::datautil::get_data_type_map()) {
		std::string name = entry.name.toStdString();
		BOOST_TEST_CONTEXT(name) {
			const int expected = static_cast<int>(entry.data_type);
			bool found = false;
			for (const auto &item : members) {
				const py::handle value = item.second;
				if (py::cast<int>(value) == expected) {
					found = true;
					break;
				}
			}

			BOOST_CHECK_MESSAGE(found,
				"No Python DataType member has C++ enum value " << name << " ("
				<< expected << ")");
		}
	}
}

BOOST_AUTO_TEST_CASE(config_keys_are_bound_test)
{
	BOOST_REQUIRE(test_module_ != nullptr);

	const py::object py_config_key = test_module_->attr("ConfigKey");
	const py::dict members = py_config_key.attr("__members__");

	for (const auto &entry : sv::devices::deviceutil::get_config_key_map()) {
		std::string name = entry.name.toStdString();
		BOOST_TEST_CONTEXT(name) {
			const int expected = static_cast<int>(entry.config_key);
			bool found = false;
			for (const auto &item : members) {
				const py::handle value = item.second;
				if (py::cast<int>(value) == expected) {
					found = true;
					break;
				}
			}

			BOOST_CHECK_MESSAGE(found,
				"No Python ConfigKey member has C++ enum value " << name << " ("
				<< expected << ")");
		}
	}
}

BOOST_AUTO_TEST_CASE(quantities_are_bound_test)
{
	BOOST_REQUIRE(test_module_ != nullptr);

	const py::object py_quantity = test_module_->attr("Quantity");
	const py::dict members = py_quantity.attr("__members__");

	for (const auto &entry : sv::data::datautil::get_quantity_map()) {
		std::string name = entry.name.toStdString();
		BOOST_TEST_CONTEXT(name) {
			const int expected = static_cast<int>(entry.quantity);
			bool found = false;
			for (const auto &item : members) {
				const py::handle value = item.second;
				if (py::cast<int>(value) == expected) {
					found = true;
					break;
				}
			}

			BOOST_CHECK_MESSAGE(found,
				"No Python Quantity member has C++ enum value " << name << " ("
				<< expected << ")");
		}
	}
}

BOOST_AUTO_TEST_CASE(quantitie_flags_are_bound_test)
{
	BOOST_REQUIRE(test_module_ != nullptr);

	const py::object py_quantity_flag = test_module_->attr("QuantityFlag");
	const py::dict members = py_quantity_flag.attr("__members__");

	for (const auto &entry : sv::data::datautil::get_quantity_flag_map()) {
		std::string name = entry.name.toStdString();
		BOOST_TEST_CONTEXT(name) {
			const int expected = static_cast<int>(entry.quantity_flag);
			bool found = false;
			for (const auto &item : members) {
				const py::handle value = item.second;
				if (py::cast<int>(value) == expected) {
					found = true;
					break;
				}
			}
			BOOST_CHECK_MESSAGE(found,
				"No Python QuantityFlag member has C++ enum value " << name
				<< " (" << expected << ")");
		}
	}
}

BOOST_AUTO_TEST_CASE(units_are_bound_test)
{
	BOOST_REQUIRE(test_module_ != nullptr);

	const py::object py_unit = test_module_->attr("Unit");
	const py::dict members = py_unit.attr("__members__");

	for (const auto &entry : sv::data::datautil::get_unit_map()) {
		std::string name = entry.name.toStdString();
		BOOST_TEST_CONTEXT(name) {
			const int expected = static_cast<int>(entry.unit);
			bool found = false;
			for (const auto &item : members) {
				const py::handle value = item.second;
				if (py::cast<int>(value) == expected) {
					found = true;
					break;
				}
			}

			BOOST_CHECK_MESSAGE(found,
				"No Python Unit member has C++ enum value " << name << " ("
				<< expected << ")");
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()
