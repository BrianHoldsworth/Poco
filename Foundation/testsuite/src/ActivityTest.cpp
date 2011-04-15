//
// ActivityTest.cpp
//
// $Id: //poco/1.4/Foundation/testsuite/src/ActivityTest.cpp#1 $
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#include "ActivityTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Poco/Activity.h"
#include "Poco/Thread.h"


using Poco::Activity;
using Poco::Thread;


namespace
{
	class ActiveObject
	{
	public:
		ActiveObject(): 
			_activity(this, &ActiveObject::run),
			_count(0)
		{
		}
		
		~ActiveObject()
		{
		}
		
		Activity<ActiveObject>& activity()
		{
			return _activity;
		}
		
		int count() const
		{
			return _count;
		}

	protected:
		void run()
		{
			while (!_activity.isStopped()) 
				++_count;
		}

	private:
		Activity<ActiveObject> _activity;
		int                    _count;
	};
}
 

ActivityTest::ActivityTest(const std::string& name): CppUnit::TestCase(name)
{
}


ActivityTest::~ActivityTest()
{
}


void ActivityTest::testActivity()
{
	ActiveObject activeObj;
	assert (activeObj.activity().isStopped());
	activeObj.activity().start();
	assert (!activeObj.activity().isStopped());
	Thread::sleep(1000);
	assert (activeObj.activity().isRunning());
	activeObj.activity().stop();
	activeObj.activity().wait();
	assert (activeObj.count() > 0);
}


void ActivityTest::setUp()
{
}


void ActivityTest::tearDown()
{
}


CppUnit::Test* ActivityTest::suite()
{
	CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("ActivityTest");

	CppUnit_addTest(pSuite, ActivityTest, testActivity);

	return pSuite;
}
