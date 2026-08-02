import QtQuick 2.0
import QtSensors 5.0

Item {
    id: detector

    property int windowMs: 2000
    property int cooldownMs: 3000
    property real gravityAlpha: 0.82
    property real jerkThreshold: 7.5
    property real rmsJerkThreshold: 5.2
    property real activeSampleRatio: 0.58
    property real directionDeadband: 1.2
    property int minimumDirectionChanges: 12
    property int minimumSamples: 30
    property var samples: []
    property double lastTriggeredAt: 0

    signal shakeDetected()

    function reset() {
        samples = [];
        accelerometer.initialized = false;
    }

    function linearAcceleration(x, y, z) {
        if (!accelerometer.initialized) {
            accelerometer.gravityX = x;
            accelerometer.gravityY = y;
            accelerometer.gravityZ = z;
            accelerometer.previousLinearX = 0;
            accelerometer.previousLinearY = 0;
            accelerometer.previousLinearZ = 0;
            accelerometer.initialized = true;
        }

        accelerometer.gravityX = gravityAlpha * accelerometer.gravityX + (1 - gravityAlpha) * x;
        accelerometer.gravityY = gravityAlpha * accelerometer.gravityY + (1 - gravityAlpha) * y;
        accelerometer.gravityZ = gravityAlpha * accelerometer.gravityZ + (1 - gravityAlpha) * z;

        return {
            x: x - accelerometer.gravityX,
            y: y - accelerometer.gravityY,
            z: z - accelerometer.gravityZ
        };
    }

    function dominantDirection(linear) {
        var axis = "x";
        var value = linear.x;

        if (Math.abs(linear.y) > Math.abs(value)) {
            axis = "y";
            value = linear.y;
        }

        if (Math.abs(linear.z) > Math.abs(value)) {
            axis = "z";
            value = linear.z;
        }

        return {
            axis: axis,
            sign: Math.abs(value) < directionDeadband ? 0 : (value > 0 ? 1 : -1)
        };
    }

    function addSample(linear, jerk) {
        var now = Date.now();
        var direction = dominantDirection(linear);
        samples.push({
            at: now,
            jerk: jerk,
            active: jerk >= jerkThreshold,
            axis: direction.axis,
            sign: direction.sign
        });

        while (samples.length > 0 && now - samples[0].at > windowMs) {
            samples.shift();
        }

        if (now - lastTriggeredAt < cooldownMs
                || samples.length < minimumSamples
                || now - samples[0].at < windowMs) {
            return;
        }

        var activeSamples = 0;
        var jerkSumOfSquares = 0;
        var directionChanges = 0;
        for (var i = 0; i < samples.length; i++) {
            jerkSumOfSquares += samples[i].jerk * samples[i].jerk;
            if (samples[i].active) {
                activeSamples++;
            }

            if (i > 0
                    && samples[i].sign !== 0
                    && samples[i - 1].sign !== 0
                    && samples[i].axis === samples[i - 1].axis
                    && samples[i].sign !== samples[i - 1].sign) {
                directionChanges++;
            }
        }

        var rmsJerk = Math.sqrt(jerkSumOfSquares / samples.length);
        if (activeSamples / samples.length >= activeSampleRatio
                && rmsJerk >= rmsJerkThreshold
                && directionChanges >= minimumDirectionChanges) {
            lastTriggeredAt = now;
            samples = [];
            shakeDetected();
        }
    }

    Accelerometer {
        id: accelerometer

        active: true
        dataRate: 25

        property real gravityX: 0
        property real gravityY: 0
        property real gravityZ: 0
        property real previousLinearX: 0
        property real previousLinearY: 0
        property real previousLinearZ: 0
        property bool initialized: false

        onReadingChanged: {
            var linear = detector.linearAcceleration(
                reading.x,
                reading.y,
                reading.z
            );

            detector.addSample(linear, Math.sqrt(
                Math.pow(linear.x - previousLinearX, 2) +
                Math.pow(linear.y - previousLinearY, 2) +
                Math.pow(linear.z - previousLinearZ, 2)
            ));

            previousLinearX = linear.x;
            previousLinearY = linear.y;
            previousLinearZ = linear.z;
        }
    }
}
