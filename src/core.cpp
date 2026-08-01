#include "core.h"

Core::Core(Secrets *secrets, QObject *parent)
    : QObject(parent), secrets(secrets)
{
}
