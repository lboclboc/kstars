/*  Ekos Lin Guider Handler
    Copyright (C) 2016 Jasem Mutlaq <mutlaqja@ikarustech.com>

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
*/

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QUrl>
#include <QVariantMap>
#include <QDebug>
#include <QHttpMultiPart>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>

#include <KMessageBox>
#include <KLocalizedString>

#include "linguider.h"
#include "Options.h"


namespace Ekos
{

LinGuider::LinGuider()
{
    tcpSocket = new QTcpSocket(this);

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readLinGuider()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));

    connect(tcpSocket, SIGNAL(connected()), this, SIGNAL(connected()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));

    methodID=1;
    state = STOPPED;
    connection = DISCONNECTED;
    event = Alert;

    events["Version"] = Version;
    events["LockPositionSet"] = LockPositionSet;
    events["CalibrationComplete"] = CalibrationComplete;
    events["StarSelected"] = StarSelected;
    events["StartGuiding"] = StartGuiding;
    events["Paused"] = Paused;
    events["StartCalibration"] = StartCalibration;
    events["AppState"] = AppState;
    events["CalibrationFailed"] = CalibrationFailed;
    events["CalibrationDataFlipped"] = CalibrationDataFlipped;
    events["LoopingExposures"] = LoopingExposures;
    events["LoopingExposuresStopped"] = LoopingExposuresStopped;
    events["Settling"] = Settling;
    events["SettleDone"] = SettleDone;
    events["StarLost"] = StarLost;
    events["GuidingStopped"] = GuidingStopped;
    events["Resumed"] = Resumed;
    events["GuideStep"] = GuideStep;
    events["GuidingDithered"] = GuidingDithered;
    events["LockPositionLost"] = LockPositionLost;
    events["Alert"] = Alert;

}

LinGuider::~LinGuider()
{

}

bool LinGuider::Connect()
{
    if (connection == DISCONNECTED)
    {
        connection = CONNECTING;
        tcpSocket->connectToHost(Options::linGuiderHost(),  Options::linGuiderPort());
    }
    // Already connected, let's connect equipment
    else
        setEquipmentConnected(true);

    return true;
}

bool LinGuider::Disconnect()
{
   if (connection == EQUIPMENT_CONNECTED)
       setEquipmentConnected(false);

   connection = DISCONNECTED;
   tcpSocket->disconnectFromHost();

   emit newStatus(GUIDE_DISCONNECTED);

   return true;
}

void LinGuider::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        emit newLog(i18n("The host was not found. Please check the host name and port settings in Guide options."));
        emit newStatus(GUIDE_DISCONNECTED);
        break;
    case QAbstractSocket::ConnectionRefusedError:
        emit newLog(i18n("The connection was refused by the peer. Make sure the LinGuider is running, and check that the host name and port settings are correct."));
        emit newStatus(GUIDE_DISCONNECTED);
        break;
    default:
        emit newLog(i18n("The following error occurred: %1.", tcpSocket->errorString()));
    }

    connection = DISCONNECTED;

}

void LinGuider::readLinGuider()
{
    QTextStream stream(tcpSocket);

    QJsonParseError qjsonError;

    while (stream.atEnd() == false)
    {
        QString rawString = stream.readLine();

        if (rawString.isEmpty())
            continue;

        QJsonDocument jdoc = QJsonDocument::fromJson(rawString.toLatin1(), &qjsonError);

        if (qjsonError.error != QJsonParseError::NoError)
        {
            emit newLog(rawString);
            emit newLog(qjsonError.errorString());
            continue;
        }

        emit newLog(rawString);

        processJSON(jdoc.object());
    }

}

void LinGuider::processJSON(const QJsonObject &jsonObj)
{
    LinGuiderMessageType messageType = LINGUIDER_UNKNOWN;
    bool result = false;

    if (jsonObj.contains("Event"))
    {
        messageType = LINGUIDER_EVENT;
        processLinGuiderEvent(jsonObj);

        if (event == Alert)
            return;
    }
    else if (jsonObj.contains("error"))
    {
        messageType = LINGUIDER_ERROR;
        result = false;
        processLinGuiderError(jsonObj);
    }
    else if (jsonObj.contains("result"))
    {
        messageType = LINGUIDER_RESULT;
        result = true;
    }

    switch (connection)
    {
    case CONNECTING:
        if (event == Version)
            connection = CONNECTED;
        return;

    case CONNECTED:
        // If initial state is STOPPED, let us connect equipment
        if (state == STOPPED)
            setEquipmentConnected(true);
        else if (state == GUIDING)
        {
            connection = EQUIPMENT_CONNECTED;
            emit newStatus(Ekos::GUIDE_CONNECTED);
        }
        return;

    case DISCONNECTED:
        emit newStatus(Ekos::GUIDE_DISCONNECTED);
        break;

    case EQUIPMENT_CONNECTING:
        if (messageType == LINGUIDER_RESULT)
        {
            if (result)
            {
                connection = EQUIPMENT_CONNECTED;
                emit newStatus(Ekos::GUIDE_CONNECTED);
            }
            else
            {
                connection = EQUIPMENT_DISCONNECTED;
                emit newStatus(Ekos::GUIDE_DISCONNECTED);
            }
        }
        return;

    case EQUIPMENT_CONNECTED:
    case EQUIPMENT_DISCONNECTED:
        break;

    case EQUIPMENT_DISCONNECTING:
        connection = EQUIPMENT_DISCONNECTED;
        //emit disconnected();
        return;
    }

    switch (state)
    {
    case GUIDING:
        break;

    case PAUSED:
        break;

    case STOPPED:
        break;

    default:
        break;
    }
}

