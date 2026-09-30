#pragma once

#include "imgui.h"
#include "imgui_internal.h"

namespace Cupertino {
    // Damped spring with unit mass, described the SwiftUI way.
    struct Spring {
        float stiffness = 157.9f;
        float damping = 25.13f;

        // .spring(response:dampingFraction:)
        static Spring Response(float response, float damping_fraction);
        // .spring(duration:bounce:)
        static Spring Duration(float duration, float bounce);
    };

    // CAMediaTimingFunction control points; Evaluate maps linear time to progress.
    struct TimingCurve {
        float x1 = 0.0f;
        float y1 = 0.0f;
        float x2 = 1.0f;
        float y2 = 1.0f;

        float Evaluate(float t) const;
    };

    // An animation in SwiftUI terms: a spring, or a timing curve over a fixed duration.
    struct Animation {
        enum class Kind {
            None,
            Spring,
            Curve,
        };

        Kind kind = Kind::None;
        Cupertino::Spring spring;
        TimingCurve curve;
        float duration = 0.0f;

        static Animation Immediate();
        static Animation SpringWith(const Cupertino::Spring& spring);
        static Animation Timed(const TimingCurve& curve, float duration);

        static Animation Smooth();
        static Animation Snappy();
        static Animation Bouncy();
        static Animation Interactive();
        static Animation Linear(float duration);
        static Animation EaseIn(float duration);
        static Animation EaseOut(float duration);
        static Animation EaseInOut(float duration);
    };

    namespace Motion {
        // Delta time used by all animations, clamped so a stalled frame does not make values jump.
        float DeltaTime();
    } // namespace Motion

    // A float that follows its target with the given animation. Springs keep their velocity when the target changes
    // mid-flight; curve animations restart from the current value.
    struct AnimatedFloat {
        float value = 0.0f;
        float velocity = 0.0f;
        float target = 0.0f;
        float from = 0.0f;
        float elapsed = 0.0f;
        bool initialized = false;

        float Update(float new_target, const Animation& animation);
        void Snap(float new_value);
        bool Settled() const;
    };
} // namespace Cupertino
