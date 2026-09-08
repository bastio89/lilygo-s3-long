#include "desk/flexispot.h"

#include <math.h>

namespace desk {

namespace {
inline bool elapsed(uint32_t now, uint32_t since, uint32_t span) {
    return static_cast<uint32_t>(now - since) >= span;
}

// Ueberlaufsichere Deadline-Pruefung (millis() laeuft nach ~49 Tagen ueber).
inline bool reached(uint32_t now, uint32_t deadline) {
    return static_cast<int32_t>(now - deadline) >= 0;
}
} // namespace

FlexiSpot::FlexiSpot(Io &io, const Config &config) : io_(io), config_(config) {}

void FlexiSpot::begin(uint32_t nowMs) {
    parser_.reset();
    io_.setWakeLine(false);
    lastSentKeys_ = loctek::KEY_NONE;
    lastTxMs_ = nowMs;
    lastHeightMs_ = nowMs;
}

void FlexiSpot::poll(uint32_t nowMs) {
    pumpRx(nowMs);

    if (heightValid_ && elapsed(nowMs, lastHeightMs_, config_.heightStaleMs)) {
        heightValid_ = false;
        motion_ = Motion::Idle;
    }

    if (holdLimited_ && reached(nowMs, holdUntilMs_)) {
        heldKeys_ = loctek::KEY_NONE;
        holdLimited_ = false;
    }

    updateTarget(nowMs);

    if (awake_ && heldKeys_ == loctek::KEY_NONE && !targetActive_ &&
        reached(nowMs, awakeUntilMs_)) {
        awake_ = false;
        io_.setWakeLine(false);
    }

    pumpTx(nowMs);
}

void FlexiSpot::pumpRx(uint32_t nowMs) {
    loctek::Frame frame;
    int guard = 256; // pro Aufruf begrenzen, damit die UI nicht haengt
    while (io_.available() > 0 && guard-- > 0) {
        const int b = io_.read();
        if (b < 0) {
            break;
        }
        if (!parser_.feed(static_cast<uint8_t>(b), frame)) {
            continue;
        }
        if (frameCb_) {
            frameCb_(frame, frameCtx_);
        }
        if (frame.type != loctek::kTypeDisplay || frame.payloadLen < 3) {
            continue;
        }
        loctek::DisplayValue value;
        if (!loctek::decodeDisplay(frame.payload, value)) {
            continue;
        }
        display_ = value;
        lastHeightMs_ = nowMs;
        if (value.numeric) {
            const float previous = heightCm_;
            const bool hadHeight = heightValid_;
            heightCm_ = value.value;
            heightValid_ = true;
            if (hadHeight) {
                const float delta = heightCm_ - previous;
                if (delta > 0.05f) {
                    motion_ = Motion::Up;
                } else if (delta < -0.05f) {
                    motion_ = Motion::Down;
                } else {
                    motion_ = Motion::Idle;
                }
            }
        }
    }
}

void FlexiSpot::sendKeys(uint16_t keys) {
    uint8_t frame[loctek::kKeyFrameSize];
    const size_t len = loctek::buildKeyFrame(keys, frame, sizeof(frame));
    if (len > 0) {
        io_.write(frame, len);
    }
}

void FlexiSpot::pumpTx(uint32_t nowMs) {
    if (!awake_) {
        return;
    }
    // Zustandswechsel (Taste gedrueckt/losgelassen) sofort senden, sonst im
    // Panel-Takt wiederholen. Das spart bis zu keyRepeatMs Nachlauf, wenn
    // moveTo() das Ziel erreicht hat.
    const bool changed = heldKeys_ != lastSentKeys_;
    if (!changed && !elapsed(nowMs, lastTxMs_, config_.keyRepeatMs)) {
        return;
    }
    lastTxMs_ = nowMs;
    lastSentKeys_ = heldKeys_;
    sendKeys(heldKeys_);
}

void FlexiSpot::wake(uint32_t nowMs) {
    if (!awake_) {
        io_.setWakeLine(true);
        parser_.reset();
        awake_ = true;
        lastTxMs_ = nowMs - config_.keyRepeatMs; // sofort senden
    }
    awakeUntilMs_ = nowMs + config_.keepAwakeMs;
}

void FlexiSpot::hold(uint16_t keys, uint32_t nowMs) {
    targetActive_ = false;
    wake(nowMs);
    heldKeys_ = keys;
    holdLimited_ = false;
}

void FlexiSpot::release(uint32_t nowMs) {
    heldKeys_ = loctek::KEY_NONE;
    holdLimited_ = false;
    awakeUntilMs_ = nowMs + config_.keepAwakeMs;
}

void FlexiSpot::tap(uint16_t keys, uint32_t nowMs, uint32_t durationMs) {
    wake(nowMs);
    heldKeys_ = keys;
    holdLimited_ = true;
    holdUntilMs_ = nowMs + durationMs;
}

void FlexiSpot::preset(uint8_t index, uint32_t nowMs) {
    uint16_t key;
    switch (index) {
    case 1: key = loctek::KEY_PRESET_1; break;
    case 2: key = loctek::KEY_PRESET_2; break;
    case 3: key = loctek::KEY_PRESET_3; break;
    case 4: key = loctek::KEY_PRESET_4; break;
    default: return;
    }
    targetActive_ = false;
    tap(key, nowMs);
}

void FlexiSpot::moveTo(float cm, uint32_t nowMs) {
    if (cm < config_.minHeightCm) {
        cm = config_.minHeightCm;
    }
    if (cm > config_.maxHeightCm) {
        cm = config_.maxHeightCm;
    }
    wake(nowMs);
    targetCm_ = cm;
    targetActive_ = true;
    lastMoveResult_ = MoveResult::None;
    moveStartedMs_ = nowMs;
    stallSinceMs_ = nowMs;
    stallReferenceCm_ = heightCm_;
    heldKeys_ = loctek::KEY_NONE;
    holdLimited_ = false;
}

void FlexiSpot::stop(uint32_t nowMs) {
    heldKeys_ = loctek::KEY_NONE;
    holdLimited_ = false;
    if (targetActive_) {
        finishMove(MoveResult::None, nowMs);
    }
    awakeUntilMs_ = nowMs + config_.keepAwakeMs;
}

void FlexiSpot::finishMove(MoveResult result, uint32_t nowMs) {
    targetActive_ = false;
    heldKeys_ = loctek::KEY_NONE;
    holdLimited_ = false;
    lastMoveResult_ = result;
    awakeUntilMs_ = nowMs + config_.keepAwakeMs;
}

void FlexiSpot::updateTarget(uint32_t nowMs) {
    if (!targetActive_) {
        return;
    }
    awakeUntilMs_ = nowMs + config_.keepAwakeMs;

    if (elapsed(nowMs, moveStartedMs_, config_.moveTimeoutMs)) {
        finishMove(MoveResult::TimedOut, nowMs);
        return;
    }

    if (!heightValid_) {
        // Ohne Hoehenrueckmeldung koennen wir nicht regeln. Kurz auf die
        // Steuerbox warten, sonst abbrechen statt blind zu fahren.
        if (elapsed(nowMs, moveStartedMs_, config_.heightStaleMs * 2)) {
            finishMove(MoveResult::NoFeedback, nowMs);
        }
        heldKeys_ = loctek::KEY_NONE;
        return;
    }

    const float delta = targetCm_ - heightCm_;

    if (fabsf(delta) <= config_.targetToleranceCm) {
        finishMove(MoveResult::Reached, nowMs);
        return;
    }

    // Nachlauf beruecksichtigen: kurz vor dem Ziel loslassen.
    if (fabsf(delta) <= config_.stopMarginCm) {
        heldKeys_ = loctek::KEY_NONE;
        // Nach dem Loslassen auf Stillstand warten und dann bewerten.
        if (motion_ == Motion::Idle) {
            finishMove(MoveResult::Reached, nowMs);
        }
        return;
    }

    heldKeys_ = (delta > 0.0f) ? loctek::KEY_UP : loctek::KEY_DOWN;

    if (fabsf(heightCm_ - stallReferenceCm_) > config_.stallEpsilonCm) {
        stallReferenceCm_ = heightCm_;
        stallSinceMs_ = nowMs;
    } else if (elapsed(nowMs, stallSinceMs_, config_.stallTimeoutMs)) {
        finishMove(MoveResult::Stalled, nowMs);
    }
}

} // namespace desk
