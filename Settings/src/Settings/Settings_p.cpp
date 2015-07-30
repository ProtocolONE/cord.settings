#include <Settings/Settings_p.h>

#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtCore/QStringList>
#include <QtCore/QStack>
#include <QtCore/QDataStream>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>
#include <QSqlDatabase>

namespace GGS {
    namespace Settings {

        QString SettingsPrivate::connection = QString("");
        QString SettingsPrivate::table  = QString("app_settings");
        QString SettingsPrivate::keyColumn  = QString("key_column");
        QString SettingsPrivate::valueColumn  = QString("value_column");

        QString SettingsPrivate::actualKey(const QString &key) const
        {
            QString n = normalizedKey(key);
            Q_ASSERT_X(!n.isEmpty(), "QSettings", "empty key");
            n.prepend(groupPrefix);
            return n;
        }

        /*
        Returns a string that never starts nor ends with a slash (or an
        empty string). Examples:

        "foo"            becomes   "foo"
        "/foo//bar///"   becomes   "foo/bar"
        "///"            becomes   ""

        This function is optimized to avoid a QString deep copy in the
        common case where the key is already normalized.
        */
        QString SettingsPrivate::normalizedKey(const QString &key) const
        {
            QString result = key;

            int i = 0;
            while (i < result.size()) {
                while (result.at(i) == QLatin1Char('/')) {
                    result.remove(i, 1);
                    if (i == result.size()){
                      if (!result.isEmpty())
                          result.truncate(i - 1); // remove the trailing slash
                      return result;
                    }
                }
                while (result.at(i) != QLatin1Char('/')) {
                    ++i;
                    if (i == result.size())
                        return result;
                }
                ++i; // leave the slash alone
            }

            if (!result.isEmpty())
                result.truncate(i - 1); // remove the trailing slash
            return result;
        }

        void SettingsPrivate::processChild(QString key, ChildSpec spec, QMap<QString, QString> &result) const
        {
            if (spec != AllKeys) {
                int slashPos = key.indexOf(QLatin1Char('/'));
                if (slashPos == -1) {
                    if (spec != ChildKeys)
                        return;
                } else {
                    if (spec != ChildGroups)
                        return;
                    key.truncate(slashPos);
                }
            }
            result.insert(key, QString());
        }

        void SettingsPrivate::beginGroupOrArray(const QSettingsGroup &group)
        {
            groupStack.push(group);
            if (!group.name().isEmpty()) {
                groupPrefix += group.name();
                groupPrefix += QLatin1Char('/');
            }
        }

        QStringList SettingsPrivate::variantListToStringList(const QVariantList &l) const
        {
            QStringList result;
            QVariantList::const_iterator it = l.constBegin();
            for (; it != l.constEnd(); ++it)
                result.append(variantToString(*it));
            return result;
        }

        QVariant SettingsPrivate::stringListToVariantList(const QStringList &l) const
        {
            QStringList outStringList = l;
            for (int i = 0; i < outStringList.count(); ++i) {
                const QString &str = outStringList.at(i);

                if (str.startsWith(QLatin1Char('@'))) {
                    if (str.length() >= 2 && str.at(1) == QLatin1Char('@')) {
                        outStringList[i].remove(0, 1);
                    } else {
                        QVariantList variantList;
                        for (int j = 0; j < l.count(); ++j)
                            variantList.append(stringToVariant(l.at(j)));
                        return variantList;
                    }
                }
            }
            return outStringList;
        }

        QString SettingsPrivate::variantToString(const QVariant &v) const
        {
            QString result;

            switch (v.type()) {
            case QVariant::Invalid:
                result = QLatin1String("@Invalid()");
                break;

            case QVariant::ByteArray: {
                QByteArray a = v.toByteArray();
                result = QLatin1String("@ByteArray(");
                result += a.toHex();
                result += QLatin1Char(')');
                break;
                                      }

            case QVariant::String:
            case QVariant::LongLong:
            case QVariant::ULongLong:
            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::Bool:
            case QVariant::Double:
            case QVariant::KeySequence: {
                result = v.toString();
                if (result.startsWith(QLatin1Char('@')))
                    result.prepend(QLatin1Char('@'));
                break;
                                        }
#ifndef QT_NO_GEOM_VARIANT
            case QVariant::Rect: {
                QRect r = qvariant_cast<QRect>(v);
                result += QLatin1String("@Rect(");
                result += QString::number(r.x());
                result += QLatin1Char(' ');
                result += QString::number(r.y());
                result += QLatin1Char(' ');
                result += QString::number(r.width());
                result += QLatin1Char(' ');
                result += QString::number(r.height());
                result += QLatin1Char(')');
                break;
                                 }
            case QVariant::Size: {
                QSize s = qvariant_cast<QSize>(v);
                result += QLatin1String("@Size(");
                result += QString::number(s.width());
                result += QLatin1Char(' ');
                result += QString::number(s.height());
                result += QLatin1Char(')');
                break;
                                 }
            case QVariant::Point: {
                QPoint p = qvariant_cast<QPoint>(v);
                result += QLatin1String("@Point(");
                result += QString::number(p.x());
                result += QLatin1Char(' ');
                result += QString::number(p.y());
                result += QLatin1Char(')');
                break;
                                  }
#endif // !QT_NO_GEOM_VARIANT

            default: {
#ifndef QT_NO_DATASTREAM
                QByteArray a;
                {
                    QDataStream s(&a, QIODevice::WriteOnly);
                    s.setVersion(QDataStream::Qt_4_0);
                    s << v;
                }

                result = QLatin1String("@Variant(");
                result += a.toHex();
                result += QLatin1Char(')');
#else
                Q_ASSERT(!"QSettings: Cannot save custom types without QDataStream support");
#endif
                break;
                     }
            }

            return result;
        }

