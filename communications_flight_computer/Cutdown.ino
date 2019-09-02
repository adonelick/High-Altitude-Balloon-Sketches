

void cutdown_payload()
{
    relays.switchRelayOn(CUTDOWN_RELAY);
    delay(2000);
    relays.switchRelayOff(CUTDOWN_RELAY);
}
