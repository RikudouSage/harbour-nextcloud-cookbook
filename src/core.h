#ifndef CORE_H
#define CORE_H

#include <QObject>

#include "secrets.h"

class Core : public QObject
{
    Q_OBJECT
public:
    explicit Core(Secrets *secrets, QObject *parent = nullptr);

private:
    Secrets *secrets;

};

#endif // CORE_H
