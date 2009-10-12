/*
 *  bdremoteng - helper daemon for Sony(R) BD Remote Control
 *  Based on bdremoted, written by Anton Starikov <antst@mail.ru>.
 *  
 *  Copyright (C) 2009  Michael Wojciechowski <wojci@wojci.dk>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "lirc_srv.h"

#include <globaldefs.h>

#include <stdint.h>
#include <keydef.h>
#include <stdio.h>
#include <assert.h>

#include <unistd.h>

#define _GNU_SOURCE
#include <signal.h>

extern volatile sig_atomic_t __io_canceled;

/** The number of repeats before this driver detects that the user is
    holding down a key on the remote. */
#define REPEATS_BEFORE_TRANSMIT 2

/** Thread. */
void* lircThread (void* q);

/** Broadcast an event to LIRC. */
void broadcastToLirc(lirc_data* _lc, const char* _name, int _rep, unsigned int _code);

/** Handle a key press. */
void handleKey(lirc_data* _ld, 
	       const char* _data, const int _size, 
	       keyState* _ks);

/** Handle key down. */
void DataInd_keyDown(lirc_data* _lc,
		     unsigned int _code, 
		     uint32_t _mask,
		     keyState* _ks);

/** Handle key up. */
void DataInd_keyUp(lirc_data* _lc,
		   unsigned int _code, 
		   uint32_t _mask,
		   keyState* _ks);

void startLircThread(lirc_data* _ld)
{
  pthread_create(&_ld->thread, NULL, lircThread, _ld);
}

void waitForLircThread(lirc_data* _ld)
{
  pthread_join(_ld->thread, NULL);
}

void* lircThread (void* q)
{
  lirc_data* ld = (lirc_data*)q;
  queueData* qd = NULL;
  int res       = Q_ERR;
  keyState ks;
  int rate_mod  = 1000;

  if (ld->config->repeat_rate > 0)
    {
      rate_mod = (int) (1000 / ld->config->repeat_rate);
    }

#if BDREMOTE_DEBUG
  BDREMOTE_DBG_HDR(ld->config->debug);
  printf("using repeat rate: %d.\n", rate_mod);
#endif

  ks.keyDown = 0;
  ks.lastKey = ps3remote_undef;

  BDREMOTE_DBG(ld->config->debug, "Started LIRC thread.");

  initTime(&ks);
  ks.elapsed_last = 0;
  ks.repeat_sent  = 0;
  ks.repeat_count = 0;

  while (!__io_canceled)
    {
      qd  = NULL;
      res = queueRem (&ld->qu, 0 /* No blocking. */, &qd);

      if (res == Q_OK)
	{
	  assert(qd->buffer != NULL);
	  handleKey(ld, qd->buffer, qd->size, &ks);
	  
	  queueDataDeInit(qd);

	  initTime(&ks);
	  ks.elapsed_last = 0;
	  ks.repeat_sent  = 0;
	  ks.repeat_count = 0;
	}

      if (ks.keyDown == 1)
	{
	  updateTime(&ks);

	  if (ks.elapsed % rate_mod == 0)
	    {
	      if (ks.elapsed_last == ks.elapsed)
		{
		  usleep(10);
		  continue;
		}

	      ks.elapsed_last = ks.elapsed;

	      if (ks.repeat_count > REPEATS_BEFORE_TRANSMIT)
		{
		  printf("Key is down: %lu\n", ks.elapsed);
		  broadcastToLirc(ld, ps3remote_keys[ks.lastKey].name, 0 /*ks.repeat_sent*/, ps3remote_keys[ks.lastKey].code);
		  //broadcastToLirc(ld, ps3remote_keys[ks.lastKey].name, 0 /* ks.repeat_sent */, 0xFF);
		  ks.repeat_sent++;
		}
	      ks.repeat_count++;
	    }
	}
      else
	{
	  usleep(100);
	}
    }

  BDREMOTE_DBG(ld->config->debug, "Thread terminating ..");

  return (NULL);
}

/* Received some data from the ps3 remote. 
 * Forward it to LIRC clients.
 * Note: no threads are used, so no need for locking.
 */

