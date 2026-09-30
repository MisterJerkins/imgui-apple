#include "Motion.h"

#include <cmath>

namespace Cupertino {
    Spring Spring::Response(float response, float damping_fraction) {
        const float omega = 2.0f * IM_PI / ImMax(response, 0.001f);
        return Spring{omega * omega, 2.0f * damping_fraction * omega};
    }

    Spring Spring::Duration(float duration, float bounce) {
        const float damping_fraction = bounce >= 0.0f ? 1.0f - bounce : 1.0f / (1.0f + bounce);
        return Response(duration, damping_fraction);
    }

    static float CubicAt(float a, float b, float t) {
        // Bezier with P0 = 0 and P3 = 1 on one axis.
        const float u = 1.0f - t;
        return 3.0f * u * u * t * a + 3.0f * u * t * t * b + t * t * t;
    }

    // Bisection on x, like Flutter's Cubic.transform; precise enough for UI timing.
    float TimingCurve::Evaluate(float t) const {
        if (t <= 0.0f)
            return 0.0f;
        if (t >= 1.0f)
            return 1.0f;
        float low = 0.0f;
        float high = 1.0f;
        for (int i = 0; i < 24; ++i) {
            const float middle = 0.5f * (low + high);
            if (CubicAt(x1, x2, middle) < t)
                low = middle;
            else
                high = middle;
        }
        return CubicAt(y1, y2, 0.5f * (low + high));
    }

    Animation Animation::Immediate() {
        return Animation{};
    }

    Animation Animation::SpringWith(const Cupertino::Spring& spring) {
        Animation animation;
        animation.kind = Kind::Spring;
        animation.spring = spring;
        return animation;
    }

    Animation Animation::Timed(const TimingCurve& curve, float duration) {
        Animation animation;
        animation.kind = Kind::Curve;
        animation.curve = curve;
        animation.duration = duration;
        return animation;
    }

    Animation Animation::Smooth() {
        return SpringWith(Spring::Duration(0.5f, 0.0f));
    }

    Animation Animation::Snappy() {
        return SpringWith(Spring::Duration(0.5f, 0.15f));
    }

    Animation Animation::Bouncy() {
        return SpringWith(Spring::Duration(0.5f, 0.3f));
    }

    Animation Animation::Interactive() {
        return SpringWith(Spring::Response(0.15f, 0.86f));
    }

    Animation Animation::Linear(float duration) {
        return Timed({0.0f, 0.0f, 1.0f, 1.0f}, duration);
    }

    Animation Animation::EaseIn(float duration) {
        return Timed({0.42f, 0.0f, 1.0f, 1.0f}, duration);
    }

    Animation Animation::EaseOut(float duration) {
        return Timed({0.0f, 0.0f, 0.58f, 1.0f}, duration);
    }

    Animation Animation::EaseInOut(float duration) {
        return Timed({0.42f, 0.0f, 0.58f, 1.0f}, duration);
    }

    float Motion::DeltaTime() {
        return ImClamp(ImGui::GetIO().DeltaTime, 0.0f, 1.0f / 30.0f);
    }

    // Exact solution of x'' = -k x - c v over dt, with x measured from the target.
    static void StepSpring(float& x, float& v, const Spring& spring, float dt) {
        const float k = spring.stiffness;
        const float c = spring.damping;
        const float omega = std::sqrt(k);
        const float zeta = c / (2.0f * omega);
        if (zeta < 0.999f) {
            const float omega_d = omega * std::sqrt(1.0f - zeta * zeta);
            const float decay = std::exp(-zeta * omega * dt);
            const float cos_t = std::cos(omega_d * dt);
            const float sin_t = std::sin(omega_d * dt);
            const float b = (v + zeta * omega * x) / omega_d;
            const float new_x = decay * (x * cos_t + b * sin_t);
            const float new_v = decay * ((b * omega_d - zeta * omega * x) * cos_t - (x * omega_d + zeta * omega * b) * sin_t);
            x = new_x;
            v = new_v;
        } else if (zeta <= 1.001f) {
            const float decay = std::exp(-omega * dt);
            const float b = v + omega * x;
            const float new_x = (x + b * dt) * decay;
            const float new_v = (v - omega * b * dt) * decay;
            x = new_x;
            v = new_v;
        } else {
            const float root = omega * std::sqrt(zeta * zeta - 1.0f);
            const float r1 = -zeta * omega + root;
            const float r2 = -zeta * omega - root;
            const float c2 = (v - r1 * x) / (r2 - r1);
            const float c1 = x - c2;
            const float e1 = std::exp(r1 * dt);
            const float e2 = std::exp(r2 * dt);
            x = c1 * e1 + c2 * e2;
            v = c1 * r1 * e1 + c2 * r2 * e2;
        }
    }

    float AnimatedFloat::Update(float new_target, const Animation& animation) {
        if (!initialized || animation.kind == Animation::Kind::None) {
            Snap(new_target);
            return value;
        }
        if (new_target != target) {
            from = value;
            elapsed = 0.0f;
            target = new_target;
        }
        if (Settled())
            return value;

        const float dt = Motion::DeltaTime();
        if (animation.kind == Animation::Kind::Spring) {
            float x = value - target;
            StepSpring(x, velocity, animation.spring, dt);
            value = target + x;
            if (ImFabs(value - target) < 0.0005f && ImFabs(velocity) < 0.01f) {
                value = target;
                velocity = 0.0f;
            }
        } else {
            elapsed += dt;
            const float progress = animation.duration > 0.0f ? ImSaturate(elapsed / animation.duration) : 1.0f;
            value = ImLerp(from, target, animation.curve.Evaluate(progress));
            velocity = progress < 1.0f ? (target - from) / ImMax(animation.duration, 0.001f) : 0.0f;
            if (progress >= 1.0f)
                value = target;
        }
        return value;
    }

    void AnimatedFloat::Snap(float new_value) {
        value = new_value;
        target = new_value;
        from = new_value;
        velocity = 0.0f;
        elapsed = 0.0f;
        initialized = true;
    }

    bool AnimatedFloat::Settled() const {
        return value == target && velocity == 0.0f;
    }
} // namespace Cupertino
