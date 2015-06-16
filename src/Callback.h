/*
 * AIEngine a deep packet inspector reverse engineering engine.
 *
 * Copyright (C) 2013-2015  Luis Campo Giralte
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
 * Written by Luis Campo Giralte <luis.camp0.2009@gmail.com> 
 *
 */
#pragma once
#ifndef SRC_CALLBACK_H_
#define SRC_CALLBACK_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>

#ifdef PYTHON_BINDING
#include <boost/python.hpp>
#include <boost/function.hpp>
#elif defined(RUBY_BINDING)
#include <ruby.h>
#endif

namespace aiengine {

class Flow;
// http://www.lysator.liu.se/~norling/ruby_callbacks.html
class Callback 
{
#ifdef PYTHON_BINDING
public:
	Callback():callback_set_(false),callback_(nullptr) {}
	virtual ~Callback() {}

	bool haveCallback() const { return callback_set_;}

	void setCallback(PyObject *callback); 
	void executeCallback(Flow *flow);
	
	PyObject *getCallback() const { return callback_;}
	
private:
	bool callback_set_;
	PyObject *callback_;

#elif defined(RUBY_BINDING)

public:
	Callback():callback_set_(false),callback_(Qnil),memory_wrapper_(Qnil) {
		memory_wrapper_ = Data_Wrap_Struct(0 /* klass */, staticMark, NULL, static_cast<void*>(this));
		rb_gc_register_address(&memory_wrapper_);

	}
	virtual ~Callback() { rb_gc_unregister_address(&memory_wrapper_);}

	bool haveCallback() const { return callback_set_;}
	
	void setCallback(VALUE callback);
	void executeCallback(Flow *flow); 
	
protected:
	static void staticMark(Callback *me) { me->mark(); }

	void mark();
private:
	bool callback_set_;
	VALUE callback_;
	VALUE memory_wrapper_;
#endif
};

} // namespace aiengine

#endif  // SRC_CALLBACK_H_

