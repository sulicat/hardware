#pragma once

#include "hid_messages.h"

typedef enum EventType{
    EVENT_CONNECT,
    EVENT_DISCONNECT,
    EVENT_MOUSE_MOVE,
    EVENT_MOUSE_DOWN,
    EVENT_KEY_DOWN
} EventType_t;

typedef struct Event{
    EventType_t type;
} Event_t;


void print_hid_packet(const HID_MESSAGE_PACKET_t *packet);
// given an HID packet, read all the events and return a list of events to process.
int parse_hid_packet(HID_MESSAGE_PACKET_t* packet, Event_t* events, int max_events);
void print_event(Event_t* event);