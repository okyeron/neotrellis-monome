#include "debug.h"

void debug(int level, const char *message) {
    if (level < DEBUG_LEVEL) return;
    Serial.print(message);
}

void debug(int level, String message) {
    debug(level, message.c_str());
}

void debugln(int level, const char *message) {
    if (level < DEBUG_LEVEL) return;
    Serial.println(message);
}

void debugln(int level, String message) {
    debugln(level, message.c_str());
}

void debugln(int level) {
    if (level < DEBUG_LEVEL) return;
    Serial.println();
}

void debugf(int level, const char *message, ...) {
    if (level < DEBUG_LEVEL) return;

    va_list ap;
    va_start(ap, message);
    Serial.printf(message, ap);
}

void debugf(int level, String message, ...) {
    debugf(level, message.c_str());
}

void debugfln(int level, const char *message, ...) {
    if (level < DEBUG_LEVEL) return;

    va_list ap;
    va_start(ap, message);
    char buffer[200]= {0};
    vsnprintf(buffer, 200, message, ap);
    va_end(ap);

    Serial.println(buffer);
}

void debugfln(int level, String message, ...) {
    debugfln(level, message.c_str());
}
