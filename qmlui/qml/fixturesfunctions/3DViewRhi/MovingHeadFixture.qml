/*
  Q Light Controller Plus
  MovingHeadFixture.qml (RHI backend)
*/

import QtQuick
import RhiQmlItem 1.0
import org.qlcplus.classes 1.0

MovingHeadLight {
    id: root

    property real baseIntensity: 1.0
    property real shutterValue: 1.0
    property real panRangeDegrees: 360.0
    property real tiltRangeDegrees: 270.0
    property int panSpeed: 4000
    property int tiltSpeed: 4000
    property real panVelocity: {
        if (panRangeDegrees <= 0)
            return 0
        var base = panRangeDegrees * 1000.0 / Math.max(1, panSpeed)
        var capped = panRangeDegrees * 1000.0 / 300.0
        return Math.min(base, capped)
    }
    property real tiltVelocity: {
        if (tiltRangeDegrees <= 0)
            return 0
        var base = tiltRangeDegrees * 1000.0 / Math.max(1, tiltSpeed)
        var capped = tiltRangeDegrees * 1000.0 / 300.0
        return Math.min(base, capped)
    }
    intensity: baseIntensity * shutterValue

    function setPanTilt(panTarget, tiltTarget) {
        pan = panTarget
        tilt = tiltTarget
    }

    function setPositionSpeed(panDuration, tiltDuration) {
        if (panDuration !== -1)
            panSpeed = panDuration
        if (tiltDuration !== -1)
            tiltSpeed = tiltDuration
    }

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

    Behavior on pan {
        SmoothedAnimation {
            velocity: root.panVelocity
            reversingMode: SmoothedAnimation.Immediate
        }
    }

    Behavior on tilt {
        SmoothedAnimation {
            velocity: root.tiltVelocity
            reversingMode: SmoothedAnimation.Immediate
        }
    }
}
