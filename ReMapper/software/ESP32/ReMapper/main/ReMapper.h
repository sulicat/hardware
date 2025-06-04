#pragma once

#include "hid_messages.h"

typedef enum EventType {
    EVENT_KEY_PRESSED,
    EVENT_KEY_RELEASED
} EventType_t;

typedef struct Event {
    EventType_t type;
    uint8_t keycode;
    char ascii;

} Event_t;

void print_hid_packet(const HID_MESSAGE_PACKET_t *packet);
int parse_hid_packet(HID_MESSAGE_PACKET_t *packet, Event_t *events, int max_events);
void print_event(Event_t *event);
char keycode_from_report(char report_val);
char keycode_to_ascii(uint8_t keycode, bool shift);