        QVariant SettingsPrivate::stringToVariant(const QString &s) const
        {
            if (s.startsWith(QLatin1Char('@'))) {
                if (s.endsWith(QLatin1Char(')'))) {
                    if (s.startsWith(QLatin1String("@ByteArray("))) {
                        QByteArray a(s.toLatin1().mid(11, s.size() - 12));
                        return QVariant(QByteArray::fromHex(a));
                    } else if (s.startsWith(QLatin1String("@Variant("))) {
#ifndef QT_NO_DATASTREAM
                        QByteArray a(s.toLatin1().mid(9, s.size() - 10));
                        a = QByteArray::fromHex(a);
                        QDataStream stream(&a, QIODevice::ReadOnly);
                        stream.setVersion(QDataStream::Qt_4_0);
                        QVariant result;
                        stream >> result;
                        return result;
#else
                        Q_ASSERT(!"QSettings: Cannot load custom types without QDataStream support");
#endif
#ifndef QT_NO_GEOM_VARIANT
                    } else if (s.startsWith(QLatin1String("@Rect("))) {
                        QStringList args = SettingsPrivate::splitArgs(s, 5);
                        if (args.size() == 4)
                            return QVariant(QRect(args[0].toInt(), args[1].toInt(), args[2].toInt(), args[3].toInt()));
                    } else if (s.startsWith(QLatin1String("@Size("))) {
                        QStringList args = SettingsPrivate::splitArgs(s, 5);
                        if (args.size() == 2)
                            return QVariant(QSize(args[0].toInt(), args[1].toInt()));
                    } else if (s.startsWith(QLatin1String("@Point("))) {
                        QStringList args = SettingsPrivate::splitArgs(s, 6);
                        if (args.size() == 2)
                            return QVariant(QPoint(args[0].toInt(), args[1].toInt()));
#endif
                    } else if (s == QLatin1String("@Invalid()")) {
                        return QVariant();
                    }

                }
                if (s.startsWith(QLatin1String("@@")))
                    return QVariant(s.mid(1));
            }

            return QVariant(s);
        }

        QStringList SettingsPrivate::splitArgs(const QString &s, int index) const
        {
            int l = s.length();
            Q_ASSERT(l > 0);
            Q_ASSERT(s.at(index) == QLatin1Char('('));
            Q_ASSERT(s.at(l - 1) == QLatin1Char(')'));

            QStringList result;
            QString item;

            for (++index; index < l; ++index) {
                QChar c = s.at(index);
                if (c == QLatin1Char(')')) {
                    Q_ASSERT(index == l - 1);
                    result.append(item);
                } else if (c == QLatin1Char(' ')) {
                    result.append(item);
                    item.clear();
                } else {
                    item.append(c);
                }
            }

            return result;
        }

        QStringList SettingsPrivate::children(const QString &prefix, ChildSpec spec) const
        {
            Q_ASSERT(!SettingsPrivate::connection.isEmpty());
            QSqlDatabase db = QSqlDatabase::database(connection);
            QSqlQuery sqlQuery(db);
            QMap<QString, QString> result;

            QString query = QString("SELECT %2 FROM %1").arg( db.driver()->escapeIdentifier(table, QSqlDriver::TableName)
                ,db.driver()->escapeIdentifier(keyColumn, QSqlDriver::FieldName));

            if (!(sqlQuery.exec(query)))
            {
                qWarning() << Q_FUNC_INFO;
                qWarning() << sqlQuery.lastError().text();

                return result.keys();
            }  

            int startPos = prefix.size();

            while (sqlQuery.next())
            {
                QString key = sqlQuery.value(0).toString();

                if (key.startsWith(prefix))
                    processChild(key.mid(startPos), spec, result);
            }

            return result.keys();
        }

        void SettingsPrivate::setConnection( const QString& _connection )
        {
            connection = _connection;
        }

    }
}