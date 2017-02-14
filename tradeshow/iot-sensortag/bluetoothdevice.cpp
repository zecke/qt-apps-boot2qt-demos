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
#include "bluetoothdevice.h"
#include "characteristicinfo.h"

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyService>
#include <QLoggingCategory>
#include <QList>
#include <QTimer>
#include <QDebug>
#include <QtMath>
#include <QDateTime>
#include "bluetoothapiconstants.h"

Q_DECLARE_LOGGING_CATEGORY(boot2QtDemos)

typedef struct {
    qint16 gyrox;
    qint16 gyroy;
    qint16 gyroz;
    qint16 accelx;
    qint16 accely;
    qint16 accelz;
    qint16 magnetomx;
    qint16 magnetomy;
    qint16 magnetomz;
} movement_data_t;

BluetoothDevice::BluetoothDevice()
    : discoveryAgent(0)
    , controller(0)
    , irTemperatureService(0)
    , baroService(0)
    , humidityService(0)
    , lightService(0)
    , motionService(0)
    , m_deviceState(DeviceState::Disconnected)
    , randomAddress(false)
{
    lastMilliseconds = QDateTime::currentMSecsSinceEpoch();
    statusUpdated("Device created");
}

BluetoothDevice::BluetoothDevice(const QBluetoothDeviceInfo &d)
    : BluetoothDevice()
{
    m_deviceInfo = d;
}

BluetoothDevice::~BluetoothDevice()
{
    delete discoveryAgent;
    delete controller;
    qDeleteAll(m_characteristics);
    m_characteristics.clear();
}
QString BluetoothDevice::getAddress() const
{
#if defined(Q_OS_DARWIN)
    // On Apple platforms we do not have addresses,
    // only unique UUIDs generated by Core Bluetooth.
    return m_deviceInfo.deviceUuid().toString();
#else
    return m_deviceInfo.address().toString();
#endif
}

QString BluetoothDevice::getName() const
{
    return m_deviceInfo.name();
}

QVariant BluetoothDevice::getCharacteristics()
{
    return QVariant::fromValue(m_characteristics);
}

void BluetoothDevice::scanServices()
{
    qDeleteAll(m_characteristics);
    m_characteristics.clear();
    emit characteristicsUpdated();

    statusUpdated("(Connecting to device...)");

    if (controller && m_previousAddress != getAddress()) {
        controller->disconnectFromDevice();
        delete controller;
        controller = 0;
    }

    if (!controller) {
        // Connecting signals and slots for connecting to LE services.
        controller = new QLowEnergyController(m_deviceInfo);
        connect(controller, SIGNAL(connected()),
                this, SLOT(deviceConnected()));
        connect(controller, SIGNAL(error(QLowEnergyController::Error)),
                this, SLOT(errorReceived(QLowEnergyController::Error)));
        connect(controller, SIGNAL(disconnected()),
                this, SLOT(deviceDisconnected()));
        connect(controller, SIGNAL(serviceDiscovered(QBluetoothUuid)),
                this, SLOT(addLowEnergyService(QBluetoothUuid)));
        connect(controller, SIGNAL(discoveryFinished()),
                this, SLOT(serviceScanDone()));
    }

    if (randomAddress)
        controller->setRemoteAddressType(QLowEnergyController::RandomAddress);
    else
        controller->setRemoteAddressType(QLowEnergyController::PublicAddress);
    controller->connectToDevice();
}

