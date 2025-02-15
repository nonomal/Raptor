﻿/**
 *
 * Copyright (C) 2024 凉州刺史. All rights reserved.
 *
 * This file is part of Raptor.
 *
 * $RAPTOR_BEGIN_LICENSE$
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl.html.
 *
 * $RAPTOR_END_LICENSE$
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include "RaptorUtil.h"

QString RaptorUtil::invoke3rdPartyDownloaderEvoke(const QString& qUrl,
                                                  const QString& qDir,
                                                  const QString& qName)
{
    auto qProgram = QString();
    auto qArgs = QStringList();
    const auto qEngines = RaptorSettingSuite::invokeItemFind(Setting::Section::Download,
                                                             Setting::Play::Engine).value<QVector<RaptorSettingItem>>();
    const auto qEngine = RaptorSettingSuite::invokeItemFind(Setting::Section::Download,
                                                            Setting::Play::ActiveEngine).toString();
    for (auto& item : qAsConst(qEngines))
    {
        if (item._Name == qEngine)
        {
            if (!QFileInfo::exists(item._Path))
            {
                return QStringLiteral("请先设置 %1 路径!").arg(QStringLiteral(WARNING_TEMPLATE).arg(qEngine));
            }

            qProgram = item._Path;
            if (qEngine == Setting::Download::IDM)
            {
                qArgs << "/d";
                qArgs << qUrl;
                qArgs << "/p";
                qArgs << qDir;
                qArgs << "/f";
                qArgs << qName;
            }
            else if (qEngine == Setting::Download::FDM)
            {
                qArgs << qUrl;
            }
        }
    }

    QProcess::startDetached(qProgram, qArgs);
    return QString();
}

QString RaptorUtil::invoke3rdPartyPlayerEvoke(const QString& qUrl)
{
    auto qProgram = QString();
    const auto qEngines = RaptorSettingSuite::invokeItemFind(Setting::Section::Play,
                                                             Setting::Play::Engine).value<QVector<RaptorSettingItem>>();
    const auto qEngine = RaptorSettingSuite::invokeItemFind(Setting::Section::Play,
                                                            Setting::Play::ActiveEngine).toString();
    for (auto& item : qAsConst(qEngines))
    {
        if (item._Name == qEngine)
        {
            if (!QFileInfo::exists(item._Path))
            {
                return QStringLiteral("请先设置 %1 路径!").arg(QStringLiteral(WARNING_TEMPLATE).arg(qEngine));
            }

            qProgram = item._Path;
        }
    }

    auto qArgs = QStringList();
    qArgs << QStringLiteral("%1").arg(qUrl);
    QProcess::startDetached(qProgram, qArgs);
    return QString();
}

QVector<QString> RaptorUtil::invokeFontLoad()
{
    const auto qDir = QDir(QStringLiteral("./Store/Font"));
    const auto items = qDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    auto itens = QVector<QString>();
    for (auto& item : items)
    {
        if (item.completeSuffix() == "ttf" || item.completeSuffix() == "ttc")
        {
            itens << item.absoluteFilePath();
        }
    }

    return itens;
}

QQueue<RaptorTimelineItem> RaptorUtil::invokeTimelineLoad()
{
    auto qSQLQuery = RaptorPersistenceSuite::invokeQueryGenerate();
    const auto qSQL = R"(
        SELECT * FROM Timeline ORDER BY STRFTIME('%Y-%m-%d', Date) DESC
    )";
    qSQLQuery.exec(qSQL);
    auto items = QQueue<RaptorTimelineItem>();
    while (qSQLQuery.next())
    {
        auto item = RaptorTimelineItem();
        item._Active = qSQLQuery.value("Active").toInt() == 1;
        item._Date = qSQLQuery.value("Date").toString();
        item._Summary = qSQLQuery.value("Summary").toString();
        item._Initial = qSQLQuery.value("Initial").toInt() == 1;
        items.enqueue(item);
    }

    return items;
}

QString RaptorUtil::invokeStyleSheetLoad(const QString& qValue)
{
    const auto qRoot = sass_make_file_context(QStringLiteral("./Store/Style/%1/%2.sass").arg(qValue, APPLICATION_NAME).toStdString().data());
    const auto qContext = sass_file_context_get_context(qRoot);
    const auto qOption = sass_context_get_options(qContext);
    sass_option_set_precision(qOption, 2);
    sass_option_set_output_style(qOption, SASS_STYLE_COMPACT);
    sass_compile_file_context(qRoot);
    if (sass_context_get_error_status(qContext) != 0)
    {
        qFatal(sass_context_get_error_message(qContext));
        return QString();
    }

    const auto qStyleSheet = QString::fromStdString(sass_context_get_output_string(qContext));
    sass_delete_file_context(qRoot);
    return qStyleSheet;
}

QString RaptorUtil::invokeCompileTimestampCompute()
{
    return QLocale(QLocale::English).toDate(QString(__DATE__).simplified(), "MMM d yyyy").toString(Qt::ISODate) + QStringLiteral(" ") + QStringLiteral(__TIME__);
}

QSize RaptorUtil::invokePrimaryDesktopGeometryCompute()
{
    return qApp->primaryScreen()->geometry().size();
}

QPixmap RaptorUtil::invokeShadowGenerate(const QSize& qSize,
                                         const quint16& qMargin,
                                         const quint16& qRadius,
                                         const QColor& qColor)
{
    auto qImage = QImage(qSize + QSize(qMargin * 2, qMargin * 2), QImage::Format_ARGB32);
    const auto qRect = QRect(QPoint(qMargin, qMargin), qSize);
    qImage.fill(Qt::transparent);
    auto qPainter = QPainter(&qImage);
    qPainter.fillRect(qRect, qColor);
    qPainter.end();
    RaptorBlurSuite::invokeExponentialBlur(12, 10, qImage, qRadius);

    auto qPaintes = QPainter{&qImage};
    qPaintes.setCompositionMode(QPainter::CompositionMode_SourceIn);
    qPaintes.fillRect(qImage.rect(), qColor);
    qPaintes.fillRect(QRect(qRect.topLeft(), qSize), Qt::transparent);
    qPaintes.setCompositionMode(QPainter::CompositionMode_SourceOver);
    qPaintes.end();
    return QPixmap::fromImage(qImage);
}

void RaptorUtil::invokeItemLocate(const QString& qPath)
{
    const auto qProgram = "explorer";
    auto qArgs = QStringList();
    qArgs << "/select,";
    qArgs << QDir::toNativeSeparators(qPath);
    QProcess::startDetached(qProgram, qArgs);
}

bool RaptorUtil::invokeSystemDarkThemeConfirm()
{
    const auto qSettings = QSettings(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)", QSettings::NativeFormat);
    return qSettings.value("AppsUseLightTheme").toInt() == 0;
}


QString RaptorUtil::invokeIconMatch(const QString& qName,
                                    const bool& qFolder,
                                    const bool& qUi)
{
    if (qFolder)
    {
        return QStringLiteral("./Store/Icon/Mime/Raptor[FOLDER].svg");
    }

    if (qUi)
    {
        if (const auto qIcon = QStringLiteral("./Store/Icon/Ui/Raptor[%1].svg").arg(qName.toUpper());
            QFile::exists(qIcon))
        {
            return QStringLiteral("./Store/Icon/Ui/Raptor[%1].svg").arg(qName.toUpper());
        }

        return QStringLiteral("./Store/Icon/Ui/Raptor[%1].png").arg(qName.toUpper());
    }

    if (qName == "SHARE")
    {
        return QStringLiteral("./Store/Icon/Mime/Raptor[SHARE].svg");
    }

    // 尝试匹配特殊无后缀的名称。如 Dockerfile
    if (const auto qIcon = QStringLiteral("./Store/Icon/Mime/Raptor[%1].svg").arg(qName.toUpper());
        QFile::exists(qIcon))
    {
        return qIcon;
    }

    if (const auto qLastDotIndex = qName.lastIndexOf(".");
        qLastDotIndex != -1)
    {
        if (const auto qIcon = QStringLiteral("./Store/Icon/Mime/Raptor[%1].svg").arg(qName.mid(qLastDotIndex + 1).toUpper());
            QFile::exists(qIcon))
        {
            return qIcon;
        }

        return QStringLiteral("./Store/Icon/Mime/Raptor[BINARY].svg");
    }

    // 不存在后缀
    return QStringLiteral("./Store/Icon/Mime/Raptor[BINARY].svg");
}

QString RaptorUtil::invoke1024SHA1Compute(const QString& qName)
{
    auto q1024SHA1 = QString();
    auto qFile = QFile(qName);
    if (!qFile.open(QIODevice::ReadOnly))
    {
        return q1024SHA1;
    }

    auto qHash = QCryptographicHash(QCryptographicHash::Sha1);
    qHash.addData(qFile.read(1024));
    qFile.close();
    q1024SHA1 = QString::fromUtf8(qHash.result().toHex());
    return q1024SHA1;
}

QString RaptorUtil::invokeSHA1Compute(const QString& qName)
{
    auto qSHA1 = QString();
    auto qFile = QFile(qName);
    if (!qFile.open(QIODevice::ReadOnly))
    {
        return qSHA1;
    }

    auto qHash = QCryptographicHash(QCryptographicHash::Sha1);
    while (!qFile.atEnd())
    {
        qHash.addData(qFile.read(64 * 1024));
    }

    qFile.close();
    qSHA1 = QString::fromUtf8(qHash.result().toHex());
    return qSHA1;
}

quint64 RaptorUtil::invokeChunkCompute(const quint64& qValue)
{
    // 256 MB
    auto qChunk = 268435456;
    // (1024 * 1024 * 10) * 10000
    if (qValue < 104857600000)
    {
        // (1024 * 1024 * 1024) 10 MB
        qChunk = 10485760;
    }

    return qChunk;
}

QJsonArray RaptorUtil::invokeItemPartialCompute(const QString& qName)
{
    const auto qSize = static_cast<quint64>(QFileInfo(qName).size());
    auto items = QJsonArray();
    if (const auto qChunk = invokeChunkCompute(qSize); qSize > qChunk)
    {
        const auto qCount = qCeil(qSize / qChunk);
        for (auto i = 1; i < qCount; ++i)
        {
            auto item = QJsonObject();
            item["part_number"] = i;
            items << item;
        }
    }
    else
    {
        auto item = QJsonObject();
        item["part_number"] = 1;
        items << item;
    }

    return items;
}

QString RaptorUtil::invokeUUIDGenerate()
{
    return QUuid::createUuid().toString().remove(QRegExp("[{}-]"));
}

QString RaptorUtil::invokePasswordGenerate()
{
    auto qPassword = QString();
    qPassword.append(QChar('a' + QRandomGenerator::global()->bounded(26)));
    qPassword.append(QChar('a' + QRandomGenerator::global()->bounded(26)));
    qPassword.append(QChar('0' + QRandomGenerator::global()->bounded(10)));
    qPassword.append(QChar('0' + QRandomGenerator::global()->bounded(10)));
    std::shuffle(qPassword.begin(), qPassword.end(), std::mt19937(std::random_device()()));
    return qPassword;
}

RaptorSession RaptorUtil::invokeDeviceSignature(const RaptorAuthenticationItem& item)
{
    const auto qDeviceId = item._Session._Device.isNull()
                               ? invokeUUIDGenerate()
                               : item._Session._Device;
    const auto qNonce = invokeUUIDGenerate();
    const auto qContent = QCryptographicHash::hash(QStringLiteral("%1:%2:%3:%4").arg(APPLICATION_ID,
                                                                                     qDeviceId,
                                                                                     item._LeafId,
                                                                                     qNonce).toUtf8(),
                                                   QCryptographicHash::Sha256);

#if defined(STANDALONE)
    auto qSeed = CryptoPP::AutoSeededRandomPool();
    auto qPrivateKey = CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PrivateKey();
    qPrivateKey.Initialize(qSeed, CryptoPP::DL_GroupParameters_EC<CryptoPP::ECP>(CryptoPP::ASN1::secp256k1()));
    auto qPublicKey = CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey();
    qPrivateKey.MakePublicKey(qPublicKey);
    const auto qPoint = qPublicKey.GetPublicElement();
    auto qSignature = std::string();
    auto qSignatureHex = std::string();
    const auto qSigner = CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::Signer(qPrivateKey);
    CryptoPP::StringSource(qContent.toStdString(), true, new CryptoPP::SignerFilter(qSeed, qSigner, new CryptoPP::StringSink(qSignature)));
    CryptoPP::StringSource(qSignature, true, new CryptoPP::HexEncoder(new CryptoPP::StringSink(qSignatureHex)));
    return RaptorSession(
        qDeviceId,
        QString::fromStdString(qSignatureHex) + "01",
        QString::fromStdString(IntToString(qPrivateKey.GetPrivateExponent())),
        QStringLiteral("04%1%2").arg(QString::fromStdString(IntToString(qPoint.x)), QString::fromStdString(IntToString(qPoint.y)))
    );
#elif defined(INTEGRATED)
    auto qPrivateKey = QByteArray(32, 0);
    auto qPublicKeyBuffer = QByteArray(33, 0);
    auto qSignatureBuffer = QByteArray(64, 0);
    auto qLength = static_cast<size_t>(33);
    auto qPublicKey = secp256k1_pubkey();
    auto qSignature = secp256k1_ecdsa_signature();
    const auto qContext = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
    auto qRandom = QByteArray(32, 0);
    QRandomGenerator::global()->fillRange(reinterpret_cast<quint32*>(qRandom.data()),
                                          qRandom.size() / sizeof(quint32));
    secp256k1_context_randomize(qContext,
                                reinterpret_cast<const unsigned char*>(qRandom.data()));

    do
    {
        if (item._Session._PrivateKey.isEmpty())
        {
            QRandomGenerator::global()->fillRange(reinterpret_cast<quint32*>(qPrivateKey.data()),
                                                  qPrivateKey.size() / sizeof(quint32));
        }
        else
        {
            qPrivateKey = QByteArray::fromHex(item._Session._PrivateKey.toUtf8());
        }

        if (secp256k1_ec_seckey_verify(qContext, reinterpret_cast<const unsigned char*>(qPrivateKey.data())))
        {
            break;
        }
    }
    while (true);

    secp256k1_ec_pubkey_create(qContext,
                               &qPublicKey,
                               reinterpret_cast<const unsigned char*>(qPrivateKey.data()));

    secp256k1_ec_pubkey_serialize(qContext,
                                  reinterpret_cast<unsigned char*>(qPublicKeyBuffer.data()),
                                  &qLength,
                                  &qPublicKey,
                                  SECP256K1_EC_COMPRESSED);

    secp256k1_ecdsa_sign(qContext, &qSignature,
                         reinterpret_cast<unsigned char*>(qContent.toHex().data()),
                         reinterpret_cast<const unsigned char*>(qPrivateKey.data()),
                         Q_NULLPTR,
                         Q_NULLPTR);

    secp256k1_ecdsa_signature_serialize_compact(qContext,
                                                reinterpret_cast<unsigned char*>(qSignatureBuffer.data()),
                                                &qSignature);

    secp256k1_context_destroy(qContext);
    return RaptorSession(
        qDeviceId,
        QByteArray(qSignatureBuffer).toHex() + "01",
        qPrivateKey.toHex(),
        "04" + QByteArray(qPublicKeyBuffer).toHex()
    );
#endif
}

QString RaptorUtil::invokeStorageUnitConvert(const qint64& qValue)
{
    return _StorageUnitLocale.formattedDataSize(qValue, 2, QLocale::DataSizeTraditionalFormat);
}

QString RaptorUtil::invokeTimeUnitConvert(const quint64& qValue)
{
    const auto qDay = qValue / 86400;
    const auto qHour = (qValue % 86400) / 3600;
    const auto qMinute = (qValue % 3600) / 60;
    const auto qSecond = qValue % 60;
    auto qValuf = QString();
    if (qDay > 0)
    {
        qValuf = QStringLiteral("%1 天 %2 小时 %3 分钟 %4 秒").arg(qDay).arg(qHour).arg(qMinute).arg(qSecond);
    }
    else if (qHour > 0)
    {
        qValuf = QStringLiteral("%1 小时 %2 分钟 %3 秒").arg(qHour).arg(qMinute).arg(qSecond);
    }
    else if (qMinute > 0)
    {
        qValuf = QStringLiteral("%1 分钟 %2 秒").arg(qMinute).arg(qSecond);
    }
    else
    {
        qValuf = QStringLiteral("%1 秒").arg(qSecond);
    }

    return qValuf;
}

QString RaptorUtil::invokeTimestampConvert(const quint64& qValue)
{
    Q_DECL_CONSTEXPR auto qSeconds = 60 * 60 * 24 * 365;
    const auto qYear = qValue / qSeconds;
    const auto qDay = (qValue % qSeconds) / (60 * 60 * 24);
    return QStringLiteral("%1 年 %2 天").arg(qYear).arg(qDay);
}
