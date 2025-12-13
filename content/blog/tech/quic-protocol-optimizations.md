# QUIC Transport Optimizations from a Reimplementation
*Published on 2025-12-05*

Reimplementing QUIC in C++ gave me room to tune congestion control and pacing. Here’s what moved the needle and why packet captures mattered.
## What I Built
- C++ QUIC transport with custom congestion control and pacing
- gRPC harness for cross-language benchmarks
- Wireshark tracing inside a Docker-based latency simulator

## Results
- ~25% reduction in average RTT under mixed loads
- Smoother cwnd evolution and fewer bursty loss events

## Key Lessons
- Fine-grained pacing tames burst loss more than conservative cwnd alone.
- Benchmark harnesses need predictable network conditions; Docker tc profiles helped.
- Wireshark filters (udp.port && quic) made iterative tuning faster.

## Next Steps
- Add BBR-style probing
- Bake in automated pcap diffing in CI
