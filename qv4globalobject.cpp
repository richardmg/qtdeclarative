/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the V4VM module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qv4globalobject.h"
#include "qv4mm.h"
#include "qmljs_value.h"
#include "qmljs_environment.h"

#include <private/qqmljsengine_p.h>
#include <private/qqmljslexer_p.h>
#include <private/qqmljsparser_p.h>
#include <private/qqmljsast_p.h>
#include <qv4ir_p.h>
#include <qv4codegen_p.h>
#include "private/qlocale_tools_p.h"

#include <QtCore/QDebug>
#include <QtCore/QString>
#include <iostream>

using namespace QQmlJS::VM;

static inline char toHex(char c)
{
    static const char hexnumbers[] = "0123456789ABCDEF";
    return hexnumbers[c & 0xf];
}

static int fromHex(QChar ch)
{
    ushort c = ch.unicode();
    if ((c >= '0') && (c <= '9'))
        return c - '0';
    if ((c >= 'A') && (c <= 'F'))
        return c - 'A' + 10;
    if ((c >= 'a') && (c <= 'f'))
        return c - 'a' + 10;
    return -1;
}

static QString escape(const QString &input)
{
    QString output;
    output.reserve(input.size() * 3);
    const int length = input.length();
    for (int i = 0; i < length; ++i) {
        ushort uc = input.at(i).unicode();
        if (uc < 0x100) {
            if (   (uc > 0x60 && uc < 0x7B)
                || (uc > 0x3F && uc < 0x5B)
                || (uc > 0x2C && uc < 0x3A)
                || (uc == 0x2A)
                || (uc == 0x2B)
                || (uc == 0x5F)) {
                output.append(QChar(uc));
            } else {
                output.append('%');
                output.append(QChar(toHex(uc >> 4)));
                output.append(QChar(toHex(uc)));
            }
        } else {
            output.append('%');
            output.append('u');
            output.append(QChar(toHex(uc >> 12)));
            output.append(QChar(toHex(uc >> 8)));
            output.append(QChar(toHex(uc >> 4)));
            output.append(QChar(toHex(uc)));
        }
    }
    return output;
}

static QString unescape(const QString &input)
{
    QString result;
    result.reserve(input.length());
    int i = 0;
    const int length = input.length();
    while (i < length) {
        QChar c = input.at(i++);
        if ((c == '%') && (i + 1 < length)) {
            QChar a = input.at(i);
            if ((a == 'u') && (i + 4 < length)) {
                int d3 = fromHex(input.at(i+1));
                int d2 = fromHex(input.at(i+2));
                int d1 = fromHex(input.at(i+3));
                int d0 = fromHex(input.at(i+4));
                if ((d3 != -1) && (d2 != -1) && (d1 != -1) && (d0 != -1)) {
                    ushort uc = ushort((d3 << 12) | (d2 << 8) | (d1 << 4) | d0);
                    result.append(QChar(uc));
                    i += 5;
                } else {
                    result.append(c);
                }
            } else {
                int d1 = fromHex(a);
                int d0 = fromHex(input.at(i+1));
                if ((d1 != -1) && (d0 != -1)) {
                    c = (d1 << 4) | d0;
                    i += 2;
                }
                result.append(c);
            }
        } else {
            result.append(c);
        }
    }
    return result;
}