void LinGuider::processLinGuiderEvent(const QJsonObject &jsonEvent)
{
    QString eventName = jsonEvent["Event"].toString();

    if (events.contains(eventName) == false)
    {
        emit newLog(i18n("Unknown LinGuider event: %1", eventName));
        return;
    }

    event = events.value(eventName);

    switch (event)
    {
    case Version:
        emit newLog(i18n("LinGuider: Version %1", jsonEvent["PHDVersion"].toString()));
        break;

    case CalibrationComplete:
        //state = CALIBRATION_SUCCESSFUL;
        // It goes immediately to guiding until PHD implements a calibration-only method
        state = GUIDING;
        emit newLog(i18n("LinGuider: Calibration Complete."));
        //emit guideReady();
        emit newStatus(Ekos::GUIDE_CALIBRATION_SUCESS);
        break;

    case StartGuiding:
        state = GUIDING;
        if (connection != EQUIPMENT_CONNECTED)
        {
            connection = EQUIPMENT_CONNECTED;
            emit newStatus(Ekos::GUIDE_CONNECTED);
        }
        emit newLog(i18n("LinGuider: Guiding Started."));
        emit newStatus(Ekos::GUIDE_GUIDING);
        break;

    case Paused:
        state = PAUSED;
        emit newLog(i18n("LinGuider: Guiding Paused."));
        emit newStatus(Ekos::GUIDE_SUSPENDED);
        break;

    case StartCalibration:
        state = CALIBRATING;
        emit newLog(i18n("LinGuider: Calibration Started."));
        emit newStatus(Ekos::GUIDE_CALIBRATING);
        break;

    case AppState:
        processLinGuiderState(jsonEvent["State"].toString());
        break;

    case CalibrationFailed:
        state = CALIBRATION_FAILED;
        emit newLog(i18n("LinGuider: Calibration Failed (%1).", jsonEvent["Reason"].toString()));
        emit newStatus(Ekos::GUIDE_CALIBRATION_ERROR);
        break;

    case CalibrationDataFlipped:
        emit newLog(i18n("Calibration Data Flipped."));
        break;

    case LoopingExposures:
        //emit newLog(i18n("LinGuider: Looping Exposures."));
        break;

    case LoopingExposuresStopped:
        emit newLog(i18n("LinGuider: Looping Exposures Stopped."));
        break;

    case Settling:
        break;

    case SettleDone:
    {
        bool error=false;

        if (jsonEvent["Status"].toInt() != 0)
        {
            error = true;
            emit newLog(i18n("LinGuider: Settling failed (%1).", jsonEvent["Error"].toString()));
        }

        if (state == GUIDING)
        {
            if (error)
                state = STOPPED;
        }
        else if (state == DITHERING)
        {
            if (error)
            {
                state = DITHER_FAILED;
                //emit ditherFailed();
                emit newStatus(GUIDE_DITHERING_ERROR);
            }
            else
            {
                state = DITHER_SUCCESSFUL;
                emit newStatus(Ekos::GUIDE_DITHERING_SUCCESS);
            }
        }
    }
    break;

    case StarSelected:
        emit newLog(i18n("LinGuider: Star Selected."));
        break;

    case StarLost:
        emit newLog(i18n("LinGuider: Star Lost."));
        emit newStatus(Ekos::GUIDE_ABORTED);
        break;

    case GuidingStopped:
        emit newLog(i18n("LinGuider: Guiding Stopped."));
        //emit autoGuidingToggled(false);
        emit newStatus(Ekos::GUIDE_IDLE);
        break;

    case Resumed:
        emit newLog(i18n("LinGuider: Guiding Resumed."));
        emit newStatus(Ekos::GUIDE_GUIDING);
        state = GUIDING;
        break;

    case GuideStep:
    {
        double diff_ra_pixels, diff_de_pixels, diff_ra_arcsecs, diff_de_arcsecs;
        diff_ra_pixels = jsonEvent["RADistanceRaw"].toDouble();
        diff_de_pixels = jsonEvent["DECDistanceRaw"].toDouble();

        diff_ra_arcsecs = 206.26480624709 * diff_ra_pixels * ccdPixelSizeX / mountFocalLength;
        diff_de_arcsecs = 206.26480624709 * diff_de_pixels * ccdPixelSizeY / mountFocalLength;

        emit newAxisDelta(diff_ra_arcsecs, diff_de_arcsecs);
    }
        break;

    case GuidingDithered:
        emit newLog(i18n("LinGuider: Guide Dithering."));
        emit newStatus(Ekos::GUIDE_DITHERING);
        break;

    case LockPositionSet:
        emit newLog(i18n("LinGuider: Lock Position Set."));
        break;

    case LockPositionLost:
        emit newLog(i18n("LinGuider: Lock Position Lost."));
        emit newStatus(Ekos::GUIDE_CALIBRATION_ERROR);
        break;

    case Alert:
        emit newLog(i18n("LinGuider %1: %2", jsonEvent["Type"].toString(), jsonEvent["Msg"].toString()));
        break;

    }
}

