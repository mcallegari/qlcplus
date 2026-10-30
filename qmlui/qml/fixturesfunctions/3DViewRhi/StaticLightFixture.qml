/*
  Q Light Controller Plus
  StaticLightFixture.qml (RHI backend)
*/

import QtQuick
import RhiQmlItem 1.0
import org.qlcplus.classes 1.0

StaticLight {
    id: root

    property real baseIntensity: 1.0
    property real shutterValue: 1.0
    intensity: baseIntensity * shutterValue

    function setShutter(type, low, high) {
        shutterAnim.stop()
        inPhase.duration = 0
        inPhase.easing.type = Easing.Linear
        highPhase.duration = 0
        outPhase.duration = 0
        outPhase.easing.type = Easing.Linear
        lowPhase.duration = low

        switch (type)
        {
            case QLCCapability.ShutterOpen:
                shutterValue = 1.0
            break

            case QLCCapability.ShutterClose:
                shutterValue = 0
            break

            case QLCCapability.StrobeFastToSlow:
            case QLCCapability.StrobeSlowToFast:
            case QLCCapability.StrobeFrequency:
            case QLCCapability.StrobeFreqRange:
                highPhase.duration = high
                shutterAnim.start()
            break

            case QLCCapability.PulseFastToSlow:
            case QLCCapability.PulseSlowToFast:
            case QLCCapability.PulseFrequency:
            case QLCCapability.PulseFreqRange:
                inPhase.duration = high / 2
                outPhase.duration = high / 2
                inPhase.easing.type = Easing.InOutCubic
                outPhase.easing.type = Easing.InOutCubic
                shutterAnim.start()
            break

            case QLCCapability.RampUpFastToSlow:
            case QLCCapability.RampUpSlowToFast:
            case QLCCapability.RampUpFrequency:
            case QLCCapability.RampUpFreqRange:
                inPhase.duration = high
                shutterAnim.start()
            break

            case QLCCapability.RampDownSlowToFast:
            case QLCCapability.RampDownFastToSlow:
            case QLCCapability.RampDownFrequency:
            case QLCCapability.RampDownFreqRange:
                outPhase.duration = high
                shutterAnim.start()
            break
        }
    }

    /* **************** Gobo rotation and shake **************** */

    // maximum lateral deviation of the gobo image while shaking, in degrees
    property real goboShakeAmplitude: 1.0

    function setGoboSpeed(cw, speed) {
        goboAnim.stop()
        if (speed <= 0)
            return

        // continue from the current angle to avoid visible jumps
        var start = goboRotation % 360
        goboAnim.from = start
        goboAnim.to = cw ? start + 360 : start - 360
        goboAnim.duration = speed
        goboAnim.start()
    }

    // period: duration in ms of a full oscillation. 0 stops shaking
    function setGoboShake(period) {
        goboShakeAnim.stop()
        if (period <= 0) {
            goboShake = 0
            return
        }
        shakeOutPhase.duration = period / 4
        shakeSwingPhase.duration = period / 2
        shakeBackPhase.duration = period / 4
        goboShakeAnim.start()
    }

    NumberAnimation on goboRotation {
        id: goboAnim
        running: false
        loops: Animation.Infinite
    }

    SequentialAnimation on goboShake {
        id: goboShakeAnim
        running: false
        loops: Animation.Infinite
        NumberAnimation { id: shakeOutPhase; from: 0; to: root.goboShakeAmplitude; easing.type: Easing.OutSine }
        NumberAnimation { id: shakeSwingPhase; from: root.goboShakeAmplitude; to: -root.goboShakeAmplitude; easing.type: Easing.InOutSine }
        NumberAnimation { id: shakeBackPhase; from: -root.goboShakeAmplitude; to: 0; easing.type: Easing.InSine }
    }

    SequentialAnimation on shutterValue
    {
        id: shutterAnim
        running: false
        loops: Animation.Infinite
        NumberAnimation { id: inPhase; from: 0; to: 1.0; duration: 0; easing.type: Easing.Linear }
        NumberAnimation { id: highPhase; from: 1.0; to: 1.0; duration: 200; easing.type: Easing.Linear }
        NumberAnimation { id: outPhase; from: 1.0; to: 0; duration: 0; easing.type: Easing.Linear }
        NumberAnimation { id: lowPhase; from: 0; to: 0; duration: 800; easing.type: Easing.Linear }
    }
}