static const char uriReserved[] = ";/?:@&=+$,";
static const char uriUnescaped[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.!~*'()";

static void addEscapeSequence(QString &output, uchar ch)
{
    output.append(QLatin1Char('%'));
    output.append(QLatin1Char(toHex(ch >> 4)));
    output.append(QLatin1Char(toHex(ch & 0xf)));
}

static QString encode(const QString &input, const QString &unescapedSet, bool *ok)
{
    *ok = true;
    QString output;
    const int length = input.length();
    int i = 0;
    while (i < length) {
        const QChar c = input.at(i);
        if (!unescapedSet.contains(c)) {
            uint uc = c.unicode();
            if ((uc >= 0xDC00) && (uc <= 0xDFFF)) {
                *ok = false;
                break;
            }
            if (!((uc < 0xD800) || (uc > 0xDBFF))) {
                ++i;
                if (i == length) {
                    *ok = false;
                    break;
                }
                const uint uc2 = input.at(i).unicode();
                if ((uc2 < 0xDC00) || (uc2 > 0xDFFF)) {
                    *ok = false;
                    break;
                }
                uc = ((uc - 0xD800) * 0x400) + (uc2 - 0xDC00) + 0x10000;
            }
            if (uc < 0x80) {
                addEscapeSequence(output, (uchar)uc);
            } else {
                if (uc < 0x0800) {
                    addEscapeSequence(output, 0xc0 | ((uchar) (uc >> 6)));
                } else {

                    if (QChar::requiresSurrogates(uc)) {
                        addEscapeSequence(output, 0xf0 | ((uchar) (uc >> 18)));
                        addEscapeSequence(output, 0x80 | (((uchar) (uc >> 12)) & 0x3f));
                    } else {
                        addEscapeSequence(output, 0xe0 | (((uchar) (uc >> 12)) & 0x3f));
                    }
                    addEscapeSequence(output, 0x80 | (((uchar) (uc >> 6)) & 0x3f));
                }
                addEscapeSequence(output, 0x80 | ((uchar) (uc&0x3f)));
            }
        } else {
            output.append(c);
        }
        ++i;
    }
    if (i != length)
        *ok = false;
    return output;
}

static QString decode(const QString &input, const QString &reservedSet, bool *ok)
{
    *ok = true;
    QString output;
    const int length = input.length();
    int i = 0;
    const QChar percent = QLatin1Char('%');
    while (i < length) {
        const QChar ch = input.at(i);
        if (ch == percent) {
            int start = i;
            if (i + 2 >= length)
                goto error;

            int d1 = fromHex(input.at(i+1));
            int d0 = fromHex(input.at(i+2));
            if ((d1 == -1) || (d0 == -1))
                goto error;

            int b = (d1 << 4) | d0;
            i += 2;
            if (b & 0x80) {
                int uc;
                int min_uc;
                int need;
                if ((b & 0xe0) == 0xc0) {
                    uc = b & 0x1f;
                    need = 1;
                    min_uc = 0x80;
                } else if ((b & 0xf0) == 0xe0) {
                    uc = b & 0x0f;
                    need = 2;
                    min_uc = 0x800;
                } else if ((b & 0xf8) == 0xf0) {
                    uc = b & 0x07;
                    need = 3;
                    min_uc = 0x10000;
                } else {
                    goto error;
                }

                if (i + (3 * need) >= length)
                    goto error;

                for (int j = 0; j < need; ++j) {
                    ++i;
                    if (input.at(i) != percent)
                        goto error;

                    d1 = fromHex(input.at(i+1));
                    d0 = fromHex(input.at(i+2));
                    if ((d1 == -1) || (d0 == -1))
                        goto error;

                    b = (d1 << 4) | d0;
                    if ((b & 0xC0) != 0x80)
                        goto error;

                    i += 2;
                    uc = (uc << 6) + (b & 0x3f);
                }
                if (uc < min_uc)
                    goto error;

                if (uc < 0x10000) {
                    output.append(QChar(uc));
                } else {
                    if (uc > 0x10FFFF)
                        goto error;

                    ushort l = ushort(((uc - 0x10000) & 0x3FF) + 0xDC00);
                    ushort h = ushort((((uc - 0x10000) >> 10) & 0x3FF) + 0xD800);
                    output.append(QChar(l));
                    output.append(QChar(h));
                }
            } else {
                QChar z(b);
                if (!reservedSet.contains(z)) {
                    output.append(z);
                } else {
                    output.append(input.mid(start, i - start + 1));
                }
            }
        } else {
            output.append(ch);
        }
        ++i;
    }
    if (i != length)
        *ok = false;
    return output;
  error:
    *ok = false;
    return QString();
}


EvalFunction::EvalFunction(ExecutionContext *scope)
    : FunctionObject(scope)
{
    name = scope->engine->id_eval;
}

Value EvalFunction::call(ExecutionContext *context, Value /*thisObject*/, Value *args, int argc, bool directCall)
{
    if (argc < 1)
        return Value::undefinedValue();

    if (!args[0].isString())
        return args[0];

    const QString code = args[0].stringValue()->toQString();
    QQmlJS::VM::Function *f = parseSource(context, QStringLiteral("eval code"), code, QQmlJS::Codegen::EvalCode);
    if (!f)
        return Value::undefinedValue();

    bool strict = f->isStrict || context->strictMode;

    ExecutionContext k;

    ExecutionContext *ctx = context;
    if (!directCall) {
        // the context for eval should be the global scope
        while (ctx->parent)
            ctx = ctx->parent;
    }

    if (strict) {
        ctx = &k;
        ctx->initCallContext(context, context->thisObject, this, args, argc);
    }

    // set the correct strict mode flag on the context
    bool cstrict = ctx->strictMode;
    ctx->strictMode = strict;

    Value result = f->code(ctx, f->codeData);

    ctx->strictMode = cstrict;

    if (strict)
        ctx->leaveCallContext();

    return result;
}


Value EvalFunction::call(ExecutionContext *context, Value thisObject, Value *args, int argc)
{
    // indirect call
    return call(context, thisObject, args, argc, false);
}

QQmlJS::VM::Function *EvalFunction::parseSource(QQmlJS::VM::ExecutionContext *ctx,
                                                const QString &fileName, const QString &source,
                                                QQmlJS::Codegen::Mode mode)
{
    using namespace QQmlJS;

    MemoryManager::GCBlocker gcBlocker(ctx->engine->memoryManager);

    VM::ExecutionEngine *vm = ctx->engine;
    IR::Module module;
    VM::Function *globalCode = 0;

    {
        QQmlJS::Engine ee, *engine = &ee;
        Lexer lexer(engine);
        lexer.setCode(source, 1, false);
        Parser parser(engine);

        const bool parsed = parser.parseProgram();

        VM::DiagnosticMessage *error = 0, **errIt = &error;
        foreach (const QQmlJS::DiagnosticMessage &m, parser.diagnosticMessages()) {
            if (m.isError()) {
                *errIt = new VM::DiagnosticMessage;
                (*errIt)->fileName = fileName;
                (*errIt)->offset = m.loc.offset;
                (*errIt)->length = m.loc.length;
                (*errIt)->startLine = m.loc.startLine;
                (*errIt)->startColumn = m.loc.startColumn;
                (*errIt)->type = VM::DiagnosticMessage::Error;
                (*errIt)->message = m.message;
                errIt = &(*errIt)->next;
            } else {
                std::cerr << qPrintable(fileName) << ':' << m.loc.startLine << ':' << m.loc.startColumn
                          << ": warning: " << qPrintable(m.message) << std::endl;
            }
        }
        if (error)
            ctx->throwSyntaxError(error);

        if (parsed) {
            using namespace AST;
            Program *program = AST::cast<Program *>(parser.rootNode());
            if (!program) {
                // if parsing was successful, and we have no program, then
                // we're done...:
                return 0;
            }

            Codegen cg(ctx);
            IR::Function *globalIRCode = cg(fileName, program, &module, mode);
            QScopedPointer<EvalInstructionSelection> isel(ctx->engine->iselFactory->create(vm, &module));
            if (globalIRCode)
                globalCode = isel->vmFunction(globalIRCode);
        }

        if (! globalCode)
            // ### should be a syntax error
            __qmljs_throw_type_error(ctx);
    }

    return globalCode;
}

static inline int toInt(const QChar &qc, int R)
{
    ushort c = qc.unicode();
    int v = -1;
    if (c >= '0' && c <= '9')
        v = c - '0';
    else if (c >= 'A' && c <= 'Z')
        v = c - 'A' + 10;
    else if (c >= 'a' && c <= 'z')
        v = c - 'a' + 10;
    if (v >= 0 && v < R)
        return v;
    else
        return -1;
}

// parseInt [15.1.2.2]
Value GlobalFunctions::method_parseInt(ExecutionContext *context)
{
    Value string = context->argument(0);
    Value radix = context->argument(1);
    int R = radix.isUndefined() ? 0 : radix.toInt32(context);

    // [15.1.2.2] step by step:
    String *inputString = string.toString(context); // 1
    QString trimmed = inputString->toQString().trimmed(); // 2
    const QChar *pos = trimmed.constData();
    const QChar *end = pos + trimmed.length();

    int sign = 1; // 3
    if (pos != end) {
        if (*pos == QLatin1Char('-'))
            sign = -1; // 4
        if (*pos == QLatin1Char('-') || *pos == QLatin1Char('+'))
            ++pos; // 5
    }
    bool stripPrefix = true; // 7
    if (R) { // 8
        if (R < 2 || R > 36)
            return Value::fromDouble(nan("")); // 8a
        if (R != 16)
            stripPrefix = false; // 8b
    } else { // 9
        R = 10; // 9a
    }
    if (stripPrefix) { // 10
        if ((end - pos >= 2)
                && (pos[0] == QLatin1Char('0'))
                && (pos[1] == QLatin1Char('x') || pos[1] == QLatin1Char('X'))) { // 10a
            pos += 2;
            R = 16;
        }
    }
    // 11: Z is progressively built below
    // 13: this is handled by the toInt function
    if (pos == end) // 12
        return Value::fromDouble(nan(""));
    bool overflow = false;
    qint64 v_overflow;
    unsigned overflow_digit_count = 0;
    int d = toInt(*pos++, R);
    if (d == -1)
        return Value::fromDouble(nan(""));
    qint64 v = d;
    while (pos != end) {
        d = toInt(*pos++, R);
        if (d == -1)
            break;
        if (overflow) {
            if (overflow_digit_count == 0) {
                v_overflow = v;
                v = 0;
            }
            ++overflow_digit_count;
            v = v * R + d;
        } else {
            qint64 vNew = v * R + d;
            if (vNew < v) {
                overflow = true;
                --pos;
            } else {
                v = vNew;
            }
        }
    }

    if (overflow) {
        double result = (double) v_overflow * pow(R, overflow_digit_count);
        result += v;
        return Value::fromDouble(sign * result);
    } else {
        return Value::fromDouble(sign * (double) v); // 15
    }
}

// parseFloat [15.1.2.3]
Value GlobalFunctions::method_parseFloat(ExecutionContext *context)
{
    Value string = context->argument(0);

    // [15.1.2.3] step by step:
    String *inputString = string.toString(context); // 1
    QString trimmed = inputString->toQString().trimmed(); // 2

    // 4:
    if (trimmed.startsWith(QLatin1String("Infinity"))
            || trimmed.startsWith(QLatin1String("+Infinity")))
        return Value::fromDouble(INFINITY);
    if (trimmed.startsWith("-Infinity"))
        return Value::fromDouble(-INFINITY);
    QByteArray ba = trimmed.toLatin1();
    bool ok;
    const char *begin = ba.constData();
    const char *end = 0;
    double d = qstrtod(begin, &end, &ok);
    if (end - begin == 0)
        return Value::fromDouble(nan("")); // 3
    else
        return Value::fromDouble(d);
}

/// isNaN [15.1.2.4]
Value GlobalFunctions::method_isNaN(ExecutionContext *context)
{
    const Value &v = context->argument(0);
    if (v.integerCompatible())
        return Value::fromBoolean(false);

    double d = v.toNumber(context);
    return Value::fromBoolean(std::isnan(d));
}

/// isFinite [15.1.2.5]
Value GlobalFunctions::method_isFinite(ExecutionContext *context)
{
    const Value &v = context->argument(0);
    if (v.integerCompatible())
        return Value::fromBoolean(true);

    double d = v.toNumber(context);
    return Value::fromBoolean(std::isfinite(d));
}

/// decodeURI [15.1.3.1]
Value GlobalFunctions::method_decodeURI(ExecutionContext *context)
{
    if (context->argumentCount == 0)
        return Value::undefinedValue();

    QString uriString = context->argument(0).toString(context)->toQString();
    bool ok;
    QString out = decode(uriString, QString::fromUtf8(uriReserved) + QString::fromUtf8("#"), &ok);
    if (!ok)
        context->throwURIError(Value::fromString(context, QStringLiteral("malformed URI sequence")));

    return Value::fromString(context, out);
}

/// decodeURIComponent [15.1.3.2]
Value GlobalFunctions::method_decodeURIComponent(ExecutionContext *context)
{
    if (context->argumentCount == 0)
        return Value::undefinedValue();

    QString uriString = context->argument(0).toString(context)->toQString();
    bool ok;
    QString out = decode(uriString, QString(), &ok);
    if (!ok)
        context->throwURIError(Value::fromString(context, QStringLiteral("malformed URI sequence")));

    return Value::fromString(context, out);
}

/// encodeURI [15.1.3.3]
Value GlobalFunctions::method_encodeURI(ExecutionContext *context)
{
    if (context->argumentCount == 0)
        return Value::undefinedValue();

    QString uriString = context->argument(0).toString(context)->toQString();
    bool ok;
    QString out = encode(uriString, QLatin1String(uriReserved) + QLatin1String(uriUnescaped) + QString::fromUtf8("#"), &ok);
    if (!ok)
        context->throwURIError(Value::fromString(context, QStringLiteral("malformed URI sequence")));

    return Value::fromString(context, out);
}

/// encodeURIComponent [15.1.3.4]
Value GlobalFunctions::method_encodeURIComponent(ExecutionContext *context)
{
    if (context->argumentCount == 0)
        return Value::undefinedValue();

    QString uriString = context->argument(0).toString(context)->toQString();
    bool ok;
    QString out = encode(uriString, QString(uriUnescaped), &ok);
    if (!ok)
        context->throwURIError(Value::fromString(context, QStringLiteral("malformed URI sequence")));

    return Value::fromString(context, out);
}

Value GlobalFunctions::method_escape(ExecutionContext *context)
{
    if (!context->argumentCount)
        return Value::fromString(context, QStringLiteral("undefined"));

    QString str = context->argument(0).toString(context)->toQString();
    return Value::fromString(context, escape(str));
}

Value GlobalFunctions::method_unescape(ExecutionContext *context)
{
    if (!context->argumentCount)
        return Value::fromString(context, QStringLiteral("undefined"));

    QString str = context->argument(0).toString(context)->toQString();
    return Value::fromString(context, unescape(str));
}