void BluetoothDevice::addLowEnergyService(const QBluetoothUuid &serviceUuid)
{
    if (serviceUuid == QBluetoothUuid(QLatin1String(IRTEMPERATURESENSOR_SERVICE_UUID))) {
        qCDebug(boot2QtDemos) << "Found infrared temperature service.";
        irTemperatureService = controller->createServiceObject(serviceUuid);
        if (!irTemperatureService) {
            qWarning() << "Could not create infrared temperature service object.";
            return;
        }
        connect(irTemperatureService, &QLowEnergyService::stateChanged, this, &BluetoothDevice::temperatureDetailsDiscovered);
        connect(irTemperatureService, &QLowEnergyService::characteristicChanged, this, &BluetoothDevice::characteristicsRead);
        irTemperatureService->discoverDetails();
    } else if (serviceUuid == QBluetoothUuid(QLatin1String(BAROMETER_SERVICE_UUID))) {
        qCDebug(boot2QtDemos) << "Found barometer service.";
        baroService = controller->createServiceObject(serviceUuid);
        if (!baroService) {
            qWarning() << "Could not create barometer service object.";
            return;
        }
        connect(baroService, &QLowEnergyService::stateChanged, this, &BluetoothDevice::barometerDetailsDiscovered);
        connect(baroService, &QLowEnergyService::characteristicChanged, this, &BluetoothDevice::characteristicsRead);
        baroService->discoverDetails();
    } else if (serviceUuid == QBluetoothUuid(QLatin1String(HUMIDITYSENSOR_SERVICE_UUID))) {
        qCDebug(boot2QtDemos) << "Found humidity service.";
        humidityService = controller->createServiceObject(serviceUuid);
        if (!humidityService) {
            qWarning() << "Could not create humidity service object.";
            return;
        }
        connect(humidityService, &QLowEnergyService::stateChanged, this, &BluetoothDevice::humidityDetailsDiscovered);
        connect(humidityService, &QLowEnergyService::characteristicChanged, this, &BluetoothDevice::characteristicsRead);
        humidityService->discoverDetails();
    } else if (serviceUuid == QBluetoothUuid(QLatin1String(LIGHTSENSOR_SERVICE_UUID))) {
        qCDebug(boot2QtDemos) << "Found light service.";
        lightService = controller->createServiceObject(serviceUuid);
        if (!lightService) {
            qWarning() << "Could not create light service object.";
            return;
        }
        connect(lightService, &QLowEnergyService::stateChanged, this, &BluetoothDevice::lightIntensityDetailsDiscovered);
        connect(lightService, &QLowEnergyService::characteristicChanged, this, &BluetoothDevice::characteristicsRead);
        lightService->discoverDetails();
    } else if (serviceUuid == QBluetoothUuid(QLatin1String(MOTIONSENSOR_SERVICE_UUID))) {
        qCDebug(boot2QtDemos) << "Found motion service.";
        motionService = controller->createServiceObject(serviceUuid);
        if (!motionService) {
            qWarning() << "Could not create motion service object.";
            return;
        }
        connect(motionService, &QLowEnergyService::stateChanged, this, &BluetoothDevice::motionDetailsDiscovered);
        connect(motionService, &QLowEnergyService::characteristicChanged, this, &BluetoothDevice::characteristicsRead);
        motionService->discoverDetails();
    } else {
        qCDebug(boot2QtDemos) << "Found unhandled service with id" << serviceUuid << ".";
    }
}

void BluetoothDevice::serviceScanDone()
{
    statusUpdated("(Service scan done!)");
    qCDebug(boot2QtDemos) << "ServiceScan done.";
    if (!irTemperatureService)
        qCDebug(boot2QtDemos) << "Infrared temperature service not found.";

    if (!baroService)
        qCDebug(boot2QtDemos) << "Barometer service not found.";

    if (!humidityService)
        qCDebug(boot2QtDemos) << "Humidity service not found.";

    if (!lightService)
        qCDebug(boot2QtDemos) << "Light service not found.";

    if (!motionService)
        qCDebug(boot2QtDemos) << "Motion service not found.";

    attitudeChangeInterval.restart();
}

