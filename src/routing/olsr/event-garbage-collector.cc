/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INESC Porto
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Gustavo J. A. M. Carneiro  <gjc@inescporto.pt>
 */
#include "event-garbage-collector.h"

#define CLEANUP_CHUNK_MIN_SIZE 8
#define CLEANUP_CHUNK_MAX_SIZE 128


namespace ns3 {


EventGarbageCollector::EventGarbageCollector () :
  m_nextCleanupSize (CLEANUP_CHUNK_MIN_SIZE)
{}

void
EventGarbageCollector::Track (EventId event)
{
  m_events.insert (event);
  if (m_events.size () >= m_nextCleanupSize)
    Cleanup ();
}

void
EventGarbageCollector::Grow ()
{
  m_nextCleanupSize += (m_nextCleanupSize < CLEANUP_CHUNK_MAX_SIZE?
                        m_nextCleanupSize : CLEANUP_CHUNK_MAX_SIZE);
}

void
EventGarbageCollector::Shrink ()
{
  while (m_nextCleanupSize > m_events.size ())
    m_nextCleanupSize >>= 1;
  Grow ();
}

// Called when a new event was added and the cleanup limit was exceeded in consequence.
void
EventGarbageCollector::Cleanup ()
{
  for (EventList::iterator iter = m_events.begin (); iter != m_events.end ();)
    {
      if ((*iter).IsExpired ())
        {
          m_events.erase (iter++);
        }
      else
        break; // EventIds are sorted by timestamp => further events are not expired for sure
    }

  // If after cleanup we are still over the limit, increase the limit.
  if (m_events.size () >= m_nextCleanupSize)
    Grow ();
  else
    Shrink ();
}


EventGarbageCollector::~EventGarbageCollector ()
{
  for (EventList::iterator event = m_events.begin ();
       event != m_events.end (); event++)
    {
      Simulator::Cancel (*event);
    }
}

}; // namespace ns3



#ifdef RUN_SELF_TESTS

#include "ns3/test.h"

namespace ns3 {

class EventGarbageCollectorTests : public Test
{
  int m_counter;
  EventGarbageCollector *m_events;

  void EventGarbageCollectorCallback ();

public:

  EventGarbageCollectorTests ();
  virtual ~EventGarbageCollectorTests ();
  virtual bool RunTests (void);
};

EventGarbageCollectorTests::EventGarbageCollectorTests ()
  : Test ("EventGarbageCollector"), m_counter (0), m_events (0)
{}

EventGarbageCollectorTests::~EventGarbageCollectorTests ()
{}

void
EventGarbageCollectorTests::EventGarbageCollectorCallback ()
{
  m_counter++;
  if (m_counter == 50)
    {
      // this should cause the remaining (50) events to be cancelled
      delete m_events;
      m_events = 0;
    }
}

bool EventGarbageCollectorTests::RunTests (void)
{
  bool result = true;

  m_events = new EventGarbageCollector ();

  for (int n = 0; n < 100; n++)
    {
      m_events->Track (Simulator::Schedule
                       (Simulator::Now (),
                        &EventGarbageCollectorTests::EventGarbageCollectorCallback,
                        this));
    }
  Simulator::Run ();
  NS_TEST_ASSERT_EQUAL (m_events, 0);
  NS_TEST_ASSERT_EQUAL (m_counter, 50);
  return result;
}

static EventGarbageCollectorTests g_eventCollectorTests;
    
};

#endif /* RUN_SELF_TESTS */
