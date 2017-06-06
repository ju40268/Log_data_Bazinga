#pragma once

#include <QTextEdit>
#include "devio.h"
#include "registerio.h"

class Register;

class r00 :
	public QTextEdit,
	public RegisterIO
{
    Q_OBJECT
public:
    explicit r00(Register* base);
private:
    Register* base;
    
    void onRegisterChange(bool write, const vector<Byte>&data) override;
};
