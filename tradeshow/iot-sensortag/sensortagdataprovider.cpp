/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of Qt for Device Creation.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "sensortagdataprovider.h"

#include <QtCore/QtMath>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(boot2QtDemos)
#define DEFAULT_REFRESH_INTERVAL_MS 1000

SensorTagDataProvider::SensorTagDataProvider(QObject *parent)
    : QObject(parent)
{

}

SensorTagDataProvider::SensorTagDataProvider(QString id, QObject* parent)
    : QObject(parent),
    humidity(0),
    irAmbientTemperature(0),
    irObjectTemperature(0),
    lightIntensityLux(0),
    barometerCelsiusTemperature(0),
    barometerHPa(0),
    gyroscopeX_degPerSec(0),
    gyroscopeY_degPerSec(0),
    gyroscopeZ_degPerSec(0),
    accelometerX(0),
    accelometerY(0),
    accelometerZ(0),
    /* Object is not "walking in the air" so have one axis at 1G */
    magnetometerMicroT_xAxis(-1),
    magnetometerMicroT_yAxis(0),
    magnetometerMicroT_zAxis(0),
    rotation_x(0),
    rotation_y(0),
    rotation_z(0),
    intervalRotation(DEFAULT_REFRESH_INTERVAL_MS),
    m_tagType(AmbientTemperature | ObjectTemperature | Humidity | AirPressure | Light | Magnetometer | Rotation | Accelometer),
    m_id(id),
    m_state(Disconnected)
{
}

QString SensorTagDataProvider::getRelativeHumidityString()
{
    return QString::number(humidity) + QLatin1String("%");
}

double SensorTagDataProvider::getRelativeHumidity()
{
    return humidity;
}

double SensorTagDataProvider::getInfraredAmbientTemperature()
{
    return irAmbientTemperature;
}

double SensorTagDataProvider::getInfraredObjectTemperature()
{
    return irObjectTemperature;
}

QString SensorTagDataProvider::getLightIntensityLuxString()
{
    return QString::number(lightIntensityLux) + QLatin1String(" Lux");
}

double SensorTagDataProvider::getLightIntensityLux()
{
    return lightIntensityLux;
}

double SensorTagDataProvider::getBarometerCelsiusTemperature()
{
    return barometerCelsiusTemperature;
}

QString SensorTagDataProvider::getBarometerCelsiusTemperatureString()
{
    return QString::number(barometerCelsiusTemperature) + QString("\u00B0C");
}

double SensorTagDataProvider::getBarometerTemperatureAverage()
{
    return barometerTemperatureAverage;
}

QString SensorTagDataProvider::getBarometer_hPaString()
{
    return QString::number(barometerHPa) + QLatin1String(" hPa");
}
double SensorTagDataProvider::getBarometer_hPa()
{
    return barometerHPa;
}

float SensorTagDataProvider::getGyroscopeX_degPerSec()
{
    return gyroscopeX_degPerSec;
}

float SensorTagDataProvider::getGyroscopeY_degPerSec()
{
    return gyroscopeY_degPerSec;
}

float SensorTagDataProvider::getGyroscopeZ_degPerSec()
{
    return gyroscopeZ_degPerSec;
}

float SensorTagDataProvider::getAccelometer_xAxis()
{
    return accelometerX;
}

float SensorTagDataProvider::getAccelometer_yAxis()
{
    return accelometerY;
}

float SensorTagDataProvider::getAccelometer_zAxis()
{
    return accelometerZ;
}

float SensorTagDataProvider::getMagnetometerMicroT_xAxis()
{
    return magnetometerMicroT_xAxis;
}

float SensorTagDataProvider::getMagnetometerMicroT_yAxis()
{
    return magnetometerMicroT_yAxis;
}

float SensorTagDataProvider::getMagnetometerMicroT_zAxis()
{
    return magnetometerMicroT_zAxis;
}

float SensorTagDataProvider::getRotationX()
{
    return rotation_x;
}

float SensorTagDataProvider::getRotationY()
{
    return rotation_y;
}

float SensorTagDataProvider::getRotationZ()
{
    return rotation_z;
}

int SensorTagDataProvider::getRotationUpdateInterval()
{
    return intervalRotation;
}

int SensorTagDataProvider::tagType() const
{
    return m_tagType;
}

QString SensorTagDataProvider::id() const
{
    return m_id;
}

SensorTagDataProvider::ProviderState SensorTagDataProvider::state() const
{
    return m_state;
}

void SensorTagDataProvider::setState(SensorTagDataProvider::ProviderState state)
{
    if (state != m_state) {
        m_state = state;
        emit stateChanged();
    }
}

void SensorTagDataProvider::reset()
{
    qCDebug(boot2QtDemos) << "Attempt to reset sensortag that doesn't have reset functionality implemented!";
}

void SensorTagDataProvider::recalibrate()
{
    reset();
}