void LinGuider::processLinGuiderState(const QString &LinGuiderState)
{
    if (LinGuiderState == "Stopped")
        state = STOPPED;
    else if (LinGuiderState == "Selected")
        state = SELECTED;
    else if (LinGuiderState == "Calibrating")
        state = CALIBRATING;
    else if (LinGuiderState == "GUIDING")
        state = GUIDING;
    else if (LinGuiderState == "LostLock")
        state = LOSTLOCK;
    else if (LinGuiderState == "Paused")
        state = PAUSED;
    else if (LinGuiderState == "Looping")
        state = LOOPING;
}

void LinGuider::processLinGuiderError(const QJsonObject &jsonError)
{
    QJsonObject jsonErrorObject = jsonError["error"].toObject();

    emit newLog(i18n("LinGuider Error: %1", jsonErrorObject["message"].toString()));

   /* switch (connection)
    {
        case CONNECTING:
        case CONNECTED:
            emit disconnected();
        break;

        default:
            break;
    }*/
}

void LinGuider::sendJSONRPCRequest(const QString & method, const QJsonArray args)
{

    QJsonObject jsonRPC;

    jsonRPC.insert("jsonrpc", "2.0");
    jsonRPC.insert("method", method);

    if (args.empty() == false)
        jsonRPC.insert("params", args);

    jsonRPC.insert("id", methodID++);

    QJsonDocument json_doc(jsonRPC);

    emit newLog(json_doc.toJson(QJsonDocument::Compact));

    tcpSocket->write(json_doc.toJson(QJsonDocument::Compact));
    tcpSocket->write("\r\n");
}

void LinGuider::setEquipmentConnected(bool enable)
{
    if ( (connection == EQUIPMENT_CONNECTED && enable == true) || (connection == EQUIPMENT_DISCONNECTED && enable == false) )
        return;

    if (enable)
        connection = EQUIPMENT_CONNECTING;
    else
        connection = EQUIPMENT_DISCONNECTING;

    QJsonArray args;

    // connected = enable
    args << enable;

    sendJSONRPCRequest("set_connected", args);
}

bool LinGuider::calibrate()
{
    // We don't explicitly do calibration since it is done in the guide step by LinGuider anyway
    emit newStatus(Ekos::GUIDE_CALIBRATION_SUCESS);
    return true;
}

bool LinGuider::guide()
{
    if (connection != EQUIPMENT_CONNECTED)
    {
        emit newLog(i18n("LinGuider Error: Equipment not connected."));
        return false;
    }

    QJsonArray args;
    QJsonObject settle;

    settle.insert("pixels", 1.5);
    settle.insert("time", 8);
    settle.insert("timeout", 45);

    // Settle param
    args << settle;
    // Recalibrate param
    args << false;

    sendJSONRPCRequest("guide", args);

    return true;
}

bool LinGuider::abort()
{
    if (connection != EQUIPMENT_CONNECTED)
    {
        emit newLog(i18n("LinGuider Error: Equipment not connected."));
        return false;
    }

   sendJSONRPCRequest("stop_capture");
   return true;
}

bool LinGuider::suspend()
{
    if (connection != EQUIPMENT_CONNECTED)
    {
        emit newLog(i18n("LinGuider Error: Equipment not connected."));
        return false;
    }

    QJsonArray args;

    // Paused param
    args << true;
    // FULL param
    args << "full";

    sendJSONRPCRequest("set_paused", args);

    return true;

}

bool LinGuider::resume()
{
    if (connection != EQUIPMENT_CONNECTED)
    {
        emit newLog(i18n("LinGuider Error: Equipment not connected."));
        return false;
    }

    QJsonArray args;

    // Paused param
    args << false;

    sendJSONRPCRequest("set_paused", args);

    return true;

}

bool LinGuider::dither(double pixels)
{
    if (connection != EQUIPMENT_CONNECTED)
    {
        emit newLog(i18n("LinGuider Error: Equipment not connected."));
        return false;
    }

    QJsonArray args;
    QJsonObject settle;

    settle.insert("pixels", 1.5);
    settle.insert("time", 8);
    settle.insert("timeout", 45);

    // Pixels
    args << pixels;
    // RA Only?
    args << false;
    // Settle
    args << settle;

    state = DITHERING;

    sendJSONRPCRequest("dither", args);

    return true;
}

}
