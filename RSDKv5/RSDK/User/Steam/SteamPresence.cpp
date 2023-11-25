#if RETRO_REV02
void SteamRichPresence::SetPresence(int32 id, String *message)
{
    char text[k_cchMaxRichPresenceValueLength];
    GetCString(text, message);
    SteamFriends()->SetRichPresence("status", text);
}
#endif
