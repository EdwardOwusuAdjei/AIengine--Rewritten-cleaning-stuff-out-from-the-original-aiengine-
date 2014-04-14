/*
 * AIEngine a deep packet inspector reverse engineering engine.
 *
 * Copyright (C) 2013  Luis Campo Giralte
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 * Written by Luis Campo Giralte <luis.camp0.2009@gmail.com> 2013
 *
 */
#ifndef SRC_IPSET_IPSET_H_
#define SRC_IPSET_IPSET_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <memory>
#include <string>
#include <iostream>
#include <unordered_set>

#ifdef PYTHON_BINDING
#include <boost/python.hpp>
#include <boost/function.hpp>
#endif

namespace aiengine {

class IPSet 
{
public:
    	explicit IPSet():total_ips_(0),
		total_ips_not_on_set_(0),total_ips_on_set_(0)
		{
			name_="Generic IPSet";
#ifdef PYTHON_BINDING
			callback_set_ = false;
			callback_ = nullptr;
#endif
		}
    	explicit IPSet(const std::string &name):name_(name),
		total_ips_not_on_set_(0),total_ips_on_set_(0),
		total_ips_(0) 
		{
#ifdef PYTHON_BINDING
			callback_set_ = false;
			callback_ = nullptr;
#endif
		}
    	virtual ~IPSet() {}

	const char *getName() { return name_.c_str();}

	void addIPAddress(const std::string &ip);
	bool lookupIPAddress(const std::string &ip); 

	void statistics(std::basic_ostream<char>& out);
	void statistics() { statistics(std::cout);}

#ifdef PYTHON_BINDING

        bool haveCallback() const { return callback_set_;}

        void setCallback(PyObject *callback) {

                // TODO: Verify that the callback have at least one parameter
                if (!PyCallable_Check(callback)) {
                        std::cerr << "Object is not callable." << std::endl;
                } else {
                        if ( callback_ ) Py_XDECREF(callback_);
                        callback_ = callback;
                        Py_XINCREF(callback_);
                        callback_set_ = true;
                }
        }

        PyObject *getCallback() { return callback_;}

#endif

	int32_t getTotalIPs() const { return total_ips_; }
	int32_t getTotalLookups() const { return (total_ips_on_set_ + total_ips_not_on_set_); }
	int32_t getTotalLookupsIn() const { return total_ips_on_set_; }
	int32_t getTotalLookupsOut() const { return total_ips_not_on_set_; }
	int getSize() const { return map_.size(); }

private:
	std::string name_;	
	int32_t total_ips_;
	int32_t total_ips_not_on_set_;
	int32_t total_ips_on_set_;
	std::unordered_set<std::string> map_;
#ifdef PYTHON_BINDING
	bool callback_set_;
	PyObject *callback_;
#endif
};

typedef std::shared_ptr<IPSet> IPSetPtr;
typedef std::weak_ptr<IPSet> IPSetPtrWeak;

} // namespace aiengine

#endif  // SRC_IPSET_IPSET_H_