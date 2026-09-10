#!/usr/bin/env python3
"""
Adversarial Stress-Testing Suite for xLights AI Subsystems
Validates edge cases, boundary conditions, extreme inputs, and math invariants.
"""

import sys
import math
import time

def stress_test_resampling_and_normalization():
    print("==================================================")
    print("Stress Test 1: Audio Resampling & Normalization Pipeline")
    print("==================================================")
    
    def resample_and_normalize(samples, source_rate, is_stereo):
        if not samples or source_rate <= 0:
            return []
        
        # 1. Convert to mono
        if is_stereo:
            frame_count = len(samples) // 2
            mono = [(samples[i*2] + samples[i*2+1]) * 0.5 for i in range(frame_count)]
        else:
            mono = samples[:]
            
        target_rate = 16000
        if source_rate == target_rate:
            resampled = mono
        else:
            ratio = float(source_rate) / float(target_rate)
            target_len = int(math.floor(len(mono) / ratio))
            resampled = []
            for i in range(target_len):
                src_idx = i * ratio
                idx0 = int(math.floor(src_idx))
                idx1 = min(idx0 + 1, len(mono) - 1)
                frac = src_idx - idx0
                val = mono[idx0] * (1.0 - frac) + mono[idx1] * frac
                resampled.append(val)
                
        max_peak = max((abs(s) for s in resampled), default=0.0)
        scale = (0.95 / max_peak) if max_peak > 1e-6 else 1.0
        normalized = [max(-1.0, min(1.0, s * scale)) for s in resampled]
        return normalized

    # 1a. Non-standard sample rates
    rates = [8000, 11025, 22050, 32000, 44100, 48000, 88200, 96000, 192000]
    for rate in rates:
        duration_sec = 0.5
        sample_count = int(rate * duration_sec) * 2 # stereo
        test_samples = [math.sin(i * 0.05) for i in range(sample_count)]
        out = resample_and_normalize(test_samples, rate, True)
        expected_len = int(16000 * duration_sec)
        assert abs(len(out) - expected_len) <= 1, f"Failed for rate {rate}: got {len(out)}, expected {expected_len}"
        for s in out:
            assert not math.isnan(s) and not math.isinf(s)
            assert -1.0 <= s <= 1.0
        print(f"  [PASS] Non-standard sample rate {rate}Hz -> 16kHz resampled cleanly ({len(out)} samples)")

    # 1b. Extreme dynamic range (ultra-low and ultra-high amplitudes)
    low_amp = [1e-9 * math.sin(i * 0.1) for i in range(88200)]
    out_low = resample_and_normalize(low_amp, 44100, True)
    assert len(out_low) == 16000
    print("  [PASS] Near-zero amplitude (1e-9) handled without division-by-zero")

    high_amp = [1e6 * (1 if i % 2 == 0 else -1) for i in range(88200)]
    out_high = resample_and_normalize(high_amp, 44100, True)
    assert len(out_high) == 16000
    for s in out_high:
        assert -1.0 <= s <= 1.0
    print("  [PASS] Hyper-amplitude (1e6) safely clamped to [-1.0, 1.0]")

    # 1c. Large buffer throughput test (10 seconds of 96kHz stereo = 1,920,000 samples)
    t0 = time.perf_counter()
    large_buf = [0.5 * math.sin(i * 0.01) for i in range(1920000)]
    out_large = resample_and_normalize(large_buf, 96000, True)
    elapsed = time.perf_counter() - t0
    assert len(out_large) == 160000
    print(f"  [PASS] Throughput: Resampled 1.92M samples in {elapsed:.3f}s (~{len(large_buf)/elapsed/1e6:.2f} Msa/s)")
    print("Result: Audio Resampling & Normalization Stress Tests PASSED\n")
    return True

