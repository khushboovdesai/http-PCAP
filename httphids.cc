/*
 * counter.{cc,hh} -- element counts packets, measures packet rate
 * Eddie Kohler
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2008 Regents of the University of California
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <click/config.h>
#include "httphids.hh"
#include <click/error.hh>
#include <click/confparse.hh>
#include <click/args.hh>
#include <click/handlercall.hh>
//#include <iostream.h>
//using namespace std;
CLICK_DECLS

HttpHids::HttpHids()
  : _count_trigger_h(0), _byte_trigger_h(0)
{
}

HttpHids::~HttpHids()
{
  delete _count_trigger_h;
  delete _byte_trigger_h;
}

void
HttpHids::reset()
{
  _count = _byte_count = 0;
  _count_triggered = _byte_triggered = false;
}

int
HttpHids::configure(Vector<String> &conf, ErrorHandler *errh)
{
  String count_call, byte_count_call;
  if (Args(conf, this, errh)
      .read("COUNT_CALL", AnyArg(), count_call)
      .read("BYTE_COUNT_CALL", AnyArg(), byte_count_call).complete() < 0)
    return -1;

  if (count_call) {
    IntArg ia;
    if (!ia.parse_saturating(cp_shift_spacevec(count_call), _count_trigger))
      return errh->error("COUNT_CALL type mismatch");
    else if (ia.status == IntArg::status_range)
      errh->error("COUNT_CALL overflow, max %s", String(_count_trigger).c_str());
    _count_trigger_h = new HandlerCall(count_call);
  } else
    _count_trigger = (counter_t)(-1);

  if (byte_count_call) {
    IntArg ia;
    if (!ia.parse_saturating(cp_shift_spacevec(byte_count_call), _byte_trigger))
      return errh->error("BYTE_COUNT_CALL type mismatch");
    else if (ia.status == IntArg::status_range)
      errh->error("BYTE_COUNT_CALL overflow, max %s", String(_count_trigger).c_str());
    _byte_trigger_h = new HandlerCall(byte_count_call);
  } else
    _byte_trigger = (counter_t)(-1);

  return 0;
}

int
HttpHids::initialize(ErrorHandler *errh)
{
  if (_count_trigger_h && _count_trigger_h->initialize_write(this, errh) < 0)
    return -1;
  if (_byte_trigger_h && _byte_trigger_h->initialize_write(this, errh) < 0)
    return -1;
  reset();
  return 0;
}

Packet *
HttpHids::simple_action(Packet *p)
{
    _count++;
     if(_count<=50) {
	printf("Count value from the cc file is %d\n",_count);
     }
     else {
	printf("Count value exceeded threshold \n");
     }

     const unsigned char *pktPointer = p->data();
     unsigned long *parseData = (unsigned long *)pktPointer;  
     unsigned long long *parseDatanew = (unsigned long long *)pktPointer;  
     unsigned long maskedData=0;
     bool isFaultURL = false;     //if URL is declared faulty, set this flag to true
     pktPointer += 54;
     parseData = (unsigned long *) pktPointer;
     maskedData = * parseData & 0xffffff;
  
     printf("GET packet bytes in header is: %06x\n",  * parseData & 0xffffff);
     
     if(maskedData == 0x474554)
     {
     	printf("Its a GET packet!!\n");
        pktPointer += 4;
        parseData = (unsigned long *) pktPointer;
        unsigned long maskedData1 = 0;
        maskedData1 = * parseData && 0xffffffff;
     	printf("MaskedData1 is %lx\n", maskedData1);
        
	pktPointer += 4;
        parseDatanew = (unsigned long long*) pktPointer;
        unsigned long long maskedData2 = 0;
        maskedData2 = * parseDatanew & 0xffffffffffffff;
     	printf("MaskedData2 is %llx\n", maskedData2);
        
	if(maskedData1 == 0x2f657463 && maskedData2 == 0x2f706173737764)   //checking for /etc and /passwd
        {
		
     		isFaultURL = true;     //if URL is declared faulty, set this flag to true
		printf("The hacker is trying to access your registered users' credentials! Anomaly detected!");
	}

     }     

    _byte_count += p->length();
    _rate.update(1);
    _byte_rate.update(p->length());

  if (_count == _count_trigger && !_count_triggered) {
    _count_triggered = true;
    //cout<<"count triggered\n";
    if (_count_trigger_h){
      (void) _count_trigger_h->call_write();
       //cout<<"count triggered header\n";
  }
}
  if (_byte_count >= _byte_trigger && !_byte_triggered) {
    _byte_triggered = true;
     //cout<<"byte triggered";
    if (_byte_trigger_h){
      (void) _byte_trigger_h->call_write();
      //cout<<"byte triggered header\n";
  }
}

  return p;
}


enum { H_COUNT, H_BYTE_COUNT, H_RATE, H_BIT_RATE, H_BYTE_RATE, H_RESET,
       H_COUNT_CALL, H_BYTE_COUNT_CALL };

String
HttpHids::read_handler(Element *e, void *thunk)
{
    HttpHids *c = (HttpHids *)e;
    switch ((intptr_t)thunk) {
      case H_COUNT:
        //cout<<"We are here\n";
	return String(c->_count);
      case H_BYTE_COUNT:
	return String(c->_byte_count);
      case H_RATE:
	c->_rate.update(0);	// drop rate after idle period
	return c->_rate.unparse_rate();
      case H_BIT_RATE:
	c->_byte_rate.update(0); // drop rate after idle period
	// avoid integer overflow by adjusting scale factor instead of
	// multiplying
	if (c->_byte_rate.scale() >= 3)
	    return cp_unparse_real2(c->_byte_rate.scaled_average() * c->_byte_rate.epoch_frequency(), c->_byte_rate.scale() - 3);
	else
	    return cp_unparse_real2(c->_byte_rate.scaled_average() * c->_byte_rate.epoch_frequency() * 8, c->_byte_rate.scale());
      case H_BYTE_RATE:
	c->_byte_rate.update(0); // drop rate after idle period
	return c->_byte_rate.unparse_rate();
      case H_COUNT_CALL:
	if (c->_count_trigger_h)
	    return String(c->_count_trigger);
	else
	    return String();
      default:
	return "<error>";
    }
}

int
HttpHids::write_handler(const String &in_str, Element *e, void *thunk, ErrorHandler *errh)
{
    HttpHids *c = (HttpHids *)e;
    String str = in_str;
    switch ((intptr_t)thunk) {
      case H_COUNT_CALL:
	  if (!IntArg().parse(cp_shift_spacevec(str), c->_count_trigger))
	    return errh->error("'count_call' first word should be unsigned (count)");
	if (HandlerCall::reset_write(c->_count_trigger_h, str, c, errh) < 0)
	    return -1;
	c->_count_triggered = false;
	return 0;
      case H_BYTE_COUNT_CALL:
	  if (!IntArg().parse(cp_shift_spacevec(str), c->_byte_trigger))
	    return errh->error("'byte_count_call' first word should be unsigned (count)");
	if (HandlerCall::reset_write(c->_byte_trigger_h, str, c, errh) < 0)
	    return -1;
	c->_byte_triggered = false;
	return 0;
      case H_RESET:
	c->reset();
	return 0;
      default:
	return errh->error("<internal>");
    }
}

void
HttpHids::add_handlers()
{
    add_read_handler("count", read_handler, H_COUNT);
    add_read_handler("byte_count", read_handler, H_BYTE_COUNT);
    add_read_handler("rate", read_handler, H_RATE);
    add_read_handler("bit_rate", read_handler, H_BIT_RATE);
    add_read_handler("byte_rate", read_handler, H_BYTE_RATE);
    add_write_handler("reset", write_handler, H_RESET, Handler::BUTTON);
    add_write_handler("reset_counts", write_handler, H_RESET, Handler::BUTTON | Handler::UNCOMMON);
    add_read_handler("count_call", read_handler, H_COUNT_CALL);
    add_write_handler("count_call", write_handler, H_COUNT_CALL);
    add_write_handler("byte_count_call", write_handler, H_BYTE_COUNT_CALL);
}

int
HttpHids::llrpc(unsigned command, void *data)
{
  if (command == CLICK_LLRPC_GET_RATE) {
    uint32_t *val = reinterpret_cast<uint32_t *>(data);
    if (*val != 0)
      return -EINVAL;
    _rate.update(0);		// drop rate after idle period
    *val = _rate.rate();
    return 0;

  } else if (command == CLICK_LLRPC_GET_COUNT) {
    uint32_t *val = reinterpret_cast<uint32_t *>(data);
    if (*val != 0 && *val != 1)
      return -EINVAL;
    *val = (*val == 0 ? _count : _byte_count);
    return 0;

  } else if (command == CLICK_LLRPC_GET_COUNTS) {
    click_llrpc_counts_st *user_cs = (click_llrpc_counts_st *)data;
    click_llrpc_counts_st cs;
    if (CLICK_LLRPC_GET_DATA(&cs, data, sizeof(cs.n) + sizeof(cs.keys)) < 0
	|| cs.n >= CLICK_LLRPC_COUNTS_SIZE)
      return -EINVAL;
    for (unsigned i = 0; i < cs.n; i++) {
      if (cs.keys[i] == 0)
	cs.values[i] = _count;
      else if (cs.keys[i] == 1)
	cs.values[i] = _byte_count;
      else
	return -EINVAL;
    }
    return CLICK_LLRPC_PUT_DATA(&user_cs->values, &cs.values, sizeof(cs.values));

  } else
    return Element::llrpc(command, data);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(HttpHids)
