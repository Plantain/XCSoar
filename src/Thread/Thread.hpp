/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef XCSOAR_THREAD_THREAD_HPP
#define XCSOAR_THREAD_THREAD_HPP

#include "Util/NonCopyable.hpp"

#ifdef HAVE_POSIX
#include <pthread.h>
#else
#include <windows.h>
#endif

/**
 * This class provides an OS independent view on a thread.
 */
class Thread : private NonCopyable {
#ifdef HAVE_POSIX
  pthread_t handle;
  bool m_defined;
#else
  HANDLE handle;
#endif

public:
#ifdef HAVE_POSIX
  Thread():m_defined(false) {}
#else
  Thread():handle(NULL) {}
#endif
  virtual ~Thread();

  bool defined() const {
#ifdef HAVE_POSIX
    return m_defined;
#else
    return handle != NULL;
#endif
  }

  /**
   * Check if this thread is the current thread.
   */
  bool inside() const {
#ifdef HAVE_POSIX
    return pthread_self() == handle;
#else
    return GetCurrentThread() == handle;
#endif
  }

  void set_low_priority() {
#ifndef HAVE_POSIX
    ::SetThreadPriority(handle, THREAD_PRIORITY_LOWEST);
#endif
  }

  bool start();
  void join();
  bool join(unsigned timeout_ms);

  void terminate();

protected:
  virtual void run() = 0;

private:
#ifdef HAVE_POSIX
  static void *thread_proc(void *lpParameter);
#else
  static DWORD WINAPI thread_proc(LPVOID lpParameter);
#endif
};

#endif