void handleKey(lirc_data* _ld, 
	       const char* _data, const int _size, 
	       keyState* _ks)
{
  int num                 = 0;
  const uint32_t* mask_in = (uint32_t *)(_data+2);
  uint32_t mask           = 0;
  const unsigned char* magic = (const unsigned char*)_data;
  const unsigned char* code  = (const unsigned char*)_data+5;	
  const unsigned char* state = (const unsigned char*)_data+11;

#if BDREMOTE_DEBUG
  assert(_ld->magic0 == 0x15);
#endif /* BDREMOTE_DEBUG */

  if ((_size==13) && (*magic==0xa1))
    {
      mask=(*mask_in) & 0xFFFFFF;

      num=-1;

      switch (*state)
	{
	case 1:
	  {
	    DataInd_keyDown(_ld, *code, mask, _ks);
	    break;
	  }
	case 0:
	  {
	    DataInd_keyUp(_ld, *code, mask, _ks);
	    break;
	  }
	}
    }
}

int codeToIndex(unsigned int _code)
{
  int num = ps3remote_undef;
  int i   = 0;
  for (i=0;i<ps3remote_num_keys;++i)
    {
      if (_code==ps3remote_keys[i].code)
	{
	  num=i;
	  break;
	}
    }
  return num;
}

void DataInd_keyDown(lirc_data* _lc,
		     unsigned int _code, 
		     uint32_t _mask,
		     keyState* _ks)
{
  int num = ps3remote_undef;
  if (_code != ps3remote_keyup)
    {
      /* Key pressed. */
#if BDREMOTE_DEBUG
      BDREMOTE_DBG_HDR(_lc->config->debug);
      printf("key down: %02X, %08X\n", _code, _mask);
      BDREMOTE_DBG(_lc->config->debug, "single.");
#endif
      num               = codeToIndex(_code);
      _ks->keyDown      = 1;
      _ks->lastKey      = num;
      _ks->repeat_count = 0;
      _ks->repeat_sent  = 0;

    }
  if (num != ps3remote_undef)
    {
      broadcastToLirc(_lc, ps3remote_keys[num].name, 0, ps3remote_keys[num].code);
    }
}

void DataInd_keyUp(lirc_data* _lc,
		   unsigned int _code, 
		   uint32_t _mask,
		   keyState* _ks)
{
  if (_code == ps3remote_keyup)
    {
      /* Key up. */
#if BDREMOTE_DEBUG
      BDREMOTE_DBG_HDR(_lc->config->debug);
      printf("key up: %02X, %08X\n", _code, _mask);
#endif
      if (_ks->lastKey != ps3remote_undef)
	{
	  // broadcastToLirc(_lc, ps3remote_keys[_ks->lastKey].name, 0, _code);

	  _ks->keyDown      = 0;
	  _ks->lastKey      = ps3remote_undef;
	  _ks->repeat_count = 0;
	  _ks->repeat_sent  = 0;
	}
    }
}

/* Broadcast the last received key to all connected LIRC clients. */
void broadcastToLirc(lirc_data* _ld, const char* _name, int _rep, unsigned int _code)
{ 
  int i = 0;
  char msg[100];
  int len = sprintf(msg,"%04X %02d %s %s\n", _code, _rep, _name, "SonyBDRemote");

  BDREMOTE_DBG(_ld->config->debug, msg);

#if BDREMOTE_DEBUG
  BDREMOTE_DBG_HDR(_ld->config->debug);
  printf("_ld->magic0=%d.\n", _ld->magic0);
  assert(_ld->magic0 == 0x15);
#endif /* BDREMOTE_DEBUG */
  assert(_ld->clin < MAX_CLIENTS);

  pthread_mutex_lock (&_ld->dataMutex);

  for (i=0; i<_ld->clin; i++)
    {
      if (write_socket(_ld->clis[i], msg, len)<len)
	{
	  remove_client(_ld, _ld->clis[i]);
	  i--;
	}
#if BDREMOTE_DEBUG
      else
	{
	  BDREMOTE_DBG_HDR(_ld->config->debug);
	  printf("broadcast %d bytes to socket id %d.\n", len, _ld->clis[i]);
	}
#endif /* BDREMOTE_DEBUG */
    }

  pthread_mutex_unlock (&_ld->dataMutex);
}