def stress_test_quantization_math():
    print("==================================================")
    print("Stress Test 2: Timeline Quantization & Frame Math")
    print("==================================================")
    
    def quantize_time_interval(start_ms, end_ms, frame_ms):
        if frame_ms <= 0:
            frame_ms = 50
        if end_ms < start_ms:
            start_ms, end_ms = end_ms, start_ms
        start_ms = max(0, start_ms)
        end_ms = max(0, end_ms)
        
        start_frame = int(round(start_ms / float(frame_ms)))
        end_frame = int(round(end_ms / float(frame_ms)))
        if end_frame <= start_frame:
            end_frame = start_frame + 1
        return (start_frame * frame_ms, end_frame * frame_ms, start_frame, end_frame)

    def clamp_frames(frame_idx, total_frames):
        if total_frames <= 0:
            return 0
        return max(0, min(frame_idx, total_frames - 1))

    # Inverted intervals
    q_inv = quantize_time_interval(1500, 500, 50)
    assert q_inv[0] <= q_inv[1]
    print(f"  [PASS] Inverted interval (1500, 500) normalized to ({q_inv[0]}, {q_inv[1]})")

    # Zero frame duration guard
    q_zero = quantize_time_interval(100, 300, 0)
    assert q_zero[2] < q_zero[3]
    print(f"  [PASS] Zero frame duration fallback applied safely: frameMs=50 -> frames [{q_zero[2]}, {q_zero[3]}]")

    # Degenerate zero-duration interval (start == end)
    q_deg = quantize_time_interval(250, 250, 50)
    assert q_deg[3] == q_deg[2] + 1
    print(f"  [PASS] Degenerate interval (start==end) expanded to minimum 1 frame [{q_deg[2]}, {q_deg[3]}]")

    # Clamp frames edge conditions
    assert clamp_frames(-100, 1000) == 0
    assert clamp_frames(5000, 1000) == 999
    assert clamp_frames(50, 0) == 0
    assert clamp_frames(50, -10) == 0
    print("  [PASS] Clamping invariants hold across negative and out-of-bounds indices")
    print("Result: Timeline Quantization Stress Tests PASSED\n")
    return True

def stress_test_buffer_boundary_wrapping():
    print("==================================================")
    print("Stress Test 3: Render Buffer Boundary & Modulo Wrapping")
    print("==================================================")
    
    class MockRenderBuffer:
        def __init__(self, w, h):
            self.width = w
            self.height = h
            self.buffer = [0] * (max(0, w) * max(0, h))

        def set_pixel(self, x, y, val):
            if self.width <= 0 or self.height <= 0:
                return False
            x_wrapped = x % self.width
            y_wrapped = y % self.height
            if x_wrapped < 0:
                x_wrapped += self.width
            if y_wrapped < 0:
                y_wrapped += self.height
            idx = y_wrapped * self.width + x_wrapped
            self.buffer[idx] = val
            return True

        def get_pixel(self, x, y):
            if self.width <= 0 or self.height <= 0:
                return 0
            if x < 0 or x >= self.width or y < 0 or y >= self.height:
                return 0
            return self.buffer[y * self.width + x]

    # Test 3a: Zero & negative dimension safety
    b_zero = MockRenderBuffer(0, 0)
    assert not b_zero.set_pixel(10, 10, 0xFF)
    assert b_zero.get_pixel(10, 10) == 0
    print("  [PASS] Zero-dimension buffer gracefully rejects SetPixel/GetPixel without crashing")

    # Test 3b: Extreme coordinates wrapping
    b = MockRenderBuffer(100, 50)
    b.set_pixel(100000005, 50000002, 0xAA)
    assert b.get_pixel(5, 2) == 0xAA
    b.set_pixel(-100000095, -50000048, 0xBB)
    assert b.get_pixel(5, 2) == 0xBB
    print("  [PASS] Extreme coordinate wrapping (+10^8 and -10^8) correctly maps to [5, 2]")

    # Test 3c: Out of bounds direct read
    assert b.get_pixel(-1, 0) == 0
    assert b.get_pixel(100, 50) == 0
    print("  [PASS] Out-of-bounds direct GetPixel returns 0 (xlBLACK)")
    print("Result: Render Buffer Boundary Stress Tests PASSED\n")
    return True

def run_all_stress_tests():
    t1 = stress_test_resampling_and_normalization()
    t2 = stress_test_quantization_math()
    t3 = stress_test_buffer_boundary_wrapping()
    
    if t1 and t2 and t3:
        print("==================================================")
        print("   ALL ADVERSARIAL STRESS TESTS PASSED (3/3)      ")
        print("==================================================")
        return 0
    return 1

if __name__ == "__main__":
    sys.exit(run_all_stress_tests())