void BluetoothDevice::temperatureDetailsDiscovered(QLowEnergyService::ServiceState newstate)
{
    if (newstate == QLowEnergyService::ServiceDiscovered) {
        connect(irTemperatureService, &QLowEnergyService::characteristicWritten, [=]() {
            qCDebug(boot2QtDemos) << "Wrote Characteristic - temperature";
        });

        connect(irTemperatureService, static_cast<void(QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
            [=](QLowEnergyService::ServiceError newError) {
            qCDebug(boot2QtDemos) << "error while writing - temperature:" << newError;
        });

        // data characteristic
        QLowEnergyCharacteristic characteristic = irTemperatureService->characteristic(
                    QBluetoothUuid(QLatin1String("f000aa01-0451-4000-b000-000000000000")));
        if (characteristic.isValid()) {
            irTemperatureHandle = characteristic.handle();
            const QLowEnergyDescriptor notificationDescriptor = characteristic.descriptor(
                        QBluetoothUuid::ClientCharacteristicConfiguration);
            if (notificationDescriptor.isValid())
                irTemperatureService->writeDescriptor(notificationDescriptor, QByteArray::fromHex(ENABLE_NOTIF_STR));
        }

        // configuration characteristic
        characteristic = irTemperatureService->characteristic(
                    QBluetoothUuid(QLatin1String("f000aa02-0451-4000-b000-000000000000")));
        if (characteristic.isValid())
            irTemperatureService->writeCharacteristic(characteristic, QByteArray::fromHex(START_MEASUREMENT_STR), QLowEnergyService::WriteWithResponse);

        // timeout characteristic
        characteristic = irTemperatureService->characteristic(
                    QBluetoothUuid(QLatin1String("f000aa03-0451-4000-b000-000000000000")));
        if (characteristic.isValid()) {
            irTemperatureService->writeCharacteristic(characteristic, QByteArray::fromHex(SENSORTAG_SLOW_TIMER_TIMEOUT_STR), QLowEnergyService::WriteWithResponse); // Period 1 second
        }

        m_temperatureMeasurementStarted = true;
    }
}

void BluetoothDevice::barometerDetailsDiscovered(QLowEnergyService::ServiceState newstate) {
    if (newstate == QLowEnergyService::ServiceDiscovered) {
        connect(baroService, &QLowEnergyService::characteristicWritten, [=]() {
            qCDebug(boot2QtDemos) << "Wrote Characteristic - barometer";
        });

        connect(baroService, static_cast<void(QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
            [=](QLowEnergyService::ServiceError newError) {
            qCDebug(boot2QtDemos) << "error while writing - barometer:" << newError;
        });

        // data characteristic
        QLowEnergyCharacteristic characteristic = baroService->characteristic(
                    QBluetoothUuid(QLatin1String("f000aa41-0451-4000-b000-000000000000")));
        if (characteristic.isValid()) {
            baroHandle = characteristic.handle();
            const QLowEnergyDescriptor notificationDescriptor = characteristic.descriptor(
                        QBluetoothUuid::ClientCharacteristicConfiguration);
            if (notificationDescriptor.isValid())
                baroService->writeDescriptor(notificationDescriptor, QByteArray::fromHex(ENABLE_NOTIF_STR));
        }

        // configuration characteristic
        characteristic = baroService->characteristic(
                    QBluetoothUuid(QLatin1String("f000aa42-0451-4000-b000-000000000000")));
        if (characteristic.isValid())
            baroService->writeCharacteristic(characteristic, QByteArray::fromHex(START_MEASUREMENT_STR), QLowEnergyService::WriteWithResponse);

        // timeout characteristic
        characteristic = baroService->characteristic(
                    QBluetoothUuid(QLatin1String("f000aa44-0451-4000-b000-000000000000")));
        if (characteristic.isValid()) {
            baroService->writeCharacteristic(characteristic, QByteArray::fromHex(SENSORTAG_MEDIUM_TIMER_TIMEOUT_STR), QLowEnergyService::WriteWithResponse); // Period 500 ms
        }

        m_barometerMeasurementStarted = true;
    }
}

void BluetoothDevice::humidityDetailsDiscovered(QLowEnergyService::ServiceState newstate)
{
    if (newstate == QLowEnergyService::ServiceDiscovered) {
        connect(humidityService, &QLowEnergyService::characteristicWritten, [=]() {
            qCDebug(boot2QtDemos) << "Wrote Characteristic - humidity";
        });

        connect(humidityService, static_cast<void(QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
            [=](QLowEnergyService::ServiceError newError) {
            qCDebug(boot2QtDemos) << "error while writing - humidity:" << newError;
        });

        // data characteristic
        QLowEnergyCharacteristic characteristic = humidityService->characteristic(
                    QBluetoothUuid(QLatin1String("f000aa21-0451-4000-b000-000000000000")));
        if (characteristic.isValid()) {
            humidityHandle = characteristic.handle();
            const QLowEnergyDescriptor notificationDescriptor = characteristic.descriptor(
                        QBluetoothUuid::ClientCharacteristicConfiguration);
            if (notificationDescriptor.isValid())
                humidityService->writeDescriptor(notificationDescriptor, QByteArray::fromHex(ENABLE_NOTIF_STR));
        }

        // configuration characteristic
        characteristic = humidityService->characteristic(
                    QBluetoothUuid(QLatin1String("f000aa22-0451-4000-b000-000000000000")));
        if (characteristic.isValid())
            humidityService->writeCharacteristic(characteristic, QByteArray::fromHex(START_MEASUREMENT_STR), QLowEnergyService::WriteWithResponse);

        // timeout characteristic
        characteristic = humidityService->characteristic(
                    QBluetoothUuid(QLatin1String("f000aa23-0451-4000-b000-000000000000")));
        if (characteristic.isValid()) {
            humidityService->writeCharacteristic(characteristic, QByteArray::fromHex(SENSORTAG_SLOW_TIMER_TIMEOUT_STR), QLowEnergyService::WriteWithResponse); // Period 500 ms
        }

        m_humidityMeasurementStarted = true;
    }
}

void BluetoothDevice::lightIntensityDetailsDiscovered(QLowEnergyService::ServiceState newstate)
{
    if (newstate == QLowEnergyService::ServiceDiscovered) {
        connect(lightService, &QLowEnergyService::characteristicWritten, [=]() {
            qCDebug(boot2QtDemos) << "Wrote Characteristic - light intensity";
        });

        connect(lightService, static_cast<void(QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
            [=](QLowEnergyService::ServiceError newError) {
            qCDebug(boot2QtDemos) << "error while writing - light intensity:" << newError;
        });

        // data characteristic
        QLowEnergyCharacteristic characteristic = lightService->characteristic(
                    QBluetoothUuid(QLatin1String("f000aa71-0451-4000-b000-000000000000")));
        if (characteristic.isValid()) {
            lightHandle = characteristic.handle();
            const QLowEnergyDescriptor notificationDescriptor = characteristic.descriptor(
                        QBluetoothUuid::ClientCharacteristicConfiguration);
            if (notificationDescriptor.isValid())
                lightService->writeDescriptor(notificationDescriptor, QByteArray::fromHex(ENABLE_NOTIF_STR));
        }

        // configuration characteristic
        characteristic = lightService->characteristic(
                    QBluetoothUuid(QLatin1String("f000aa72-0451-4000-b000-000000000000")));
        if (characteristic.isValid())
            lightService->writeCharacteristic(characteristic, QByteArray::fromHex(START_MEASUREMENT_STR), QLowEnergyService::WriteWithResponse);

        // timeout characteristic
        characteristic = lightService->characteristic(
                    QBluetoothUuid(QLatin1String("f000aa73-0451-4000-b000-000000000000")));
        if (characteristic.isValid()) {
            lightService->writeCharacteristic(characteristic, QByteArray::fromHex(SENSORTAG_MEDIUM_TIMER_TIMEOUT_STR), QLowEnergyService::WriteWithResponse); // Period 500 ms
        }

        m_lightIntensityMeasurementStarted = true;
    }
}

void BluetoothDevice::motionDetailsDiscovered(QLowEnergyService::ServiceState newstate)
{
    // reset the time once more before we start to receive measurements.
    lastMilliseconds = QDateTime::currentMSecsSinceEpoch();

    if (newstate == QLowEnergyService::ServiceDiscovered) {
        connect(motionService, &QLowEnergyService::characteristicWritten, [=]() {
            qCDebug(boot2QtDemos) << "Wrote Characteristic - gyro";
        });

        connect(motionService, static_cast<void(QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
            [=](QLowEnergyService::ServiceError newError) {
            qCDebug(boot2QtDemos) << "error while writing - gyro:" << newError;
        });

        // data characteristic
        QLowEnergyCharacteristic characteristic = motionService->characteristic(
                    QBluetoothUuid(QLatin1String("f000aa81-0451-4000-b000-000000000000")));
        if (characteristic.isValid()) {
            motionHandle = characteristic.handle();
            const QLowEnergyDescriptor notificationDescriptor = characteristic.descriptor(
                        QBluetoothUuid::ClientCharacteristicConfiguration);
            if (notificationDescriptor.isValid())
                motionService->writeDescriptor(notificationDescriptor, QByteArray::fromHex(ENABLE_NOTIF_STR));
        }

        // configuration characteristic
        characteristic = motionService->characteristic(
                    QBluetoothUuid(QLatin1String("f000aa82-0451-4000-b000-000000000000")));
        if (characteristic.isValid())
            motionService->writeCharacteristic(characteristic, QByteArray::fromHex(MOVEMENT_ENABLE_SENSORS_BITMASK_VALUE), QLowEnergyService::WriteWithResponse);

        // timeout characteristic
        characteristic = motionService->characteristic(
                    QBluetoothUuid(QLatin1String("f000aa83-0451-4000-b000-000000000000")));
        if (characteristic.isValid()) {
            motionService->writeCharacteristic(characteristic, QByteArray::fromHex(SENSORTAG_RAPID_TIMER_TIMEOUT_STR), QLowEnergyService::WriteWithResponse);
        }
        m_motionMeasurementStarted = true;
    }
}

void BluetoothDevice::characteristicsRead(const QLowEnergyCharacteristic &info, const QByteArray &value)
{
    const QLowEnergyHandle handle = info.handle();
    if (handle == irTemperatureHandle)
        irTemperatureReceived(value);
    else if (handle == humidityHandle)
        humidityReceived(value);
    else if (handle == baroHandle)
        barometerReceived(value);
    else if (handle == motionHandle)
        motionReceived(value);
    else if (handle == lightHandle)
        lightIntensityReceived(value);
    else
        qWarning() << "Invalid handle" << info.handle() << "in characteristicsRead!";
}

void BluetoothDevice::setState(BluetoothDevice::DeviceState state)
{
    if (m_deviceState != state) {
        m_deviceState = state;
        emit stateChanged();
    }
}

double BluetoothDevice::convertIrTemperatureAPIReadingToCelsius(quint16 rawReading)
{
    // Compute and filter final value according to TI Bluetooth LE API
    const float SCALE_LSB = 0.03125;
    int it = (int)((rawReading) >> 2);
    float t = (float)it;
    return t * SCALE_LSB;
}

void BluetoothDevice::irTemperatureReceived(const QByteArray &value)
{
    const unsigned int rawObjectTemperature = (((quint8)value.at(3)) << 8) + ((quint8)value.at(2));
    const double objectTemperature = convertIrTemperatureAPIReadingToCelsius(rawObjectTemperature);
    const unsigned int rawAmbientTemperature = (((quint8)value.at(1)) << 8) + ((quint8)value.at(0));
    const double ambientTemperature = convertIrTemperatureAPIReadingToCelsius(rawAmbientTemperature);
    emit temperatureChanged(ambientTemperature, objectTemperature);
}

void BluetoothDevice::barometerReceived(const QByteArray &value)
{
    //Merge bytes
    unsigned int temperature_raw;
    unsigned int barometer_raw;
    if (value.length() == 6) {
        temperature_raw = (((quint8)value.at(2)) << 16) + (((quint8)value.at(1)) << 8) + ((quint8)value.at(0));
        barometer_raw = (((quint8)value.at(5)) << 16) + (((quint8)value.at(4)) << 8) + ((quint8)value.at(3));
    } else {
        temperature_raw = (((quint8)value.at(1)) << 8) + ((quint8)value.at(0));
        barometer_raw = (((quint8)value.at(3)) << 8) + ((quint8)value.at(2));
    }

    double temperature = static_cast<double>(temperature_raw);
    temperature /= BAROMETER_API_READING_DIVIDER;

    double barometer = static_cast<double>(barometer_raw);
    barometer /= BAROMETER_API_READING_DIVIDER;
    emit barometerChanged(temperature, barometer);
}

void BluetoothDevice::humidityReceived(const QByteArray &value)
{
    //Merge bytes
    unsigned int humidity_raw = (((quint8)value.at(3)) << 8) + ((quint8)value.at(2));
    double humidity = static_cast<double>(humidity_raw);
    humidity = humidity * HUMIDITY_API_READING_MULTIPLIER;
    emit humidityChanged(humidity);
}

void BluetoothDevice::lightIntensityReceived(const QByteArray &value)
{
    //Merge bytes
    uint16_t lightIntensity_raw;
    lightIntensity_raw = (((quint8)value.at(1)) << 8) + ((quint8)value.at(0));
    uint16_t e, m;
    m = lightIntensity_raw & 0x0FFF;
    e = (lightIntensity_raw & 0xF000) >> 12;

    // Compute and final value according to TI Bluetooth LE API
    double lightIntensity = ((double)m) * (0.01 * (double)qPow(2.0,(qreal)e));
    emit lightIntensityChanged(lightIntensity);
}

void BluetoothDevice::motionReceived(const QByteArray &value)
{
    static MotionSensorData data;
    data.msSincePreviousData = lastMilliseconds;
    lastMilliseconds = QDateTime::currentMSecsSinceEpoch();
    data.msSincePreviousData = lastMilliseconds - data.msSincePreviousData;
    movement_data_t values;
    quint8* writePtr = (quint8*)(&values);

    for (int i = 0; i < 18; ++i) {
        *writePtr = (quint8)value.at(i);
        writePtr++;
    }

    // Data is in little endian. Fix here if needed.

    //Convert gyroscope and accelometer readings to proper units
    data.gyroScope_x = double(values.gyrox) * GYROSCOPE_API_READING_MULTIPLIER;
    data.gyroScope_y = double(values.gyroy) * GYROSCOPE_API_READING_MULTIPLIER;
    data.gyroScope_z = double(values.gyroz) * GYROSCOPE_API_READING_MULTIPLIER;
    // Accelometer at 8G
    data.accelometer_x = double(values.accelx) * ACCELOMETER_API_READING_MULTIPLIER;
    data.accelometer_y = double(values.accely) * ACCELOMETER_API_READING_MULTIPLIER;
    data.accelometer_z = double(values.accelz) * ACCELOMETER_API_READING_MULTIPLIER;
    data.magnetometer_x = double(values.magnetomx);
    data.magnetometer_y = double(values.magnetomy);
    data.magnetometer_z = double(values.magnetomz);
    emit motionChanged(data);
}

void BluetoothDevice::deviceConnected()
{
    setState(DeviceState::Connected);
    statusUpdated("(Discovering services...)");
    controller->discoverServices();
}

void BluetoothDevice::errorReceived(QLowEnergyController::Error /*error*/)
{
    setState(DeviceState::Error);
    statusUpdated(QString("Error: %1)").arg(controller->errorString()));
}

void BluetoothDevice::disconnectFromDevice()
{
    // UI always expects disconnect() signal when calling this signal
    // TODO what is really needed is to extend state() to a multi value
    // and thus allowing UI to keep track of controller progress in addition to
    // device scan progress

    if (controller->state() != QLowEnergyController::UnconnectedState)
        controller->disconnectFromDevice();
    else
        deviceDisconnected();
}

void BluetoothDevice::deviceDisconnected()
{
    statusUpdated("Disconnect from device");
    setState(BluetoothDevice::Disconnected);
}

void BluetoothDevice::serviceDetailsDiscovered(QLowEnergyService::ServiceState newState)
{
    if (newState != QLowEnergyService::ServiceDiscovered) {
        // do not hang in "Scanning for characteristics" mode forever
        // in case the service discovery failed
        // We have to queue the signal up to give UI time to even enter
        // the above mode
        if (newState != QLowEnergyService::DiscoveringServices) {
            QMetaObject::invokeMethod(this, "characteristicsUpdated",
                                      Qt::QueuedConnection);
        }
        return;
    }

    QLowEnergyService *service = qobject_cast<QLowEnergyService *>(sender());
    if (!service)
        return;


    const QList<QLowEnergyCharacteristic> chars = service->characteristics();
    foreach (const QLowEnergyCharacteristic &ch, chars) {
        CharacteristicInfo *cInfo = new CharacteristicInfo(ch);
        m_characteristics.append(cInfo);
    }

    emit characteristicsUpdated();
}

BluetoothDevice::DeviceState BluetoothDevice::state() const
{
    return m_deviceState;
}
