/*
 * Copyright (c) 2011-2013 Juli Malett. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef	EVENT_SPEED_TEST_H
#define	EVENT_SPEED_TEST_H

#include <common/thread/mutex.h>

#include <event/callback.h>
#include <event/event_system.h>

#define	SPEED_TEST_TIMER_MS	1000

class SpeedTest {
protected:
	Mutex mtx_;
public:
	SimpleCallback::Method<SpeedTest> callback_complete_;
	Action *callback_action_;
	SimpleCallback::Method<SpeedTest> timer_;
	Action *timeout_action_;
public:
	SpeedTest(void)
	: mtx_("SpeedTest"),
	  callback_complete_(NULL, &mtx_, this, &SpeedTest::callback_complete),
	  callback_action_(NULL),
	  timer_(NULL, &mtx_, this, &SpeedTest::timer),
	  timeout_action_(NULL)
	{
		ScopedLock _(&mtx_);
		timeout_action_ = EventSystem::instance()->timeout(SPEED_TEST_TIMER_MS, &timer_);
	}

	virtual ~SpeedTest()
	{
		ASSERT_NULL("/speed/test", callback_action_);
		ASSERT_NULL("/speed/test", timeout_action_);
	}

private:
	void callback_complete(void)
	{
		ASSERT_LOCK_OWNED("/speed/test", &mtx_);
		callback_action_->cancel();
		callback_action_ = NULL;

		perform();
	}

	void timer(void)
	{
		ASSERT_LOCK_OWNED("/speed/test", &mtx_);
		timeout_action_->cancel();
		timeout_action_ = NULL;

		if (callback_action_ != NULL) {
			callback_action_->cancel();
			callback_action_ = NULL;
		}

		finish();

		EventSystem::instance()->stop();
	}

protected:
	void schedule(void)
	{
		ASSERT_LOCK_OWNED("/speed/test", &mtx_);
		ASSERT_NULL("/speed/test", callback_action_);
		callback_action_ = callback_complete_.schedule();
	}

	virtual void perform(void)
	{ }

	virtual void finish(void)
	{ }
};

#endif /* !EVENT_SPEED_TEST_H */
